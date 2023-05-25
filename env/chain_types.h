#ifndef CHAIN_TYPES_H
#define CHAIN_TYPES_H

#include "../common/account.h"
#include <stdint.h>

class Account {
public:
    address addr;
    uint32_t nonce;
    uint32_t balance;
    uint32_t code_size;
    char* code;

    Account(address addr);

    Account(address addr, uint32_t code_size, char *code);

    ~Account();

};

#endif