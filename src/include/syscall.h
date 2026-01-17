#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>
#include <stddef.h>

// System call numbers
#define SYS_EXIT        0
#define SYS_FORK        1
#define SYS_READ        2
#define SYS_WRITE       3
#define SYS_OPEN        4
#define SYS_CLOSE       5
#define SYS_WAITPID     6
#define SYS_CREAT       7
#define SYS_LINK        8
#define SYS_UNLINK      9
#define SYS_EXECVE      10
#define SYS_CHDIR       11
#define SYS_TIME        12
#define SYS_MKNOD       13
#define SYS_CHMOD       14
#define SYS_LCHOWN      15
#define SYS_STAT        16
#define SYS_LSEEK       17
#define SYS_GETPID      18
#define SYS_MOUNT       19
#define SYS_UMOUNT      20
#define SYS_SETUID      21
#define SYS_GETUID      22
#define SYS_STIME       23
#define SYS_PTRACE      24
#define SYS_ALARM       25
#define SYS_FSTAT       26
#define SYS_PAUSE       27
#define SYS_UTIME       28
#define SYS_ACCESS      29
#define SYS_NICE        30
#define SYS_SYNC        31
#define SYS_KILL        32
#define SYS_RENAME      33
#define SYS_MKDIR       34
#define SYS_RMDIR       35
#define SYS_DUP         36
#define SYS_PIPE        37
#define SYS_TIMES       38
#define SYS_BRK         39
#define SYS_SETGID      40
#define SYS_GETGID      41
#define SYS_SIGNAL      42
#define SYS_GETEUID     43
#define SYS_GETEGID     44
#define SYS_ACCT        45
#define SYS_UMOUNT2     46
#define SYS_IOCTL       47
#define SYS_FCNTL       48
#define SYS_SETPGID     49
#define SYS_ULIMIT      50

// ParadoxOS specific system calls
#define SYS_PARADOX_VERSION     100
#define SYS_PARADOX_INFO        101
#define SYS_PARADOX_DEBUG       102

// ABI Version
#define PARADOX_ABI_MAJOR       1
#define PARADOX_ABI_MINOR       0
#define PARADOX_ABI_PATCH       0

// System call return values
#define SYSCALL_SUCCESS         0
#define SYSCALL_ERROR          -1

// Error codes
#define EPERM           1       // Operation not permitted
#define ENOENT          2       // No such file or directory
#define ESRCH           3       // No such process
#define EINTR           4       // Interrupted system call
#define EIO             5       // I/O error
#define ENXIO           6       // No such device or address
#define E2BIG           7       // Argument list too long
#define ENOEXEC         8       // Exec format error
#define EBADF           9       // Bad file number
#define ECHILD          10      // No child processes
#define EAGAIN          11      // Try again
#define ENOMEM          12      // Out of memory
#define EACCES          13      // Permission denied
#define EFAULT          14      // Bad address
#define ENOTBLK         15      // Block device required
#define EBUSY           16      // Device or resource busy
#define EEXIST          17      // File exists
#define EXDEV           18      // Cross-device link
#define ENODEV          19      // No such device
#define ENOTDIR         20      // Not a directory
#define EISDIR          21      // Is a directory
#define EINVAL          22      // Invalid argument
#define ENFILE          23      // File table overflow
#define EMFILE          24      // Too many open files
#define ENOTTY          25      // Not a typewriter
#define ETXTBSY         26      // Text file busy
#define EFBIG           27      // File too large
#define ENOSPC          28      // No space left on device
#define ESPIPE          29      // Illegal seek
#define EROFS           30      // Read-only file system
#define EMLINK          31      // Too many links
#define EPIPE           32      // Broken pipe
#define EDOM            33      // Math argument out of domain of func
#define ERANGE          34      // Math result not representable

// System call interface
void syscall_init(void);
int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, 
                       uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

// Individual system call implementations
int64_t sys_exit(int status);
int64_t sys_fork(void);
int64_t sys_read(int fd, void *buf, size_t count);
int64_t sys_write(int fd, const void *buf, size_t count);
int64_t sys_open(const char *pathname, int flags, int mode);
int64_t sys_close(int fd);
int64_t sys_getpid(void);
int64_t sys_brk(void *addr);
int64_t sys_paradox_version(void);
int64_t sys_paradox_info(void *info_struct);

#endif