#include <memory>
extern "C" {
#include <wasm.h>
}
#include "runtime.h"
#include "host.h"

bool check_wasm_func_by_name(const wasm_exporttype_t *export_type, const char *func_name);

int index_of_function(wasm_module_t *module, wasm_exporttype_vec_t &export_types, const char* func_name);

Runtime::Runtime(Host* host)
    :host(host), wasm_module(nullptr), wasm_instance(nullptr)
{
    wasm_engine = wasm_engine_new();
    wasm_store = wasm_store_new(wasm_engine);

    wasm_functype_t* sender_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
    wasm_func_t* sender_opcode = wasm_func_new(wasm_store, sender_type, sender);
    wasm_functype_delete(sender_type);

    wasm_functype_t* balance_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* balance_opcode = wasm_func_new(wasm_store, balance_type, balance);
    wasm_functype_delete(balance_type);

    wasm_functype_t* transfer_type = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* transfer_opcode = wasm_func_new(wasm_store, transfer_type, transfer);
    wasm_functype_delete(transfer_type);

    wasm_functype_t* blocktime_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
    wasm_func_t* blocktime_opcode = wasm_func_new(wasm_store, blocktime_type, blocktime);
    wasm_functype_delete(blocktime_type);
    
    wasm_externs = new wasm_extern_t*[4];
    wasm_externs[0] = wasm_func_as_extern(sender_opcode);
    wasm_externs[1] = wasm_func_as_extern(balance_opcode);
    wasm_externs[2] = wasm_func_as_extern(transfer_opcode);
    wasm_externs[3] = wasm_func_as_extern(blocktime_opcode);

    chain_opcodes = WASM_ARRAY_VEC(wasm_externs);
}

Runtime::~Runtime()
{
    wasm_store_delete(wasm_store);
    wasm_engine_delete(wasm_engine);

    delete[] wasm_externs;
}

void Runtime::reset_session()
{
    if(wasm_instance)
    {
        wasm_instance_delete(wasm_instance);
        wasm_instance = nullptr;
    }
    if(wasm_module)
    {
        wasm_module_delete(wasm_module);
        wasm_module = nullptr;
    }
}
/*
    data layout:
    - First 16 bytes are for function name
    - The remaining are for call params
*/
bool Runtime::execute(address sender, address contract_address, uint32_t value, uint32_t nonce, uint32_t data_size, char *data) 
{
    printf("Sender 0x%x execute SmartContract 0x%x |%p| \n", sender, contract_address, data);
    context.sender = sender;
    char function_name[16];
    int start_func_name_index = -1;
    for (int i = 1; i < 16; ++i)
        if (data[i] != ' ')
        {
            start_func_name_index = i;
            break;
        }
    
    if (start_func_name_index == -1)
        return false;
    
    int func_name_length = 16 - start_func_name_index;
    memcpy(function_name, data + start_func_name_index, func_name_length);
    printf("Funcname: %s\n", function_name);

    create_wasm_module(contract_address);
    create_wasm_instance();

    wasm_exporttype_vec_t export_types;
    wasm_module_exports(wasm_module, &export_types);

    int func_index = index_of_function(wasm_module, export_types, function_name);
    printf("Func index %d \n", func_index);
    
    const wasm_externtype_t* extern_type = wasm_exporttype_type(export_types.data[func_index]);
    if (wasm_externtype_kind(extern_type)== WASM_EXTERN_FUNC)
    {
        const wasm_functype_t* func_type = wasm_externtype_as_functype_const(extern_type);
        const wasm_valtype_vec_t* params = wasm_functype_params(func_type);
        
        for(int j = 0; j < params->size; j++)
        {
            wasm_valtype_t *val_type = params->data[j];
            wasm_valkind_t valkind = wasm_valtype_kind(val_type);                
        }
        const wasm_valtype_vec_t* results = wasm_functype_results(func_type);
        printf("Found a function: params: %d, result: %d\n", params->size, results->size);

    }
    const wasm_functype_t* func_type;
    
    return true;
}

Runtime* Runtime::new_runtime(Host *host)
{
    Runtime *runtime = new Runtime(host);
    return runtime;
}

void  Runtime::del_runtime(Runtime *runtime)
{
    delete runtime;
}

void Runtime::create_wasm_module(address contract_address)
{
    std::shared_ptr<Account> account = host->account_by_address(contract_address);
    wasm_byte_vec_t wasm_bytes;
    if (account)
    {
        int code_size = account->code_size;
        wasm_byte_t* code = account->code;
        wasm_byte_vec_new(&wasm_bytes, code_size, code);
        wasm_module = wasm_module_new(wasm_store, &wasm_bytes);
        wasm_byte_vec_delete(&wasm_bytes);
    }    
}

void Runtime::create_wasm_instance()
{
    if (wasm_module)
        wasm_instance = wasm_instance_new(wasm_store, wasm_module, &chain_opcodes, NULL);
}

wasm_trap_t* sender(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{    
    return nullptr;
}

wasm_trap_t* balance(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    return nullptr;
}

wasm_trap_t* transfer(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    return nullptr;
}

wasm_trap_t* blocktime(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    return nullptr;
}

int index_of_function(wasm_module_t* module, wasm_exporttype_vec_t &export_types, const char* function_name)
{
    for (int i = 0; i < export_types.size; ++i)
    {
        if (check_wasm_func_by_name(export_types.data[i], function_name))
        {
            return i;
        }
    }
    return -1;
}

bool check_wasm_func_by_name(const wasm_exporttype_t *export_type, const char *func_name)
{
    const wasm_name_t *name = wasm_exporttype_name(export_type);
    return memcmp(name->data, func_name, name->size) == 0;    
}