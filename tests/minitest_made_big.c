/// TESTFILE: minitest_made_big.c
///
/// Testing for miniarena.h:
/// Allocating more bytes than a single chunk can handle by default
///
/// COMMAND: cc -Wall -Wextra -std=c11 -g -DMA_IMPLEMENTATION -o minitest_made_big minitest_made_big.c
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

    mini_arena arena = ma_new_with_size(1, 2*1024);
    const size_t array_len =  4*1024;
    const size_t array_size = sizeof(int32_t) * array_len;

    int* array = NULL;

    { // testing alloc too big
        fprintf(stderr, "TESTING: ma_alloc(a: %p, nbytes: %zu)\n", &arena, array_size);

        array = ma_alloc(&arena, array_size);
        assert(array != NULL);

        for(size_t i = 0; i < (array_len); ++i) {
           array[i] = i + 1; 
        }
    }

    fprintf(stderr, "\n");

    { // printing Arena info

        fprintf(stderr, "ARENA INFO\n");
        fprintf(stderr, "|  \\_ count: %zu\n", arena.count);

        for(size_t i = 0; i < arena.count; ++i) {
            fprintf(stderr, "|  \\_ CHUNK[%zu]:\n", i);
            fprintf(stderr, "|  |   \\_ size: %zu\n", arena.chunks[i]->size);
            fprintf(stderr, "|  |   \\_ len:  %zu\n", arena.chunks[i]->len);
            fprintf(stderr, "|  |   \\_ data: %p\n",  arena.chunks[i]->data);
        }
    }

    ma_free(&arena);

    return 0;
}
