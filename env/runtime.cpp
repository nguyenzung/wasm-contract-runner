#include <memory>
extern "C" {
#include <wasm.h>
}
#include "runtime.h"
#include "host.h"

bool check_wasm_func_by_name(const wasm_exporttype_t *export_type, const char *func_name, int func_name_length);

int index_of_function(wasm_module_t *module, wasm_exporttype_vec_t &export_types, const char* func_name, int func_name_length);

// wasm_val_t build_data(wasm_valkind_t ); 

Runtime::Runtime(Host* host)
    :host(host), wasm_module(nullptr), wasm_instance(nullptr)
{
    wasm_engine = wasm_engine_new();
    wasm_store = wasm_store_new(wasm_engine);

    wasm_functype_t* sender_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
    wasm_func_t* sender_opcode = wasm_func_new(wasm_store, sender_type, sender);
    // wasm_functype_delete(sender_type);

    wasm_functype_t* balance_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* balance_opcode = wasm_func_new(wasm_store, balance_type, balance);
    // wasm_functype_delete(balance_type);

    wasm_functype_t* transfer_type = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* transfer_opcode = wasm_func_new(wasm_store, transfer_type, transfer);
    // wasm_functype_delete(transfer_type);

    wasm_functype_t* blocktime_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
    wasm_func_t* blocktime_opcode = wasm_func_new(wasm_store, blocktime_type, blocktime);
    // wasm_functype_delete(blocktime_type);
    
    // wasm_externs = new wasm_extern_t*[];
    wasm_externs[0] = wasm_func_as_extern(sender_opcode);
    wasm_externs[1] = wasm_func_as_extern(balance_opcode);
    wasm_externs[2] = wasm_func_as_extern(transfer_opcode);
    wasm_externs[3] = wasm_func_as_extern(blocktime_opcode);

    chain_opcodes = WASM_ARRAY_VEC(wasm_externs);
    printf("Opcode size: %d, %d\n", chain_opcodes.size, sizeof(wasm_externs));
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

    if (!wasm_instance)
    {
        return false;
    }

    wasm_exporttype_vec_t export_types;
    wasm_module_exports(wasm_module, &export_types);

    int func_index = index_of_function(wasm_module, export_types, function_name, func_name_length);
    // printf("Func index %d \n", func_index);

    if (func_index < 0 )
    {
        return false;
    }
        
    const wasm_externtype_t* extern_type = wasm_exporttype_type(export_types.data[func_index]);
    if (wasm_externtype_kind(extern_type)== WASM_EXTERN_FUNC)
    {
        const wasm_functype_t* func_type = wasm_externtype_as_functype_const(extern_type);
        const wasm_valtype_vec_t* params = wasm_functype_params(func_type);
        const wasm_valtype_vec_t* results = wasm_functype_results(func_type);

        wasm_val_t *args_val = new wasm_val_t[params->size];
        wasm_val_t *results_val = (wasm_val_t*)malloc(sizeof(wasm_val_t) *results->size);

        int param_ptr = 16;

        for(int i = 0; i < params->size; i++)
        {
            wasm_valtype_t *val_type = params->data[i];
            wasm_valkind_t valkind = wasm_valtype_kind(val_type); 
            
            
            char *data_ptr = data + param_ptr; 
            // printf(" Data value of the function %x %p \n", *((uint32_t*)data_ptr), data_ptr);
            args_val[i] = WASM_I32_VAL(*((int*)data_ptr));
            param_ptr += 4;
        }

        for(int i = 0; i < results->size; i++)
        {
            wasm_valtype_t *val_type = results->data[i];
            wasm_valkind_t valkind = wasm_valtype_kind(val_type); 
            results_val[i] = WASM_INIT_VAL;
            // printf(" Data type of the function %d \n", valkind);         
        }
        // printf("Found a function: num params: %ld, result: %ld\n", params->size, results->size);

        wasm_val_vec_t args_func;
        wasm_val_vec_new(&args_func, params->size, args_val);
        wasm_val_vec_t results_func;
        wasm_val_vec_new(&results_func, results->size, results_val);

        wasm_extern_vec_t exports;
        wasm_instance_exports(wasm_instance, &exports);

        // printf("Total %ld exports function \n", exports.size);

        wasm_func_t* func_call = wasm_extern_as_func(exports.data[func_index]);
        if (wasm_func_call(func_call, &args_func, &results_func)) {
            printf("Error calling the `time` function!\n");
            return 1;
        }else {
            if (results_func.size > 0)
                printf("Func call result %d \n", results_func.data[0].of.i32);
        }
    }
    printf("Finished executing smart contract\n");
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
        // wasm_byte_vec_delete(&wasm_bytes);
        printf("Runtime::create_wasm_module size: %ld value %ld\n", code_size, wasm_module);
    }    
}

void Runtime::create_wasm_instance()
{
    if (wasm_module)
    {
        printf("Opcode size in create instance: %d \n", chain_opcodes.size);
        wasm_instance = wasm_instance_new(wasm_store, wasm_module, &chain_opcodes, NULL);
    }
}

wasm_trap_t* sender(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{    
    Host* host = Host::instance();
    uint32_t _sender = host->runtime()->context.sender;
    printf("Smart contract query sender %x \n", _sender);
    wasm_val_t val = WASM_I32_VAL(_sender);
    wasm_val_copy(&results->data[0], &val);
    return nullptr;
}

wasm_trap_t* balance(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    Host* host = Host::instance();
    uint32_t _balance = host->balance_of(args->data[0].of.i32);
    printf("Smart contract query balance of %x is %d \n", args->data[0].of.i32, _balance);
    wasm_val_t val = WASM_I32_VAL(_balance);
    wasm_val_copy(&results->data[0], &val);
    return nullptr;
}

wasm_trap_t* transfer(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    Host* host = Host::instance();
    uint32_t _sender = host->runtime()->context.sender;
    uint32_t to = args->data[0].of.i32;
    uint32_t amount = args->data[1].of.i32;
    uint32_t sender_balance = host->balance_of(_sender);
    printf("tranfer opcode params sender %x has %d transfer to %x amount %d\n", _sender, sender_balance, to, amount);
    if (sender_balance >= amount) 
    {
        host->add_balance(to, amount);
        host->sub_balance(_sender, amount);
        wasm_val_t val = WASM_I32_VAL(amount);
        wasm_val_copy(&results->data[0], &val);
    } else {
        wasm_val_t val = WASM_I32_VAL(0);
        wasm_val_copy(&results->data[0], &val);
    }
    
    return nullptr;
}

wasm_trap_t* blocktime(const wasm_val_vec_t* args, wasm_val_vec_t* results)
{
    printf("Blocktime function is called\n");
    time_t current = time(0);
    wasm_val_t val = WASM_I32_VAL(current);
    wasm_val_copy(&results->data[0], &val);
    return nullptr;
}

int index_of_function(wasm_module_t* module, wasm_exporttype_vec_t &export_types, const char* function_name, int func_name_length)
{
    for (int i = 0; i < export_types.size; ++i)
    {
        if (check_wasm_func_by_name(export_types.data[i], function_name, func_name_length))
        {
            return i;
        }
    }
    return -1;
}

bool check_wasm_func_by_name(const wasm_exporttype_t *export_type, const char *func_name, int func_name_len)
{
    const wasm_name_t *name = wasm_exporttype_name(export_type);
    // printf(" Export func name %s %ld: func name: %s \n", name->data, name->size, func_name);
    return memcmp(name->data, func_name, name->size) == 0 && func_name_len == name->size;    
}