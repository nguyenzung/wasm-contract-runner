#include <stdint.h>
// #include <stdlib.h>
#include <emscripten.h>


#include "sdk/account.h"

// extern int blocktime();

// extern int time();

// extern int env_max(int a, int b);

// EMSCRIPTEN_KEEPALIVE
// int sum(int a, int b) 
// {
//   return a + b + env_max(a, b);
// }

EMSCRIPTEN_KEEPALIVE
uint32_t test_time() 
{
  return blocktime();
}

EMSCRIPTEN_KEEPALIVE
uint32_t test_balance(uint32_t address) 
{
  return balance(address);
}

EMSCRIPTEN_KEEPALIVE
uint32_t test_transfer(uint32_t address, uint32_t amount) 
{
  return transfer(address, amount);
  ;
}

EMSCRIPTEN_KEEPALIVE
uint32_t test_sender() 
{
  return sender();
}

