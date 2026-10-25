/// TESTFILE: minitest.c
///
/// Basic testing for miniarena.h
///
/// COMMAND: cc -Wall -Wextra -std=c11 -g -DMA_IMPLEMENTATION -o minitest minitest.c
///
#include "../miniarena.h"

#include <stdio.h>
#include <signal.h>

void signal_handler(int signal) {
    if(signal == SIGSEGV) {
        fprintf(stderr, "\n ----------------- Segmantation Fault ----------------- \n");
    } else if(signal == SIGABRT) {
        fprintf(stderr, "\n ----------------- ABORT ----------------- \n");
    }

    exit(1);
};

int main(void) {
    signal(SIGSEGV, signal_handler);
    signal(SIGABRT, signal_handler);

    mini_arena arena = ma_new( );
    const size_t array_len = 10;
    const size_t array_size = sizeof(int32_t) * array_len;

    int* array = NULL;
    uintptr_t start_address = 0x0;

    { // testing alloc
        fprintf(stderr, "TESTING: ma_alloc(a: %p, nbytes: %zu)\n", &arena, array_size);

        array = ma_alloc(&arena, array_size);
        assert(array != NULL);
        start_address = (uintptr_t)&array[0];

        for(size_t i = 0; i < (array_len); ++i) {
           array[i] = i + 1; 
        }
    
        for(size_t i = 0; i < (array_len); ++i) {
            fprintf(stderr, "(0x%lx) [%02zu]: %d\n", (uintptr_t)&array[i], i, array[i]);
        }
    }

    fprintf(stderr, "\n\n");

    { // testing realloc
        fprintf(stderr, "TESTING: ma_realloc(a: %p, array: %p, old_size: %zu, new_size: %zu)\n", 
                                            &arena, array, 
                                            array_size, (array_size * 2));

        array = ma_realloc(&arena, array, array_size, (array_size * 2));
        assert(array != NULL);
        if((uintptr_t)&array[0] == start_address) {
            fprintf(stderr, "ma_realloc failed as it ended up returning the same pointer!\n");
            abort();
        }

        for(size_t i = 0; i < array_len; ++i) {
           array[array_len + i] = (array_size/sizeof(array[0])) + i + 1; 
        }

        for(size_t i = 0; i < array_len*2; ++i) {
            fprintf(stderr, "(0x%lx) [%02zu]: %d\n", (uintptr_t)&array[i], i, array[i]);
        }
    }

    MA_ASSERT(arena.chunks[0]->len >= (array_size * 3));

    fprintf(stderr, "\n\n");

    { // printing Arena info

        fprintf(stderr, "ARENA INFO\n");

        for(size_t i = 0; i < arena.count; ++i) {
            fprintf(stderr, "|- CHUNK[%zu]:\n", i);
            fprintf(stderr, "|    \\_ size: %zu\n", arena.chunks[i]->size);
            fprintf(stderr, "|    \\_ len:  %zu\n", arena.chunks[i]->len);
            fprintf(stderr, "|    \\_ data: %p\n",  arena.chunks[i]->data);
        }
    }

    ma_free(&arena);

    return 0;
}
