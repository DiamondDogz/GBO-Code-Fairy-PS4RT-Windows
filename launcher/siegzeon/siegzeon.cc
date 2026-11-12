// SPDX-License-Identifier: GPL-2.0-or-later
// SiegZeon.exe: opens a WebView2 window onto the page that siegzeon/launcher_host.py serves.
// The same program as Kagekidan.exe and Ravn.exe, with Code Fairy's paths and name.
#include "webview/webview.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::wstring exe_dir() {
  wchar_t path[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
  return fs::path(std::wstring(path, n)).parent_path().wstring();
}

static bool port_open(int port) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == INVALID_SOCKET) return false;
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<u_short>(port));
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  bool ok = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
  closesocket(sock);
  return ok;
}

static int read_port(const std::wstring& dir) {
  std::ifstream file(fs::path(dir) / "launcher.port");
  int port = 0;
  file >> port;
  return port;
}

static int current_port(const std::wstring& dir) {
  std::wstring parent = fs::path(dir).parent_path().wstring();
  for (const std::wstring& candidate : {dir, parent}) {
    int port = read_port(candidate);
    if (port > 0 && port_open(port)) return port;
  }
  return 0;
}

static int wait_port(const std::wstring& dir) {
  for (int i = 0; i < 80; ++i) {
    int port = current_port(dir);
    if (port > 0) return port;
    Sleep(100);
  }
  return 0;
}

static std::wstring first_existing(const std::vector<std::wstring>& paths) {
  for (const auto& path : paths) {
    if (path.empty()) continue;
    std::error_code error;
    if (fs::exists(path, error)) return path;
  }
  return L"";
}

static std::wstring read_line(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) return L"";
  std::string bytes((std::istreambuf_iterator<char>(file)), {});
  while (!bytes.empty() && (bytes.back() == '\n' || bytes.back() == '\r' || bytes.back() == ' ')) bytes.pop_back();
  return std::wstring(bytes.begin(), bytes.end());
}

static std::wstring search_program(const wchar_t* name) {
  wchar_t buffer[MAX_PATH];
  DWORD n = SearchPathW(nullptr, name, L".exe", MAX_PATH, buffer, nullptr);
  if (n == 0 || n >= MAX_PATH) return L"";
  return buffer;
}

// Message boxes follow the Windows display language: French for French, English otherwise.
static bool french() { return PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_FRENCH; }
static std::wstring say(const wchar_t* en, const wchar_t* fr) { return french() ? fr : en; }

// Gives every top-level window of this thread the comet icon (resource 1).
static BOOL CALLBACK set_icon(HWND hwnd, LPARAM) {
  HINSTANCE self = GetModuleHandleW(nullptr);
  HICON big = (HICON)LoadImageW(self, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
  HICON little = (HICON)LoadImageW(self, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
  if (big) SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)big);
  if (little) SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)little);
  return TRUE;
}

static HANDLE start_host(const std::wstring& dir, std::wstring& reason) {
  std::wstring script = first_existing({
      dir + L"\\siegzeon\\launcher_host.py",
      dir + L"\\launcher\\siegzeon\\launcher_host.py",
      dir + L"\\..\\launcher\\siegzeon\\launcher_host.py",
  });
  if (script.empty()) {
    reason = say(L"SiegZeon.exe is alone in this folder. Unzip the whole archive and keep "
                 L"SiegZeon.exe next to the launcher folder.",
                 L"SiegZeon.exe est seul dans ce dossier. Décompresse toute l'archive et garde "
                 L"SiegZeon.exe à côté du dossier launcher.");
    return nullptr;
  }
  fs::path script_dir = fs::path(script).parent_path();
  std::wstring noted = read_line(script_dir / "python.txt");
  // The Python shipped in the archive comes first; a developer build notes its own in python.txt.
  std::wstring python = first_existing({
      dir + L"\\python\\python.exe",
      dir + L"\\..\\python\\python.exe",
      noted,
      L"C:\\msys64\\usr\\bin\\python.exe",
      L"C:\\msys64\\usr\\bin\\python3.exe",
      L"C:\\msys64\\clang64\\bin\\python.exe",
      search_program(L"python.exe"),
      search_program(L"python3.exe"),
  });
  if (python.empty()) {
    reason = say(L"Python cannot be found. The python folder of the archive is missing.",
                 L"Python est introuvable. Le dossier python de l'archive manque.");
    return nullptr;
  }
  std::wstring command = L"\"" + python + L"\" \"" + script + L"\" --no-browser";
  std::vector<wchar_t> mutable_command(command.begin(), command.end());
  mutable_command.push_back(L'\0');
  STARTUPINFOW startup{};
  startup.cb = sizeof(startup);
  PROCESS_INFORMATION process{};
  if (!CreateProcessW(nullptr, mutable_command.data(), nullptr, nullptr, FALSE,
                      CREATE_NO_WINDOW, nullptr, dir.c_str(), &startup, &process)) {
    reason = say(L"Python did not start: ", L"Python n'a pas démarré : ") + python;
    return nullptr;
  }
  CloseHandle(process.hThread);
  return process.hProcess;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  WSADATA wsa{};
  WSAStartup(MAKEWORD(2, 2), &wsa);
  std::wstring dir = exe_dir();
  HANDLE host = nullptr;
  int port = current_port(dir);
  if (port <= 0) {
    std::wstring reason;
    host = start_host(dir, reason);
    if (!host) {
      MessageBoxW(nullptr, reason.c_str(), L"SiegZeon", MB_ICONERROR);
      return 1;
    }
    port = wait_port(dir);
  }
  if (port <= 0) {
    MessageBoxW(nullptr, say(L"The launcher did not start. See launcher-error.txt.",
                             L"Le lanceur n'a pas démarré. Voir launcher-error.txt.").c_str(),
                L"SiegZeon", MB_ICONERROR);
    if (host) TerminateProcess(host, 1);
    return 1;
  }
  std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
  int code = 0;
  try {
    webview::webview window(false, nullptr);
    window.set_title("Sieg Zeon");
    window.set_size(1100, 780, WEBVIEW_HINT_NONE);
    window.navigate(url);
    EnumThreadWindows(GetCurrentThreadId(), set_icon, 0);
    window.run();
  } catch (const std::exception&) {
    MessageBoxW(nullptr,
                say(L"WebView2 cannot be found. Install the Microsoft Edge WebView2 runtime, "
                    L"then start SiegZeon.exe again.",
                    L"WebView2 est introuvable. Installez le runtime Microsoft Edge WebView2, "
                    L"puis relancez SiegZeon.exe.").c_str(),
                L"SiegZeon", MB_ICONERROR);
    code = 1;
  }
  if (host) TerminateProcess(host, 0);
  WSACleanup();
  return code;
}
