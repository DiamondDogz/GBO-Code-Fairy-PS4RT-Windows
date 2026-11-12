# SPDX-License-Identifier: GPL-2.0-or-later
"""SiegZeon: the Windows launcher for Code Fairy PS4RT.

Modelled on Kagekidan and Ravn. It does not add options. The graphics controls are the
compact in-game menu (gpu/shim/ps4rt_overlay.cpp CompactGraphics, ps4rt_graphics.h).
The game is an extracted folder (--game). The update is a PKG (--update-pkg) and the
volume keys are the PKGs of one folder (repeated --dlc-pkg).
The game is still prepared and started by scripts/ps4rt_launch.py.
"""
import json
import os
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse

HERE = Path(__file__).resolve().parent


def port_root():
    for folder in (HERE, *HERE.parents):
        if (folder / "scripts" / "ps4rt_launch.py").is_file():
            return folder
    return HERE.parent


ROOT = port_root()
PAGE = HERE / "index.html"
STATE = ROOT / "launcher-state.json"
LOG = ROOT / "launcher.log"
PORT_FILE = ROOT / "launcher.port"
LOCK = threading.Lock()
PROCESS = {"handle": None}

# gpu/shim/ps4rt_graphics.h, in the in-game menu order.
RESOLUTIONS = (
    ("540p", "1024 x 576", 1024, 576, "16:9"),
    ("720p", "1280 x 720", 1280, 720, "16:9"),
    ("native", "1920 x 1080", 1920, 1080, "16:9"),
    ("1440p", "2560 x 1440", 2560, 1440, "16:9"),
    ("2160p", "3840 x 2160 (4K)", 3840, 2160, "16:9"),
    ("2560x1080", "2560 x 1080", 2560, 1080, "21:9"),
    ("3440x1440", "3440 x 1440", 3440, 1440, "21:9"),
    ("3840x1440", "3840 x 1440", 3840, 1440, "24:9"),
    ("3840x1080", "3840 x 1080", 3840, 1080, "32:9"),
    ("5120x1440", "5120 x 1440", 5120, 1440, "32:9"),
)
OUTPUTS = (
    ("native", "Native (AA only)", 0, 0),
    ("1920x1080", "1920 x 1080", 1920, 1080),
    ("2560x1440", "2560 x 1440", 2560, 1440),
    ("3840x2160", "3840 x 2160 (4K)", 3840, 2160),
    ("3440x1440", "3440 x 1440", 3440, 1440),
    ("3840x1440", "3840 x 1440", 3840, 1440),
    ("5120x1440", "5120 x 1440", 5120, 1440),
)
QUALITIES = (("max", "Max (original)"), ("medium", "Medium"), ("low", "Low"))
UPSCALERS = (("off", "Off"), ("fsr1", "FSR 1"), ("fsr3", "FSR 3.1"), ("fsr4", "FSR 4 v07"))
DISPLAY_MODES = (("windowed", "Windowed"), ("borderless", "Borderless"), ("fullscreen", "Fullscreen"))
FPS_LIMITS = ((0, "60 (original)"), (30, "30"))
ANISOTROPIES = ((0, "Game default"), (2, "2x"), (4, "4x"), (8, "8x"), (16, "16x"))

DEFAULT = {
    "settings": {
        "renderResolution": "native",
        "graphicsQuality": "max",
        "upscaler": "off",
        "outputRes": "native",
        "sharpness": 0,
        "displayMode": "windowed",
        "showFps": False,
        "fpsLimit": 0,
        "anisotropy": 0,
        "vibration": 100,
    },
    "paths": {"game": "", "updatePkg": "", "dlcDir": ""},
    "lang": "en",
}
LANGS = ("en", "fr")


class LauncherError(RuntimeError):
    """An error the page translates: str() is a key of its errors table."""


def title_id():
    try:
        data = json.loads((ROOT / "port_features.json").read_text(encoding="utf-8"))
        title = str(data.get("title_id") or "")
    except (OSError, json.JSONDecodeError, AttributeError):
        title = ""
    if len(title) == 9 and title.startswith("CUSA") and title[4:].isdigit():
        return title
    return "CUSA28763"


def ini_path():
    return ROOT / "out" / title_id() / "ps4rt.ini"


def resolution_size(key):
    for value, _label, width, height, _group in RESOLUTIONS:
        if value == key:
            return width, height
    return 1920, 1080


def output_allowed(internal_w, internal_h, output_key):
    """ps4rt_settings.cpp OutputAllowed: Native always; otherwise larger and the same aspect."""
    if output_key == "native":
        return True
    output = {value: (width, height) for value, _label, width, height in OUTPUTS}
    if output_key not in output or internal_w <= 0 or internal_h <= 0:
        return False
    out_w, out_h = output[output_key]
    if not (out_w > internal_w and out_h > internal_h):
        return False
    return abs((out_w / out_h) - (internal_w / internal_h)) <= 0.01 * (internal_w / internal_h)


def pick(value, allowed, default):
    return value if value in allowed else default


def read_ini(path):
    found = {}
    try:
        lines = Path(path).read_text(encoding="utf-8").splitlines()
    except OSError:
        return found
    for line in lines:
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, value = line.partition("=")
        found[key.strip()] = value.strip()
    return found


def write_keys(path, updates):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except OSError:
        lines = ["# ps4rt settings (in-game menu: Insert / R3+L2)"]
    seen = set()
    written = []
    for line in lines:
        if line and not line.startswith("#") and "=" in line:
            key = line.split("=", 1)[0].strip()
            if key in updates:
                written.append(f"{key}={updates[key]}")
                seen.add(key)
                continue
        written.append(line)
    for key, value in updates.items():
        if key not in seen:
            written.append(f"{key}={value}")
    path.write_text("\n".join(written) + "\n", encoding="utf-8")


def settings_from_ini(ini):
    found = {}
    resolutions = {value for value, *_rest in RESOLUTIONS}
    if ini.get("render_resolution") in resolutions:
        found["renderResolution"] = ini["render_resolution"]
    if ini.get("graphics_quality") in {value for value, _label in QUALITIES}:
        found["graphicsQuality"] = ini["graphics_quality"]
    if ini.get("upscaler") in {value for value, _label in UPSCALERS}:
        found["upscaler"] = ini["upscaler"]
    if ini.get("output_res") in {value for value, *_rest in OUTPUTS}:
        found["outputRes"] = ini["output_res"]
    if ini.get("display_mode") in {value for value, _label in DISPLAY_MODES}:
        found["displayMode"] = ini["display_mode"]
    try:
        if int(ini.get("anisotropy", "")) in {value for value, _label in ANISOTROPIES}:
            found["anisotropy"] = int(ini["anisotropy"])
    except ValueError:
        pass
    try:
        limit = int(ini.get("fps_limit", ""))
        if limit in {value for value, _label in FPS_LIMITS}:
            found["fpsLimit"] = limit
    except ValueError:
        pass
    if "show_fps" in ini:
        found["showFps"] = ini["show_fps"] == "1"
    try:
        if "vibration" in ini:
            found["vibration"] = max(0, min(100, int(ini["vibration"])))
    except ValueError:
        pass
    try:
        if "sharpen" in ini:
            found["sharpness"] = int(round(max(0.0, min(1.0, float(ini.get("sharpness") or 0))) * 100)) if ini["sharpen"] == "1" else 0
    except ValueError:
        pass
    return found


def ini_updates(settings):
    """Only the keys CompactGraphics shows. Other ps4rt.ini keys stay as the game saved them."""
    render = pick(settings.get("renderResolution"), {value for value, *_rest in RESOLUTIONS}, "native")
    width, height = resolution_size(render)
    output = pick(settings.get("outputRes"), {value for value, *_rest in OUTPUTS}, "native")
    if not output_allowed(width, height, output):
        output = "native"
    upscaler = pick(settings.get("upscaler"), {value for value, _label in UPSCALERS}, "off")
    try:
        sharp = max(0, min(100, int(settings.get("sharpness") or 0)))
    except (TypeError, ValueError):
        sharp = 0
    try:
        aniso = int(settings.get("anisotropy") or 0)
    except (TypeError, ValueError):
        aniso = 0
    try:
        fps = int(settings.get("fpsLimit") or 0)
    except (TypeError, ValueError):
        fps = 0
    try:
        vibration = int(100 if settings.get("vibration") is None else settings.get("vibration"))
    except (TypeError, ValueError):
        vibration = 100
    vibration = max(0, min(100, vibration))
    temporal = upscaler != "off"
    return {
        "render_resolution": render,
        "graphics_quality": pick(settings.get("graphicsQuality"), {value for value, _label in QUALITIES}, "max"),
        "upscaler": upscaler,
        "output_res": output,
        "sharpen": "1" if sharp > 0 else "0",
        "sharpness": f"{sharp / 100:.2f}",
        "rcas": "0" if temporal or sharp == 0 else "1",
        "rcas_strength": str(sharp),
        "display_mode": pick(settings.get("displayMode"), {value for value, _label in DISPLAY_MODES}, "windowed"),
        "show_fps": "1" if settings.get("showFps") else "0",
        "fps_limit": str(pick(fps, {value for value, _label in FPS_LIMITS}, 0)),
        "anisotropy": str(pick(aniso, {value for value, _label in ANISOTROPIES}, 0)),
        "vibration": str(vibration),
    }


def catalog():
    return {
        "resolutions": [
            {"value": value, "label": label, "w": width, "h": height, "group": group}
            for value, label, width, height, group in RESOLUTIONS
        ],
        "outputs": [
            {"value": value, "label": label, "w": width, "h": height}
            for value, label, width, height in OUTPUTS
        ],
        "qualities": [{"value": value, "label": label} for value, label in QUALITIES],
        "upscalers": [{"value": value, "label": label} for value, label in UPSCALERS],
        "displayModes": [{"value": value, "label": label} for value, label in DISPLAY_MODES],
        "fpsLimits": [{"value": value, "label": label} for value, label in FPS_LIMITS],
        "anisotropies": [{"value": value, "label": label} for value, label in ANISOTROPIES],
    }


def blank_state():
    return json.loads(json.dumps(DEFAULT))


def load_state():
    state = blank_state()
    try:
        saved = json.loads(STATE.read_text(encoding="utf-8"))
        if isinstance(saved.get("settings"), dict):
            state["settings"].update(saved["settings"])
        paths = saved.get("paths") or {}
        state["paths"]["game"] = str(paths.get("game") or "")
        state["paths"]["updatePkg"] = str(paths.get("updatePkg") or "")
        state["paths"]["dlcDir"] = str(paths.get("dlcDir") or "")
        if saved.get("lang") in LANGS:
            state["lang"] = saved["lang"]
    except (OSError, json.JSONDecodeError, AttributeError):
        pass
    state["settings"].update(settings_from_ini(read_ini(ini_path())))
    width, height = resolution_size(state["settings"]["renderResolution"])
    if not output_allowed(width, height, state["settings"]["outputRes"]):
        state["settings"]["outputRes"] = "native"
    return state


def save_state(state):
    STATE.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")


def apply_payload(state, payload):
    if isinstance(payload.get("settings"), dict):
        state["settings"].update(payload["settings"])
    paths = payload.get("paths")
    if isinstance(paths, dict):
        if "game" in paths:
            state["paths"]["game"] = str(paths.get("game") or "")
        if "updatePkg" in paths:
            state["paths"]["updatePkg"] = str(paths.get("updatePkg") or "")
        if "dlcDir" in paths:
            state["paths"]["dlcDir"] = str(paths.get("dlcDir") or "")
    if payload.get("lang") in LANGS:
        state["lang"] = payload["lang"]
    state["settings"].update(settings_from_ini(ini_updates(state["settings"])))
    # settings_from_ini reads ini text. Feed it the updates we are about to write.
    state["settings"].update(settings_from_ini(ini_updates(state["settings"])))
    return state


def normalize(state):
    """Clamp to the source lists by writing and reading the ini keys."""
    updates = ini_updates(state["settings"])
    state["settings"].update(settings_from_ini(updates))
    width, height = resolution_size(state["settings"]["renderResolution"])
    if not output_allowed(width, height, state["settings"]["outputRes"]):
        state["settings"]["outputRes"] = "native"
    return state


def powershell(script):
    completed = subprocess.run(
        ["powershell", "-NoProfile", "-STA", "-Command", script],
        capture_output=True, text=True, encoding="utf-8", errors="replace", check=False,
    )
    return (completed.stdout or "").strip()


def folder_dialog(description):
    script = (
        "Add-Type -AssemblyName System.Windows.Forms; "
        "$d = New-Object System.Windows.Forms.FolderBrowserDialog; "
        f"$d.Description = '{description}'; "
        "if ($d.ShowDialog() -eq 'OK') { [Console]::Out.Write($d.SelectedPath) }"
    )
    return powershell(script)


# Folder picker titles, in the launcher language.
PICKER = {
    "en": {"game": "Extracted CUSA28763 folder (eboot.bin, sce_sys)",
           "update": "v01.03 update PKG (only if the game is still 01.00)",
           "dlc": "Folder with the three volume key PKGs"},
    "fr": {"game": "Dossier CUSA28763 extrait (eboot.bin, sce_sys)",
           "update": "PKG de mise à jour v01.03 (seulement si le jeu est encore en 01.00)",
           "dlc": "Dossier des trois PKG de clés de volume"},
}


def browse(kind, lang):
    if os.name != "nt":
        raise LauncherError("windowsOnly")
    titles = PICKER.get(lang, PICKER["en"])
    keys = {"game": "game", "update": "updatePkg", "dlc": "dlcDir"}
    if kind not in keys:
        raise LauncherError("unknownFolder")
    title = titles[kind].replace("'", "''")
    if kind == "update":
        chosen = powershell(
            "Add-Type -AssemblyName System.Windows.Forms; "
            "$d = New-Object System.Windows.Forms.OpenFileDialog; "
            "$d.Filter = 'PlayStation PKG (*.pkg)|*.pkg'; "
            f"$d.Title = '{title}'; "
            "if ($d.ShowDialog() -eq 'OK') { [Console]::Out.Write($d.FileName) }")
    else:
        chosen = folder_dialog(title)
    return {keys[kind]: chosen} if chosen else {}


def console_python():
    exe = Path(sys.executable)
    if exe.name.lower() == "pythonw.exe" and exe.with_name("python.exe").is_file():
        return str(exe.with_name("python.exe"))
    return str(exe)


def running():
    handle = PROCESS["handle"]
    return handle is not None and handle.poll() is None


def launch(state):
    game = str(state["paths"].get("game") or "").strip()
    if not game:
        raise LauncherError("noGame")
    if not (Path(game) / "eboot.bin").is_file():
        raise LauncherError("noEboot")
    if Path(game).suffix.lower() == ".pkg":
        raise LauncherError("pkgGiven")
    update = str(state["paths"].get("updatePkg") or "").strip()
    if update and not (Path(update).is_file() and Path(update).suffix.lower() == ".pkg"):
        raise LauncherError("noUpdate")
    dlc = str(state["paths"].get("dlcDir") or "").strip()
    if dlc and not Path(dlc).is_dir():
        raise LauncherError("noDlc")
    if running():
        raise LauncherError("running")
    normalize(state)
    write_keys(ini_path(), ini_updates(state["settings"]))
    save_state(state)
    command = [console_python(), str(ROOT / "scripts" / "ps4rt_launch.py"),
               "--game", game, "--no-dialogs"]
    if update:
        command += ["--update-pkg", update]
    if dlc:
        command += ["--dlc-dir", dlc]
        seen = set()
        for pkg in sorted(Path(dlc).iterdir()):
            if not pkg.is_file() or pkg.suffix.lower() != ".pkg":
                continue
            key = str(pkg).lower()
            if key in seen:
                continue
            seen.add(key)
            command += ["--dlc-pkg", str(pkg)]
    bat = ROOT / "siegzeon-play.bat"
    quoted = " ".join('"' + str(part).replace('"', '""') + '"' for part in command)
    bat.write_text(
        "@echo off\r\n"
        f"cd /d \"{ROOT}\"\r\n"
        "title SiegZeon\r\n"
        f"{quoted}\r\n"
        "echo.\r\n"
        "echo Exit code %ERRORLEVEL%\r\n"
        "pause\r\n",
        encoding="ascii", errors="replace",
    )
    env = dict(os.environ)
    env["PYTHONUNBUFFERED"] = "1"
    env["PYTHONIOENCODING"] = "utf-8"
    with LOG.open("ab") as log:
        log.write(b"\n--- play ---\n")
    flags = getattr(subprocess, "CREATE_NEW_CONSOLE", 0)
    PROCESS["handle"] = subprocess.Popen(
        ["cmd.exe", "/c", str(bat)], cwd=str(ROOT), env=env, creationflags=flags)


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        return

    def send_json(self, code, payload):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def read_json(self):
        length = int(self.headers.get("Content-Length") or 0)
        if length <= 0 or length > 1_000_000:
            return {}
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def do_GET(self):
        path = urlparse(self.path).path
        if path in ("/", "/index.html"):
            body = PAGE.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)
            return
        if path in ("/artwork.jpg", "/artwork.mp4", "/crowbar.png"):
            kind = {"/artwork.jpg": "image/jpeg", "/artwork.mp4": "video/mp4", "/crowbar.png": "image/png"}[path]
            body = (HERE / path[1:]).read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", kind)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(body)
            return
        if path == "/api/state":
            state = load_state()
            state["catalog"] = catalog()
            state["running"] = running()
            state["titleId"] = title_id()
            self.send_json(200, state)
            return
        self.send_error(404)

    def do_POST(self):
        path = urlparse(self.path).path
        try:
            payload = self.read_json()
            if path == "/api/state":
                with LOCK:
                    state = load_state()
                    normalize(apply_payload(state, payload))
                    save_state(state)
                    write_keys(ini_path(), ini_updates(state["settings"]))
                self.send_json(200, state)
                return
            if path == "/api/browse":
                found = browse(str(payload.get("kind") or ""), load_state()["lang"])
                with LOCK:
                    state = load_state()
                    state["paths"].update(found)
                    save_state(state)
                self.send_json(200, state["paths"])
                return
            if path == "/api/play":
                with LOCK:
                    state = load_state()
                    normalize(apply_payload(state, payload))
                    launch(state)
                self.send_json(200, {"ok": True})
                return
        except LauncherError as error:
            self.send_json(400, {"error": str(error), "code": True})
            return
        except Exception as error:
            self.send_json(400, {"error": str(error)})
            return
        self.send_error(404)


def serve():
    server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
    port = server.server_address[1]
    PORT_FILE.write_text(str(port), encoding="utf-8")
    try:
        server.serve_forever()
    finally:
        try:
            PORT_FILE.unlink()
        except OSError:
            pass


if __name__ == "__main__":
    try:
        serve()
    except Exception as error:
        (ROOT / "launcher-error.txt").write_text(str(error), encoding="utf-8")
        raise
