/* Windows stand-ins for the bits the Linux runtime gets from POSIX. */
#ifndef PS4RT_COMPAT_WIN_H
#define PS4RT_COMPAT_WIN_H
#ifdef _WIN32
#include <stddef.h>
#include <time.h>
typedef unsigned long long ps4rt_jmp_buf[32];
__attribute__((returns_twice)) int ps4rt_setjmp(ps4rt_jmp_buf buffer);
__attribute__((noreturn)) void ps4rt_longjmp(ps4rt_jmp_buf buffer, int value);
void compat_sleep_ns(unsigned long long ns);
long compat_utc_offset(time_t seconds);
void compat_remove_tree(const char *path);
int compat_errno_from_win32(unsigned long error);
void compat_cpu_times(int who, long long *user_us, long long *kernel_us);
int compat_random(void *buffer, size_t size);
void *ps4rt_VirtualAlloc2(void *process, void *base, unsigned long long size, unsigned long alloc_type, unsigned long protect, void *params, unsigned long param_count);
void *ps4rt_MapViewOfFile3(void *mapping, void *process, void *base, unsigned long long offset, unsigned long long view_size, unsigned long alloc_type, unsigned long protect, void *params, unsigned long param_count);
void *ps4rt_CreateFileMapping2(void *file, void *security, unsigned long access, unsigned long protect, unsigned long alloc_attr, unsigned long long maximum, const wchar_t *name, void *params, unsigned long param_count);
int ps4rt_UnmapViewOfFile2(void *process, void *base, unsigned long flags);
#ifndef PROT_READ
#define PROT_NONE 0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#ifndef O_SYNC
#define O_SYNC 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#endif
long long pread(int fd, void *buf, unsigned long long count, long long offset);
long long pwrite(int fd, const void *buf, unsigned long long count, long long offset);
int fsync(int fd);
#endif
#endif
