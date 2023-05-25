#include <stdlib.h>
#include <memory>
extern "C" {
#include <openssl/sha.h>
}
#include "host.h"
#include "storage.h"
#include "runtime.h"
#include "chain_types.h"

#define ACCOUNT_PREFIX  "ACC:"

uint32_t calculate_contract_address(address creator, uint32_t nonce);

Host* Host::_instance = nullptr;

Host::Host()
{
    engine = new StorageEngine();
    runtime = Runtime::new_runtime(this);
}

Host::~Host()
{
    delete engine;
}

Host* Host::instance()
{
    if(!_instance)
        _instance = new Host();
    return _instance;
}

void Host::clear_instance()
{
    if(_instance)
        delete _instance;
    _instance = nullptr;
}

uint32_t Host::balance_of(address addr)
{
    std::shared_ptr<Account> account = account_by_address(addr);
    return account? account->balance: 0;
}

void Host::add_balance(address addr, uint32_t amount)
{
    std::shared_ptr<Account> account = account_by_address(addr);
    if (!account) {
        Account *acc = new Account(addr);
        account = std::shared_ptr<Account>(acc);
    }
    account->balance += amount;
    store_account_by_key(addr, account);
}

bool Host::sub_balance(address addr, uint32_t amount)
{
    std::shared_ptr<Account> account = account_by_address(addr);
    if (!account) {
        Account *acc = new Account(addr);
        account = std::shared_ptr<Account>(acc);
    }
    if(account->balance >= amount)
    {
        account->balance -= amount;
        store_account_by_key(addr, account);
        return true;
    }
    
    return false;
}


uint32_t Host::handle_execute(address sender, address to, uint32_t value, uint32_t nonce, uint32_t data_size, char *data)
{
    // data 
    if(data_size)
    {
        if(to)
            return runtime->execute(sender, to, value, nonce, data_size, data) ? 1 : 0;
        else{   // Deploy smart contract
            auto contract_address = calculate_contract_address(sender, nonce);
            printf("New contract deployed 0x%08x \n", contract_address);
            auto *acc = new Account(contract_address, data_size, data);
            auto account = std::shared_ptr<Account>(acc);
            store_account_by_key(contract_address, account);
            return contract_address;
        }
    } else {
        if (to) // transfer native token
        {
            auto sender_balance = this->balance_of(sender);
            if (sender_balance >= value)
            {
                this->sub_balance(sender, value);
                this->add_balance(to, value);
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

std::shared_ptr<Account> Host::account_by_address(address addr)
{
    uint64_t key = addr;
    if (engine->contain(key))
    {
        auto account = engine->load(key);
        return account;
    } else {
        return std::shared_ptr<Account>(nullptr);
    }   
}

void Host::store_account_by_key(address addr, std::shared_ptr<Account> account)
{
    uint64_t key = (addr);
    engine->store(key, account);
}

uint32_t calculate_contract_address(address creator, uint32_t nonce)
{
    uint64_t input = creator;
    input = (input << 32 | nonce);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256ctx;
    SHA256_Init(&sha256ctx);
    SHA256_Update(&sha256ctx, &input, sizeof(uint64_t));
    SHA256_Final(hash, &sha256ctx);
    return *((uint32_t*)hash);
}