#include "user.h"
#include <stddef.h>

static user_t user_db[MAX_USERS];
static int user_count = 0;
static user_t* current_user = NULL;

/* Simple string comparison for freestanding kernel */
static int k_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static void k_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

void user_init() {
    // Create a default administrator user for production testing
    user_register("admin", "1234");
}

int user_register(const char* username, const char* password) {
    if (user_count >= MAX_USERS) return 0;
    
    k_strcpy(user_db[user_count].username, username);
    k_strcpy(user_db[user_count].password, password);
    user_db[user_count].is_active = 1;
    user_count++;
    return 1;
}

int user_login(const char* username, const char* password) {
    for (int i = 0; i < user_count; i++) {
        if (k_strcmp(user_db[i].username, username) == 0 &&
            k_strcmp(user_db[i].password, password) == 0) {
            current_user = &user_db[i];
            return 1;
        }
    }
    return 0;
}

user_t* user_get_current() {
    return current_user;
}
