#include <syscall.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <arch/idt.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <fs/vfs.h>
#include <stdbool.h>

// System call table
typedef int64_t (*syscall_func_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

static syscall_func_t syscall_table[256];
static int current_errno = 0;

// Parameter validation
static bool validate_user_pointer(const void *ptr, size_t size) {
    if (!ptr) return false;
    
    // Check if pointer is in user space (below kernel space)
    uint64_t addr = (uint64_t)ptr;
    if (addr >= 0xffff800000000000) return false; // Kernel space
    
    // Check if the entire range is accessible
    // For now, just basic checks - in a real OS you'd walk page tables
    if (addr + size < addr) return false; // Overflow check
    
    return true;
}

static bool validate_string(const char *str, size_t max_len) {
    if (!validate_user_pointer(str, 1)) return false;
    
    // Check string length and ensure it's null-terminated within max_len
    for (size_t i = 0; i < max_len; i++) {
        if (str[i] == '\0') return true;
    }
    return false; // String too long or not null-terminated
}

// System call implementations
int64_t sys_exit(int status) {
    printk("Process exit with status: %d\n", status);
    // TODO: Implement proper process termination
    return 0;
}

int64_t sys_fork(void) {
    printk("Fork system call not yet implemented\n");
    current_errno = ENOSYS;
    return -1;
}

int64_t sys_read(int fd, void *buf, size_t count) {
    if (!validate_user_pointer(buf, count)) {
        current_errno = EFAULT;
        return -1;
    }
    
    if (fd < 0) {
        current_errno = EBADF;
        return -1;
    }
    
    // For now, just return 0 (EOF) for all reads
    printk("Read system call: fd=%d, count=%zu\n", fd, count);
    return 0;
}

int64_t sys_write(int fd, const void *buf, size_t count) {
    if (!validate_user_pointer(buf, count)) {
        current_errno = EFAULT;
        return -1;
    }
    
    if (fd < 0) {
        current_errno = EBADF;
        return -1;
    }
    
    // Handle stdout/stderr
    if (fd == 1 || fd == 2) {
        // Write to console
        const char *str = (const char *)buf;
        for (size_t i = 0; i < count; i++) {
            printk("%c", str[i]);
        }
        return count;
    }
    
    printk("Write system call: fd=%d, count=%zu\n", fd, count);
    return count; // Pretend we wrote everything
}

int64_t sys_open(const char *pathname, int flags, int mode) {
    if (!validate_string(pathname, 4096)) {
        current_errno = EFAULT;
        return -1;
    }
    
    printk("Open system call: path=%s, flags=%d, mode=%d\n", pathname, flags, mode);
    // TODO: Implement file system
    current_errno = ENOENT;
    return -1;
}

int64_t sys_close(int fd) {
    if (fd < 0) {
        current_errno = EBADF;
        return -1;
    }
    
    printk("Close system call: fd=%d\n", fd);
    return 0;
}

int64_t sys_getpid(void) {
    // TODO: Implement proper process management
    return 1; // Return PID 1 for now
}

int64_t sys_brk(void *addr) {
    printk("Brk system call: addr=0x%p\n", addr);
    // TODO: Implement heap management
    return (int64_t)addr;
}

int64_t sys_paradox_version(void) {
    return (PARADOX_ABI_MAJOR << 16) | (PARADOX_ABI_MINOR << 8) | PARADOX_ABI_PATCH;
}

struct paradox_info {
    uint32_t abi_version;
    uint32_t kernel_version;
    uint64_t total_memory;
    uint64_t free_memory;
    char kernel_name[32];
};

int64_t sys_paradox_info(void *info_struct) {
    if (!validate_user_pointer(info_struct, sizeof(struct paradox_info))) {
        current_errno = EFAULT;
        return -1;
    }
    
    struct paradox_info *info = (struct paradox_info *)info_struct;
    info->abi_version = sys_paradox_version();
    info->kernel_version = 0x010000; // Version 1.0.0
    info->total_memory = 0x10000000; // 256MB for demo
    info->free_memory = 0x8000000;   // 128MB free for demo
    strcpy(info->kernel_name, "ParadoxOS");
    
    return 0;
}

// Generic system call wrapper functions
static int64_t syscall_wrapper_0(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_exit((int)a1);
}

static int64_t syscall_wrapper_1(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_fork();
}

static int64_t syscall_wrapper_2(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a4; (void)a5; (void)a6;
    return sys_read((int)a1, (void *)a2, (size_t)a3);
}

static int64_t syscall_wrapper_3(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a4; (void)a5; (void)a6;
    return sys_write((int)a1, (const void *)a2, (size_t)a3);
}

static int64_t syscall_wrapper_4(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a4; (void)a5; (void)a6;
    return sys_open((const char *)a1, (int)a2, (int)a3);
}

static int64_t syscall_wrapper_5(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_close((int)a1);
}

static int64_t syscall_wrapper_18(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_getpid();
}

static int64_t syscall_wrapper_39(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_brk((void *)a1);
}

static int64_t syscall_wrapper_100(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_paradox_version();
}

static int64_t syscall_wrapper_101(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
    (void)a2; (void)a3; (void)a4; (void)a5; (void)a6;
    return sys_paradox_info((void *)a1);
}

// Main system call handler
int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, 
                       uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    
    // Reset errno
    current_errno = 0;
    
    // Validate system call number
    if (syscall_num >= 256) {
        current_errno = ENOSYS;
        return -1;
    }
    
    // Check if system call is implemented
    if (!syscall_table[syscall_num]) {
        printk("Unimplemented system call: %lu\n", syscall_num);
        current_errno = ENOSYS;
        return -1;
    }
    
    // Log system call for debugging
    printk("System call %lu called with args: %lx, %lx, %lx, %lx, %lx, %lx\n", 
           syscall_num, arg1, arg2, arg3, arg4, arg5, arg6);
    
    // Call the system call
    int64_t result = syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5, arg6);
    
    // Set errno if there was an error
    if (result == -1 && current_errno != 0) {
        // TODO: Set errno in user space
    }
    
    return result;
}

void syscall_init(void) {
    // Initialize system call table
    memset(syscall_table, 0, sizeof(syscall_table));
    
    // Register system calls
    syscall_table[SYS_EXIT] = syscall_wrapper_0;
    syscall_table[SYS_FORK] = syscall_wrapper_1;
    syscall_table[SYS_READ] = syscall_wrapper_2;
    syscall_table[SYS_WRITE] = syscall_wrapper_3;
    syscall_table[SYS_OPEN] = syscall_wrapper_4;
    syscall_table[SYS_CLOSE] = syscall_wrapper_5;
    syscall_table[SYS_GETPID] = syscall_wrapper_18;
    syscall_table[SYS_BRK] = syscall_wrapper_39;
    syscall_table[SYS_PARADOX_VERSION] = syscall_wrapper_100;
    syscall_table[SYS_PARADOX_INFO] = syscall_wrapper_101;
    
    // Install system call interrupt handler (INT 0x80)
    idt_set_descriptor(0x80, (void *)syscall_entry, 0xEE); // User-accessible interrupt gate
    
    printk("[SYSCALL] System call interface initialized\n");
    printk("[SYSCALL] ABI Version: %d.%d.%d\n", PARADOX_ABI_MAJOR, PARADOX_ABI_MINOR, PARADOX_ABI_PATCH);
    printk("[SYSCALL] Registered %d system calls\n", 10);
}