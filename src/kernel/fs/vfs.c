#include <fs/vfs.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <mm/pmm.h>
#include <syscall.h>

// VFS state
static struct vfs_fd file_descriptors[VFS_MAX_FDS];
static struct vfs_mount mounts[16];
static struct vfs_filesystem *filesystems[16];
static struct vfs_node *root_node = NULL;
static bool vfs_initialized = false;
static int next_fd = 3; // 0, 1, 2 reserved for stdin, stdout, stderr

// Helper functions
static struct vfs_node *vfs_create_node(const char *name, uint32_t type) {
    void *phys_node = pmm_alloc(1);
    if (!phys_node) return NULL;
    struct vfs_node *node = (struct vfs_node *)phys_to_virt((uint64_t)phys_node);
    
    memset(node, 0, sizeof(struct vfs_node));
    strncpy(node->name, name, VFS_MAX_NAME - 1);
    node->type = type;
    node->permissions = VFS_PERM_READ | VFS_PERM_WRITE;
    node->ref_count = 1;
    
    // Set default timestamps (simplified)
    node->created_time = 0;
    node->modified_time = 0;
    node->accessed_time = 0;
    
    return node;
}

static void vfs_destroy_node(struct vfs_node *node) {
    if (!node) return;
    
    node->ref_count--;
    if (node->ref_count == 0) {
        pmm_free((void *)virt_to_phys(node), 1);
    }
}

static int vfs_allocate_fd(void) {
    for (int i = 3; i < VFS_MAX_FDS; i++) {
        if (!file_descriptors[i].in_use) {
            return i;
        }
    }
    return -1;
}

static struct vfs_node *vfs_resolve_path_recursive(struct vfs_node *current, const char *path) {
    if (!current || !path) return NULL;
    
    // Skip leading slashes
    while (*path == '/') path++;
    
    // If path is empty, return current node
    if (*path == '\0') return current;
    
    // Find next path component
    const char *next_slash = strchr(path, '/');
    size_t component_len;
    
    if (next_slash) {
        component_len = next_slash - path;
    } else {
        component_len = strlen(path);
    }
    
    // Handle special cases
    if (component_len == 1 && path[0] == '.') {
        // Current directory
        if (next_slash) {
            return vfs_resolve_path_recursive(current, next_slash);
        } else {
            return current;
        }
    }
    
    if (component_len == 2 && path[0] == '.' && path[1] == '.') {
        // Parent directory
        struct vfs_node *parent = current->parent ? current->parent : current;
        if (next_slash) {
            return vfs_resolve_path_recursive(parent, next_slash);
        } else {
            return parent;
        }
    }
    
    // Look for child with matching name
    struct vfs_node *child = current->children;
    while (child) {
        if (strlen(child->name) == component_len && 
            strncmp(child->name, path, component_len) == 0) {
            
            if (next_slash) {
                return vfs_resolve_path_recursive(child, next_slash);
            } else {
                return child;
            }
        }
        child = child->next_sibling;
    }
    
    return NULL; // Not found
}

// Simple in-memory filesystem operations
static ssize_t memfs_read(struct vfs_node *node, void *buffer, size_t size, uint64_t offset) {
    if (!node->private_data) return 0;
    
    if (offset >= node->size) return 0;
    
    size_t to_read = size;
    if (offset + size > node->size) {
        to_read = node->size - offset;
    }
    
    memcpy(buffer, (uint8_t *)node->private_data + offset, to_read);
    return to_read;
}

static ssize_t memfs_write(struct vfs_node *node, const void *buffer, size_t size, uint64_t offset) {
    // For simplicity, allocate fixed-size buffers
    if (!node->private_data) {
        void *phys_buffer = pmm_alloc(1); // 4KB buffer
        if (!phys_buffer) return -1;
        node->private_data = phys_to_virt((uint64_t)phys_buffer);
        memset(node->private_data, 0, PAGE_SIZE);
    }
    
    if (offset + size > PAGE_SIZE) {
        size = PAGE_SIZE - offset;
    }
    
    memcpy((uint8_t *)node->private_data + offset, buffer, size);
    
    if (offset + size > node->size) {
        node->size = offset + size;
    }
    
    return size;
}

static int memfs_readdir(struct vfs_node *node, struct vfs_dirent *dirent, uint64_t index) {
    if (node->type != VFS_DIRECTORY) return -1;
    
    struct vfs_node *child = node->children;
    uint64_t current_index = 0;
    
    while (child && current_index < index) {
        child = child->next_sibling;
        current_index++;
    }
    
    if (!child) return 0; // End of directory
    
    strncpy(dirent->name, child->name, VFS_MAX_NAME - 1);
    dirent->name[VFS_MAX_NAME - 1] = '\0';
    dirent->type = child->type;
    dirent->size = child->size;
    
    return 1; // Success
}

static struct vfs_node *memfs_lookup(struct vfs_node *node, const char *name) {
    if (node->type != VFS_DIRECTORY) return NULL;
    
    struct vfs_node *child = node->children;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next_sibling;
    }
    
    return NULL;
}

static int memfs_create(struct vfs_node *parent, const char *name, uint32_t type) {
    if (parent->type != VFS_DIRECTORY) return -1;
    
    // Check if file already exists
    if (memfs_lookup(parent, name)) return -1;
    
    struct vfs_node *new_node = vfs_create_node(name, type);
    if (!new_node) return -1;
    
    new_node->parent = parent;
    new_node->next_sibling = parent->children;
    parent->children = new_node;
    
    return 0;
}

// Default file and directory operations
static struct vfs_file_ops default_file_ops = {
    .open = NULL,
    .close = NULL,
    .read = memfs_read,
    .write = memfs_write,
    .ioctl = NULL,
};

static struct vfs_dir_ops default_dir_ops = {
    .readdir = memfs_readdir,
    .lookup = memfs_lookup,
    .create = memfs_create,
};

// VFS implementation
int vfs_init(void) {
    // Initialize file descriptor table
    memset(file_descriptors, 0, sizeof(file_descriptors));
    
    // Reserve stdin, stdout, stderr
    file_descriptors[0].in_use = true; // stdin
    file_descriptors[1].in_use = true; // stdout
    file_descriptors[2].in_use = true; // stderr
    
    // Initialize mount table
    memset(mounts, 0, sizeof(mounts));
    memset(filesystems, 0, sizeof(filesystems));
    
    // Create root directory
    root_node = vfs_create_node("/", VFS_DIRECTORY);
    if (!root_node) {
        printk("[VFS] Failed to create root node\n");
        return -1;
    }
    
    root_node->dir_ops = &default_dir_ops;
    
    // Create some basic directories
    memfs_create(root_node, "home", VFS_DIRECTORY);
    memfs_create(root_node, "tmp", VFS_DIRECTORY);
    memfs_create(root_node, "dev", VFS_DIRECTORY);
    memfs_create(root_node, "proc", VFS_DIRECTORY);
    
    // Create some demo files
    struct vfs_node *readme = vfs_create_node("README.txt", VFS_FILE);
    if (readme) {
        readme->parent = root_node;
        readme->next_sibling = root_node->children;
        root_node->children = readme;
        readme->file_ops = &default_file_ops;
        
        const char *content = "Welcome to ParadoxOS!\n\nThis is a custom operating system built from scratch.\n";
        memfs_write(readme, content, strlen(content), 0);
    }
    
    vfs_initialized = true;
    printk("[VFS] Virtual File System initialized\n");
    printk("[VFS] Root filesystem: in-memory filesystem\n");
    
    return 0;
}

struct vfs_node *vfs_resolve_path(const char *path) {
    if (!vfs_initialized || !path) return NULL;
    
    if (path[0] == '/') {
        // Absolute path
        return vfs_resolve_path_recursive(root_node, path);
    } else {
        // Relative path (from root for now)
        return vfs_resolve_path_recursive(root_node, path);
    }
}

int vfs_open(const char *path, uint32_t flags) {
    if (!vfs_initialized) return -1;
    
    struct vfs_node *node = vfs_resolve_path(path);
    
    // Handle file creation
    if (!node && (flags & VFS_O_CREAT)) {
        // Extract directory and filename
        void *phys_path_copy = pmm_alloc(1);
        if (!phys_path_copy) return -1;
        char *path_copy = (char *)phys_to_virt((uint64_t)phys_path_copy);
        
        strncpy(path_copy, path, PAGE_SIZE - 1);
        path_copy[PAGE_SIZE - 1] = '\0';
        
        char *last_slash = strrchr(path_copy, '/');
        if (last_slash) {
            *last_slash = '\0';
            const char *filename = last_slash + 1;
            const char *dirname = path_copy;
            
            struct vfs_node *parent = vfs_resolve_path(dirname);
            if (parent && parent->dir_ops && parent->dir_ops->create) {
                if (parent->dir_ops->create(parent, filename, VFS_FILE) == 0) {
                    node = vfs_resolve_path(path);
                }
            }
        }
        
        pmm_free((void *)virt_to_phys(path_copy), 1);
    }
    
    if (!node) return -ENOENT;
    
    // Allocate file descriptor
    int fd = vfs_allocate_fd();
    if (fd < 0) return -EMFILE;
    
    struct vfs_fd *file_desc = &file_descriptors[fd];
    file_desc->in_use = true;
    file_desc->flags = flags;
    file_desc->position = 0;
    file_desc->node = node;
    file_desc->private_data = NULL;
    
    node->ref_count++;
    
    // Handle truncation
    if (flags & VFS_O_TRUNC && node->type == VFS_FILE) {
        node->size = 0;
    }
    
    printk("[VFS] Opened '%s' as fd %d\n", path, fd);
    return fd;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !file_descriptors[fd].in_use) {
        return -EBADF;
    }
    
    struct vfs_fd *file_desc = &file_descriptors[fd];
    
    if (file_desc->node) {
        vfs_destroy_node(file_desc->node);
    }
    
    memset(file_desc, 0, sizeof(struct vfs_fd));
    
    return 0;
}

ssize_t vfs_read(int fd, void *buffer, size_t size) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !file_descriptors[fd].in_use) {
        return -EBADF;
    }
    
    struct vfs_fd *file_desc = &file_descriptors[fd];
    struct vfs_node *node = file_desc->node;
    
    if (!node || !node->file_ops || !node->file_ops->read) {
        return -ENOSYS;
    }
    
    ssize_t bytes_read = node->file_ops->read(node, buffer, size, file_desc->position);
    if (bytes_read > 0) {
        file_desc->position += bytes_read;
    }
    
    return bytes_read;
}

ssize_t vfs_write(int fd, const void *buffer, size_t size) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !file_descriptors[fd].in_use) {
        return -EBADF;
    }
    
    struct vfs_fd *file_desc = &file_descriptors[fd];
    struct vfs_node *node = file_desc->node;
    
    if (!node || !node->file_ops || !node->file_ops->write) {
        return -ENOSYS;
    }
    
    uint64_t write_pos = file_desc->position;
    if (file_desc->flags & VFS_O_APPEND) {
        write_pos = node->size;
    }
    
    ssize_t bytes_written = node->file_ops->write(node, buffer, size, write_pos);
    if (bytes_written > 0) {
        if (!(file_desc->flags & VFS_O_APPEND)) {
            file_desc->position += bytes_written;
        } else {
            file_desc->position = node->size;
        }
    }
    
    return bytes_written;
}

int vfs_stat(const char *path, struct vfs_stat *stat) {
    if (!vfs_initialized || !path || !stat) return -EINVAL;
    
    struct vfs_node *node = vfs_resolve_path(path);
    if (!node) return -ENOENT;
    
    stat->size = node->size;
    stat->type = node->type;
    stat->permissions = node->permissions;
    stat->created_time = node->created_time;
    stat->modified_time = node->modified_time;
    stat->accessed_time = node->accessed_time;
    
    return 0;
}

int vfs_readdir(int fd, struct vfs_dirent *dirent, uint64_t index) {
    if (fd < 0 || fd >= VFS_MAX_FDS || !file_descriptors[fd].in_use) {
        return -EBADF;
    }
    
    struct vfs_fd *file_desc = &file_descriptors[fd];
    struct vfs_node *node = file_desc->node;
    
    if (!node || node->type != VFS_DIRECTORY || !node->dir_ops || !node->dir_ops->readdir) {
        return -ENOTDIR;
    }
    
    return node->dir_ops->readdir(node, dirent, index);
}

int vfs_mkdir(const char *path) {
    if (!vfs_initialized || !path) return -EINVAL;
    
    // Extract parent directory and new directory name
    char *path_copy = (char *)pmm_alloc(1);
    if (!path_copy) return -ENOMEM;
    
    strncpy(path_copy, path, PAGE_SIZE - 1);
    path_copy[PAGE_SIZE - 1] = '\0';
    
    char *last_slash = strrchr(path_copy, '/');
    if (!last_slash) {
        pmm_free(path_copy, 1);
        return -EINVAL;
    }
    
    *last_slash = '\0';
    const char *dirname = last_slash + 1;
    const char *parent_path = path_copy;
    
    struct vfs_node *parent = vfs_resolve_path(parent_path);
    if (!parent || parent->type != VFS_DIRECTORY) {
        pmm_free(path_copy, 1);
        return -ENOTDIR;
    }
    
    int result = -ENOSYS;
    if (parent->dir_ops && parent->dir_ops->create) {
        result = parent->dir_ops->create(parent, dirname, VFS_DIRECTORY);
    }
    
    pmm_free(path_copy, 1);
    return result;
}