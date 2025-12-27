#ifndef AUTH_STORAGE_H
#define AUTH_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

void auth_load(char *user, uint8_t pass_hash[32]);
bool auth_save(const char *user, const char *pass);
void auth_factory_reset(void);


#endif
