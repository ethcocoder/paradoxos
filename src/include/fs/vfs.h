#ifndef _FS_VFS_H
#define _FS_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Define ssize_t for kernel use
typedef long ssize_t;

// Error codes
#define ENOSYS  38  // Function not implemented
#define ENOSPC  28  // No space left on device

// File types
#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_SYMLINK     0x04
#define VFS_DEVICE      0x08

// File permissions
#define VFS_PERM_READ   0x01
#define VFS_PERM_WRITE  0x02
#define VFS_PERM_EXEC   0x04

// File descriptor flags
#define VFS_O_RDONLY    0x00
#define VFS_O_WRONLY    0x01
#define VFS_O_RDWR      0x02
#define VFS_O_CREAT     0x40
#define VFS_O_TRUNC     0x200
#define VFS_O_APPEND    0x400

// Seek modes
#define VFS_SEEK_SET    0
#define VFS_SEEK_CUR    1
#define VFS_SEEK_END    2

// Maximum values
#define VFS_MAX_PATH    4096
#define VFS_MAX_NAME    256
#define VFS_MAX_FDS     256

// File information structure
struct vfs_stat {
    uint64_t size;
    uint32_t type;
    uint32_t permissions;
    uint64_t created_time;
    uint64_t modified_time;
    uint64_t accessed_time;
};

// Directory entry
struct vfs_dirent {
    char name[VFS_MAX_NAME];
    uint32_t type;
    uint64_t size;
};

// File descriptor
struct vfs_fd {
    bool in_use;
    uint32_t flags;
    uint64_t position;
    void *private_data;
    struct vfs_node *node;
};

// Forward declarations
struct vfs_node;
struct vfs_filesystem;

// File operations
struct vfs_file_ops {
    int (*open)(struct vfs_node *node, uint32_t flags);
    int (*close)(struct vfs_node *node);
    ssize_t (*read)(struct vfs_node *node, void *buffer, size_t size, uint64_t offset);
    ssize_t (*write)(struct vfs_node *node, const void *buffer, size_t size, uint64_t offset);
    int (*ioctl)(struct vfs_node *node, uint32_t cmd, void *arg);
};

// Directory operations
struct vfs_dir_ops {
    int (*readdir)(struct vfs_node *node, struct vfs_dirent *dirent, uint64_t index);
    struct vfs_node *(*lookup)(struct vfs_node *node, const char *name);
    int (*create)(struct vfs_node *parent, const char *name, uint32_t type);
    int (*unlink)(struct vfs_node *parent, const char *name);
    int (*mkdir)(struct vfs_node *parent, const char *name);
    int (*rmdir)(struct vfs_node *parent, const char *name);
};

// VFS node (inode equivalent)
struct vfs_node {
    char name[VFS_MAX_NAME];
    uint32_t type;
    uint32_t permissions;
    uint64_t size;
    uint64_t created_time;
    uint64_t modified_time;
    uint64_t accessed_time;
    
    struct vfs_node *parent;
    struct vfs_node *children;
    struct vfs_node *next_sibling;
    
    struct vfs_file_ops *file_ops;
    struct vfs_dir_ops *dir_ops;
    struct vfs_filesystem *filesystem;
    
    void *private_data;
    uint32_t ref_count;
};

// Filesystem operations
struct vfs_fs_ops {
    int (*mount)(struct vfs_filesystem *fs, const char *device);
    int (*unmount)(struct vfs_filesystem *fs);
    struct vfs_node *(*get_root)(struct vfs_filesystem *fs);
    int (*sync)(struct vfs_filesystem *fs);
};

// Filesystem structure
struct vfs_filesystem {
    char name[64];
    struct vfs_fs_ops *ops;
    struct vfs_node *root;
    void *private_data;
    bool mounted;
};

// Mount point
struct vfs_mount {
    char path[VFS_MAX_PATH];
    struct vfs_filesystem *filesystem;
    struct vfs_node *mount_point;
};

// VFS functions
int vfs_init(void);
int vfs_mount(const char *device, const char *path, const char *fstype);
int vfs_unmount(const char *path);

// File operations
int vfs_open(const char *path, uint32_t flags);
int vfs_close(int fd);
ssize_t vfs_read(int fd, void *buffer, size_t size);
ssize_t vfs_write(int fd, const void *buffer, size_t size);
int64_t vfs_seek(int fd, int64_t offset, int whence);
int vfs_stat(const char *path, struct vfs_stat *stat);
int vfs_fstat(int fd, struct vfs_stat *stat);

// Directory operations
int vfs_readdir(int fd, struct vfs_dirent *dirent, uint64_t index);
int vfs_mkdir(const char *path);
int vfs_rmdir(const char *path);
int vfs_unlink(const char *path);

// Path operations
struct vfs_node *vfs_resolve_path(const char *path);
char *vfs_get_absolute_path(const char *path);

// Filesystem registration
int vfs_register_filesystem(struct vfs_filesystem *fs);
int vfs_unregister_filesystem(const char *name);

#endif