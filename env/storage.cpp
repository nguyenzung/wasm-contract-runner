#include <stdio.h>
#include "storage.h"

StorageEngine::StorageEngine()
{
}

bool StorageEngine::contain(uint64_t key)
{
    return storage.find(key) != storage.end();
}

std::shared_ptr<Account> StorageEngine::load(uint64_t key)
{
    return storage[key];
}

void StorageEngine::store(uint64_t key, std::shared_ptr<Account> account)
{
    storage[key] = account;
}