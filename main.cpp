
extern "C" {
    #include <wasm.h>
}
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unordered_map>
#include <ctime>
#include <stdint.h>

#include "env/host.h"
#include "env/runtime.h"

wasm_trap_t* env_max(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
    wasm_val_t val = WASM_I32_VAL(args->data[0].of.i32 > args->data[1].of.i32 ? args->data[0].of.i32 : args->data[1].of.i32);
    wasm_val_copy(&results->data[0], &val);
    return NULL;
}

wasm_trap_t* blocktime_main(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
    time_t current = time(0);
    wasm_val_t val = WASM_I32_VAL(current);
    wasm_val_copy(&results->data[0], &val);
    return NULL;
}

bool test_wasm_func_by_name(const wasm_exporttype_t *export_type, const char *func_name)
{
    const wasm_name_t *name = wasm_exporttype_name(export_type);
    return memcmp(name->data, func_name, name->size) == 0;
}

int get_index_of_function(wasm_module_t* module, const char* function_name)
{
    wasm_exporttype_vec_t export_types;
    wasm_module_exports(module, &export_types);
    int num_export = export_types.size;
    for (int i = 0; i < num_export; ++i)
    {
        if (test_wasm_func_by_name(export_types.data[i], function_name))
        {
            return i;
        }
    }
    return -1;
}

void put_func_name(char *buffer, const char* func_name)
{
    int func_name_len = strlen(func_name);
    int start_index = 16 - func_name_len;
    memset(buffer, ' ', start_index);
    memcpy(buffer + start_index, func_name, func_name_len);
}

int main() {
    Host *host = Host::instance();
    uint32_t balance_ = host->balance_of(0xaa);
    std::cout << "Balance of 0xaa " << balance_ << std::endl;

    host->add_balance(0xaa, 12);
    host->add_balance(0xaa, 15);

    // Transfer native token
    host->handle_execute(0xaa, 0xab, 17, 0, 0, nullptr);
    std::cout << "New balance 0xaa " << host->balance_of(0xaa) << " 0xab " << host->balance_of(0xab) << std::endl;


    FILE* file = fopen("module.wasm", "rb");
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    wasm_byte_t* wasm_content = (wasm_byte_t*)malloc(size);
    size_t n = fread(wasm_content, 1, size, file);
    fclose(file);

    if (n == 0) {
        printf("Wasm file is empty");
        return 1;
    }

    // Deploy Smart Contract
    uint32_t contract_address = host->handle_execute(0xaa, 0, 0, 1, size, wasm_content);
    printf("Contract address 0x%x \n", contract_address);

    // Execute function test_time()
    char calldata_test_time[16];
    put_func_name(calldata_test_time, "test_time");
    uint32_t status = host->handle_execute(0xaa, contract_address, 0, 2, 16, calldata_test_time);
    std::cout << " --------> " << status << std::endl;


    // Execute function test_sender()
    char calldata_test_sender[16];
    put_func_name(calldata_test_sender, "test_sender");
    status = host->handle_execute(0xaabb, contract_address, 0, 3, 16, calldata_test_sender);
    std::cout << " --------> " << status << std::endl;

    // Execute function test_sender()
    char calldata_test_balance[20];
    put_func_name(calldata_test_balance, "test_balance");
    uint32_t addr1 = 0xaa;
    memcpy(calldata_test_balance + 16, &addr1, 4);
    status = host->handle_execute(0xaabb, contract_address, 0, 4, 20, calldata_test_balance);
    std::cout << " --------> " << status << std::endl;

    // Execute function test_sender()
    char calldata_test_transfer[24];
    put_func_name(calldata_test_transfer, "test_transfer");
    addr1 = 0xac;
    uint32_t amount1 = 5;
    memcpy(calldata_test_transfer + 16, &addr1, 4);
    memcpy(calldata_test_transfer + 20, &amount1, 4);
    status = host->handle_execute(0xaabb, contract_address, 0, 5, 24, calldata_test_transfer);
    std::cout << " --------> " << status << std::endl;

    status = host->handle_execute(0xaa, contract_address, 0, 5, 24, calldata_test_transfer);
    std::cout << " --------> " << status << std::endl;

    return 0;
}
