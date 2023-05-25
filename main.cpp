
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
    uint32_t balance = host->balance_of(0xaa);
    std::cout << "Balance of 0xaa " << balance << std::endl;

    host->add_balance(0xaa, 12);
    host->add_balance(0xaa, 15);

    // Transfer native token
    host->handle_execute(0xaa, 0xab, 27, 0, 0, nullptr);
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

    // Execute function
    char calldata_test_time[16];
    put_func_name(calldata_test_time, "test_time");
    host->handle_execute(0xaa, contract_address, 0, 2, 16, calldata_test_time);

    wasm_byte_vec_t wasm_bytes;
    wasm_byte_vec_new(&wasm_bytes, size, wasm_content);

    wasm_engine_t* engine = wasm_engine_new();
    wasm_store_t* store = wasm_store_new(engine);

    wasm_module_t* module = wasm_module_new(store, &wasm_bytes);
    // wasm_module_t* module1 = wasm_module_new(store, &wasm_bytes);
    free(wasm_content);
    wasm_byte_vec_delete(&wasm_bytes);
    if (!module) {
        printf("> Error compiling module!\n");
        return 1;
    }

    wasm_functype_t* max_type = wasm_functype_new_2_1(wasm_valtype_new_i32(), wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasm_func_t* max_func = wasm_func_new(store, max_type, env_max);
    wasm_functype_t* blocktime_type = wasm_functype_new_0_1(wasm_valtype_new_i32());
    wasm_func_t* time_func = wasm_func_new(store, blocktime_type, blocktime_main);
    wasm_extern_vec_t import_object;
    {
        wasm_extern_t* externs[] = {
            wasm_func_as_extern(max_func), wasm_func_as_extern(time_func)
        };
        import_object = WASM_ARRAY_VEC(externs);
    }
    
    // wasm_extern_vec_t *import_objects = WASM_EMPTY_VEC;    

    // Create the instance
    wasm_instance_t* instance = wasm_instance_new(store, module, &import_object, NULL);
    // wasm_instance_t* instance1 = wasm_instance_new(store, module, &import_object, NULL);

    if (!instance) {
      printf("Error instantiating module!\n");
      return 1;
    }

    wasm_exporttype_vec_t export_types;
    wasm_module_exports(module, &export_types);



    // // Get the context data pointer
    wasm_extern_vec_t exports;
    wasm_instance_exports(instance, &exports);

    if (exports.size == 0) {
        printf("No export functions!\n");
        return 1;
    }
    printf("Size of exports %ld\n", exports.size);

    wasm_func_t* sum_func = wasm_extern_as_func(exports.data[2]);
    wasm_func_t* blocktime_func = wasm_extern_as_func(exports.data[3]);

    wasm_functype_t *func_type = wasm_func_type(sum_func);
    wasm_externtype_t *extern_type = wasm_functype_as_externtype(func_type);
    wasm_externkind_t kind = wasm_externtype_kind(extern_type);

    const char *func_name = "sum";
    int index = get_index_of_function(module, func_name);
    printf("\nindex of function %s is %d\n", func_name, index);

    const wasm_name_t *name = wasm_exporttype_name(export_types.data[2]);
    char *name_info = (char*)malloc(name->size + 1);
    strncpy(name_info, name->data, name->size);
    name_info[name->size] = 0;
    printf("Function name %s \n", name_info);

    printf("Kind == %d\n", kind);
    // wasm_exporttype_name(wasm_ex)
    // wasm_ref_t * ref = wasm_func_as_ref(sum_func);
    // printf("%d", *ref);
    // exports.data[2];
    if (sum_func == NULL) {
        printf("> Failed to get the `sum` function!\n");
        return 1;
    }

    printf("Calling `sum` function...\n");
    wasm_val_t args_val[0] = {};
    wasm_val_t results_val[1] = { WASM_INIT_VAL };
    wasm_val_vec_t args = WASM_ARRAY_VEC(args_val);
    
    wasm_val_vec_t results = WASM_ARRAY_VEC(results_val);

    if (wasm_func_call(blocktime_func, &args, &results)) {
        printf("Error calling the `time` function!\n");
        return 1;
    }
    printf("Results of `sum`: %d\n", results_val[0].of.i32);
    return 0;
}
