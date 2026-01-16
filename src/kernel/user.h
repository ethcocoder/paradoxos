#ifndef USER_H
#define USER_H

#include <stdint.h>

#define MAX_USERS 10
#define MAX_NAME_LEN 32

typedef struct {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    int is_active;
} user_t;

typedef enum {
    SYS_STATE_LOGIN,
    SYS_STATE_REGISTER,
    SYS_STATE_DESKTOP,
    SYS_STATE_LOCKED
} system_state_t;

void user_init();
int user_register(const char* username, const char* password);
int user_login(const char* username, const char* password);
user_t* user_get_current();
int k_strlen(const char* s);

#endif
