#include <emscripten.h>
#include <stdint.h>
#include <stdlib.h>

#include "sdk/account.h"

extern int env_max(int a, int b);

EMSCRIPTEN_KEEPALIVE
int sum(int a, int b) 
{
  return a + b + env_max(a, b);
}

EMSCRIPTEN_KEEPALIVE
uint32_t test_time() 
{
  return time();
}

// EMSCRIPTEN_KEEPALIVE
// uint32_t test_sender() 
// {
//   return sender();
// }

