#ifndef STORAGE_H
#define STORAGE_H

#include "chain_types.h"
#include <unordered_map>
#include <string>
#include <memory>

class StorageEngine {
    std::unordered_map<uint64_t, std::shared_ptr<Account>> storage;

public:
    StorageEngine();

    bool contain(uint64_t key);

    std::shared_ptr<Account> load(uint64_t key);

    void store(uint64_t key, std::shared_ptr<Account> account);
};

#endif