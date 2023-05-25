#include "chain_types.h"
#include <cstring>
#include <stdio.h>

Account::Account(address addr): addr(addr), nonce(0),balance(0),code_size(0), code(nullptr)
{
    // std::memcpy(this->addr, addr, 65);
}

Account::Account(address addr, uint32_t code_size, char *code): addr(addr), nonce(0), balance(0), code_size(code_size), code(code)
{
    // std::memcpy(this->addr, addr, 65);
    this->code = new char[code_size];
    memcpy(this->code, code, code_size);
}

Account::~Account()
{
    printf("Clean account\n");
    if (code)
        delete[] code;
}

