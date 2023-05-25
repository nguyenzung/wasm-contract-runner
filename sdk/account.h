#ifndef SDK_ACCOUNT_H
#define SDK_ACCOUNT_H

#include "../common/account.h"

#include <stdint.h>

extern uint32_t sender();
extern uint32_t balance(address user);
extern uint32_t transfer(address to, uint32_t amount);
extern uint32_t time();

#endif