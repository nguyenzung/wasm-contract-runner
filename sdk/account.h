#ifndef SDK_ACCOUNT_H
#define SDK_ACCOUNT_H

#include "../common/account.h"

#include <emscripten.h>


// #include <stdint.h>
extern uint32_t sender();

extern uint32_t balance(address user);

extern uint32_t transfer(address to, uint32_t amount);

extern uint32_t blocktime();

EMSCRIPTEN_KEEPALIVE
void keep_extern_func_order()
{
    sender();
    balance(0);
    transfer(0, 0);
    blocktime();
}

#endif