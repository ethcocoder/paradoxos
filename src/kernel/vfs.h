#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_FILENAME 256
#define MAX_FILES 1024

typedef enum {
    FS_FILE,
    FS_DIRECTORY,
    FS_DEVICE
} fs_node_type_t;

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent * (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node * (*finddir_type_t)(struct fs_node*, char *name);

typedef struct fs_node {
    char name[MAX_FILENAME];
    uint32_t mask;        // Permissions
    uint32_t uid;         // User ID
    uint32_t gid;         // Group ID
    uint32_t flags;       // Node type
    uint32_t inode;       // Device specific
    uint32_t length;      // Size in bytes
    uint32_t impl;        // Implementation defined
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    struct fs_node *ptr;  // Used by mountpoints and symlinks
} fs_node_t;

struct dirent {
    char name[MAX_FILENAME];
    uint32_t inode;
};

extern fs_node_t *fs_root; // Root of the file system

// Helper functions
uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void vfs_open(fs_node_t *node);
void vfs_close(fs_node_t *node);
struct dirent *vfs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *vfs_finddir(fs_node_t *node, char *name);

#endif
