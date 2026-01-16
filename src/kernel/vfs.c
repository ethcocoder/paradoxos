#include "vfs.h"

fs_node_t *fs_root = 0;

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !node->read) return 0;
    return node->read(node, offset, size, buffer);
}

uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (!node || !node->write) return 0;
    return node->write(node, offset, size, buffer);
}

void vfs_open(fs_node_t *node) {
    if (node->open != 0)
        return node->open(node);
}

void vfs_close(fs_node_t *node) {
    if (node->close != 0)
        return node->close(node);
}

struct dirent *vfs_readdir(fs_node_t *node, uint32_t index) {
    if (node && (node->flags & 0x07) == FS_DIRECTORY && node->readdir != 0)
        return node->readdir(node, index);
    else
        return 0;
}

fs_node_t *vfs_finddir(fs_node_t *node, char *name) {
    if (node && (node->flags & 0x07) == FS_DIRECTORY && node->finddir != 0)
        return node->finddir(node, name);
    else
        return 0;
}
