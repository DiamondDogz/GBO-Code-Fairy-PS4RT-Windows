#ifdef _WIN32
#include "compat_win.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <bcrypt.h>
#include <errno.h>

void compat_sleep_ns(unsigned long long ns) {
    static __thread HANDLE timer;
    LARGE_INTEGER due;
    if (!ns) {
        SwitchToThread();
        return;
    }
    if (!timer) {
        timer = CreateWaitableTimerExW(NULL, NULL, 0x2 /* HIGH_RESOLUTION */, TIMER_ALL_ACCESS);
        if (!timer) timer = CreateWaitableTimerW(NULL, TRUE, NULL);
    }
    due.QuadPart = -(LONGLONG)(ns / 100);
    if (!due.QuadPart) due.QuadPart = -1;
    if (!timer || !SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE)) {
        Sleep((DWORD)(ns / 1000000ull ? ns / 1000000ull : 1));
        return;
    }
    WaitForSingleObject(timer, INFINITE);
}

long compat_utc_offset(time_t seconds) {
    struct tm local_tm, utc_tm;
    long sec;
    int daydiff;
    if (localtime_s(&local_tm, &seconds) || gmtime_s(&utc_tm, &seconds)) return 0;
    daydiff = local_tm.tm_yday - utc_tm.tm_yday;
    if (daydiff > 1) daydiff = -1;
    if (daydiff < -1) daydiff = 1;
    sec = (long)daydiff * 86400L
        + (local_tm.tm_hour - utc_tm.tm_hour) * 3600L
        + (local_tm.tm_min - utc_tm.tm_min) * 60L
        + (local_tm.tm_sec - utc_tm.tm_sec);
    return sec;
}

static int remove_one(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    if (attr & FILE_ATTRIBUTE_DIRECTORY) return RemoveDirectoryA(path) ? 0 : -1;
    return DeleteFileA(path) ? 0 : -1;
}

void compat_remove_tree(const char *path) {
    char pattern[1024], child[1024];
    WIN32_FIND_DATAA found;
    HANDLE search;
    snprintf(pattern, sizeof pattern, "%s\\*", path);
    search = FindFirstFileA(pattern, &found);
    if (search != INVALID_HANDLE_VALUE) {
        do {
            if (!strcmp(found.cFileName, ".") || !strcmp(found.cFileName, "..")) continue;
            snprintf(child, sizeof child, "%s\\%s", path, found.cFileName);
            if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) compat_remove_tree(child);
            else DeleteFileA(child);
        } while (FindNextFileA(search, &found));
        FindClose(search);
    }
    remove_one(path);
}

int compat_errno_from_win32(unsigned long error) {
    switch (error) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND: return ENOENT;
    case ERROR_ACCESS_DENIED: return EACCES;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS: return EEXIST;
    case ERROR_DIR_NOT_EMPTY: return ENOTEMPTY;
    case ERROR_DISK_FULL: return ENOSPC;
    case ERROR_INVALID_HANDLE: return EBADF;
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION: return EBUSY;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY: return ENOMEM;
    default: return EIO;
    }
}

void compat_cpu_times(int who, long long *user_us, long long *kernel_us) {
    FILETIME created, exited, kernel, user;
    BOOL ok = who == 0 ? GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user)
                       : GetThreadTimes(GetCurrentThread(), &created, &exited, &kernel, &user);
    if (!ok) {
        *user_us = *kernel_us = 0;
        return;
    }
    *user_us = ((((long long)user.dwHighDateTime << 32) | user.dwLowDateTime) / 10);
    *kernel_us = ((((long long)kernel.dwHighDateTime << 32) | kernel.dwLowDateTime) / 10);
}

int compat_random(void *buffer, size_t size) {
    if (size > 0xffffffffu) return -1;
    return BCryptGenRandom(NULL, buffer, (unsigned long)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) ? -1 : 0;
}

#include <io.h>
long long pread(int fd, void *buf, unsigned long long count, long long offset) {
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    OVERLAPPED overlapped;
    DWORD got = 0;
    if (handle == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    /* POSIX pread leaves the file offset alone. ReadFile with OVERLAPPED on a
     * synchronous handle moves it to offset+got, which would make the next
     * read() on this descriptor start somewhere else. Save and restore it.
     * Callers serialize operations per descriptor (runtime_file.c). */
    LARGE_INTEGER zero, saved;
    zero.QuadPart = 0;
    if (!SetFilePointerEx(handle, zero, &saved, FILE_CURRENT)) saved.QuadPart = -1;
    memset(&overlapped, 0, sizeof overlapped);
    overlapped.Offset = (DWORD)offset;
    overlapped.OffsetHigh = (DWORD)((unsigned long long)offset >> 32);
    BOOL ok = ReadFile(handle, buf, (DWORD)count, &got, &overlapped);
    DWORD error = ok ? 0 : GetLastError();
    if (saved.QuadPart >= 0) SetFilePointerEx(handle, saved, NULL, FILE_BEGIN);
    if (!ok) {
        /* At or past end of file POSIX returns 0, not an error. */
        if (error == ERROR_HANDLE_EOF) return 0;
        errno = EIO;
        return -1;
    }
    return (long long)got;
}
long long pwrite(int fd, const void *buf, unsigned long long count, long long offset) {
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    OVERLAPPED overlapped;
    DWORD wrote = 0;
    if (handle == INVALID_HANDLE_VALUE) {
        errno = EBADF;
        return -1;
    }
    LARGE_INTEGER zero, saved;
    zero.QuadPart = 0;
    if (!SetFilePointerEx(handle, zero, &saved, FILE_CURRENT)) saved.QuadPart = -1;
    memset(&overlapped, 0, sizeof overlapped);
    overlapped.Offset = (DWORD)offset;
    overlapped.OffsetHigh = (DWORD)((unsigned long long)offset >> 32);
    BOOL ok = WriteFile(handle, buf, (DWORD)count, &wrote, &overlapped);
    if (saved.QuadPart >= 0) SetFilePointerEx(handle, saved, NULL, FILE_BEGIN);
    if (!ok) {
        errno = EIO;
        return -1;
    }
    return (long long)wrote;
}
int fsync(int fd) { return _commit(fd); }

static void *kernel_symbol(const char *name) {
    HMODULE module = GetModuleHandleA("kernelbase.dll");
    void *symbol = module ? (void *)GetProcAddress(module, name) : NULL;
    if (!symbol) {
        module = GetModuleHandleA("kernel32.dll");
        symbol = module ? (void *)GetProcAddress(module, name) : NULL;
    }
    return symbol;
}
void *ps4rt_VirtualAlloc2(void *process, void *base, unsigned long long size, unsigned long alloc_type, unsigned long protect, void *params, unsigned long param_count) {
    typedef void *(__stdcall *Fn)(void *, void *, unsigned long long, unsigned long, unsigned long, void *, unsigned long);
    static Fn fn;
    if (!fn) fn = (Fn)kernel_symbol("VirtualAlloc2");
    return fn ? fn(process, base, size, alloc_type, protect, params, param_count) : NULL;
}
void *ps4rt_MapViewOfFile3(void *mapping, void *process, void *base, unsigned long long offset, unsigned long long view_size, unsigned long alloc_type, unsigned long protect, void *params, unsigned long param_count) {
    typedef void *(__stdcall *Fn)(void *, void *, void *, unsigned long long, unsigned long long, unsigned long, unsigned long, void *, unsigned long);
    static Fn fn;
    if (!fn) fn = (Fn)kernel_symbol("MapViewOfFile3");
    return fn ? fn(mapping, process, base, offset, view_size, alloc_type, protect, params, param_count) : NULL;
}
void *ps4rt_CreateFileMapping2(void *file, void *security, unsigned long access, unsigned long protect, unsigned long alloc_attr, unsigned long long maximum, const wchar_t *name, void *params, unsigned long param_count) {
    typedef void *(__stdcall *Fn)(void *, void *, unsigned long, unsigned long, unsigned long, unsigned long long, const wchar_t *, void *, unsigned long);
    static Fn fn;
    if (!fn) fn = (Fn)kernel_symbol("CreateFileMapping2");
    return fn ? fn(file, security, access, protect, alloc_attr, maximum, name, params, param_count) : NULL;
}
int ps4rt_UnmapViewOfFile2(void *process, void *base, unsigned long flags) {
    typedef int (__stdcall *Fn)(void *, void *, unsigned long);
    static Fn fn;
    if (!fn) fn = (Fn)kernel_symbol("UnmapViewOfFile2");
    return fn ? fn(process, base, flags) : 0;
}
#endif
