#ifndef HOST_H
#define HOST_H

#include "../common/account.h"
#include "chain_types.h"
#include <stdint.h>
#include <memory>

class StorageEngine;
class Runtime;

// extern "C" {
class Host{
    StorageEngine *engine;
    Runtime *_runtime;
    
    static Host *_instance;

    Host();
    ~Host();

public:
    static Host* instance();

    static void clear_instance();

    Runtime *runtime();

    uint32_t balance_of(address addr);

    void add_balance(address addr, uint32_t amount);

    bool sub_balance(address addr, uint32_t amount);

    uint32_t handle_execute(address sender, address to, uint32_t value, uint32_t nonce, uint32_t data_size, char *data);

    std::shared_ptr<Account> account_by_address(address addr);

    void store_account_by_key(address addr, std::shared_ptr<Account> account);
};
// }




#endif