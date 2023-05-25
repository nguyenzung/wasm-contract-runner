#ifndef RUNTIME_H
#define RUNTIME_H

#include "chain_types.h"

extern "C" {
#include <wasm.h>
}

typedef struct Context {
    address sender;
} Context;

class Host;
class Runtime 
{
protected:
    Host *host;
    wasm_engine_t* wasm_engine;
    wasm_store_t* wasm_store;
    wasm_module_t* wasm_module;
    wasm_instance_t* wasm_instance;
    
    wasm_extern_t** wasm_externs;
    wasm_extern_vec_t chain_opcodes;

public:
    Context context;

protected:
    Runtime(Host *host);
    ~Runtime();

public:    
    void reset_session();

    bool execute(address sender, address contract_address, uint32_t value, uint32_t nonce, uint32_t data_size, char *data);

    static Runtime* new_runtime(Host *host);

    static void del_runtime(Runtime *runtime);

protected:    
    void create_wasm_module(address contract_address);

    void create_wasm_instance();
};

wasm_trap_t* sender(const wasm_val_vec_t* args, wasm_val_vec_t* results);

wasm_trap_t* balance(const wasm_val_vec_t* args, wasm_val_vec_t* results);

wasm_trap_t* transfer(const wasm_val_vec_t* args, wasm_val_vec_t* results);

wasm_trap_t* blocktime(const wasm_val_vec_t* args, wasm_val_vec_t* results);

#endif