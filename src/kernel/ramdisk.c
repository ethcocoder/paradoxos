#include "ramdisk.h"
#include "user.h" // For k_strlen
#include <string.h>

#define MAX_RAMDISK_FILES 64

static fs_node_t ramdisk_nodes[MAX_RAMDISK_FILES];
static struct dirent dirent_list[MAX_RAMDISK_FILES];
static int ramdisk_count = 0;

static uint32_t ramdisk_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    if (offset > node->length) return 0;
    if (offset + size > node->length) size = node->length - offset;
    
    uint8_t *data = (uint8_t*)(uintptr_t)node->impl;
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = data[offset + i];
    }
    return size;
}

static struct dirent *ramdisk_readdir(fs_node_t *node, uint32_t index) {
    if (index >= (uint32_t)ramdisk_count) return 0;
    return &dirent_list[index];
}

static fs_node_t *ramdisk_finddir(fs_node_t *node, char *name) {
    for (int i = 0; i < ramdisk_count; i++) {
        // Simple string compare (since we don't have full string.h yet, we'll implement it or use user.h)
        const char* n1 = ramdisk_nodes[i].name;
        const char* n2 = name;
        int match = 1;
        while (*n1 && *n2) {
            if (*n1 != *n2) { match = 0; break; }
            n1++; n2++;
        }
        if (match && *n1 == 0 && *n2 == 0) return &ramdisk_nodes[i];
    }
    return 0;
}

static void ramdisk_add_file(char *name, char *contents) {
    if (ramdisk_count >= MAX_RAMDISK_FILES) return;
    
    fs_node_t *node = &ramdisk_nodes[ramdisk_count];
    int i = 0;
    while(name[i] && i < MAX_FILENAME-1) { node->name[i] = name[i]; i++; }
    node->name[i] = 0;
    
    node->length = k_strlen(contents);
    node->impl = (uint64_t)(uintptr_t)contents;
    node->flags = FS_FILE;
    node->read = ramdisk_read;
    
    // Update dirent
    int j = 0;
    while(name[j] && j < MAX_FILENAME-1) { dirent_list[ramdisk_count].name[j] = name[j]; j++; }
    dirent_list[ramdisk_count].name[j] = 0;
    dirent_list[ramdisk_count].inode = ramdisk_count;
    
    ramdisk_count++;
}

fs_node_t *ramdisk_init() {
    // Root Directory
    static fs_node_t root;
    root.flags = FS_DIRECTORY;
    root.readdir = ramdisk_readdir;
    root.finddir = ramdisk_finddir;
    
    // Some mock files for Phase 4
    ramdisk_add_file("welcome.txt", "Welcome to ParadoxOS!\nThis is the future of AI operating systems.");
    ramdisk_add_file("admin.cfg", "USER=admin\nPASS=paradox");
    ramdisk_add_file("readme.md", "# Paradox Intelligence\nNeural OS initialized.");
    
    return &root;
}
