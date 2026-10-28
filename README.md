# miniarena.h - A simple Arena allocator in C

## Basic example
```c
#define MA_IMPLEMENTATION
#include "miniarena.h"
#include <stdio.h>

int main(void) {
    mini_arena arena = ma_new();

    const size_t array_len = 10;
    int* array = ma_alloc(&arena, sizeof(int)*array_len);
    for(size_t i = 0; i < array_len; ++i) {
        array[i] = i * i; 
    }

    for(size_t i = 0; i < array_len; ++i) {
        fprintf(stderr, "(0x%lx) [%02zu]: %d\n", (uintptr_t)&array[i], i, array[i]);
    }

    ma_free(&arena);
    return 0;
}
```

## How to use the library in your own project

The only file you need from here is [miniarena.h](https://github.com/Nickys0/miniarena.h/blob/main/miniarena.h). Just copy-paste it to your project and start using it

Aftery copy-paste just include "miniarena.h" in your project and define MA_IMPLEMENTATION to include the implementation.

## Configuration

The library lets you define some MACROS before including the library to change its behaviour:

You can define this to use different allocators/assertions which are compatible with the libc related functions

```c
// Redefine default behaviors of miniarena.h
#define MA_ASSERT(expr) // (default to libc's assert macro)
#define MA_MALLOC(nbytes) // (default to libc's malloc function)
#define MA_REALLOC(old_ptr, new_nbytes) // (default to libc's realloc function)
#define MA_FREE(ptr) // (default to libc's free function)
```

## How to run tests

To run tests under ./tests just compile run_tests like this:

```bash
gcc -o tester run_tests.c -Wall -Wextra
```

and then run it:

```bash
./tester
```

(If you are on Windows you may need to use a different compiler)

## Compatibility

The library is tested on Linux and it should work also on Windows but it is not tested yet.