/*
    miniarena - v0.1.0 - MIT

    MINIARENA LIBRARY (STB-STYLE)

    Simple arena allocator in C

    # Macro Interface

        All these macros are `#define`d by the user before including miniarena.h

    ## Flags

        Enable or disable certain aspects of miniarena.h

        - MA_IMPLEMENTATION - Enable definitions of the functions. By default only declarations are included.

    ## Redefinable Macros

            Redefine default behaviors of miniarena.h

        - #define MA_ASSERT         (default to libc)
        - #define MA_MALLOC         (default to libc)
        - #define MA_REALLOC        (default to libc)
        - #define MA_FREE           (default to libc)

   --------------------------------------------------------------------------------
   LICENSE: MIT
    
   Copyright (c) 2026 @Nickys0 <https://github.com/Nickys0>
  
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
  
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
  
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef _MINI_ARENA_H
#define _MINI_ARENA_H

#include <stdint.h>
#include <stdalign.h>
#include <stdlib.h>

#if !defined(MA_ASSERT)
#include <assert.h>
#define MA_ASSERT(expr) assert((expr))
#endif

#if !defined(MA_MALLOC) && !defined(MA_FREE) && !defined(MA_REALLOC)
    #define MA_MALLOC(bytes)        malloc((bytes))
    #define MA_FREE(ptr)            free((ptr))
    #define MA_REALLOC(old, nbytes) realloc((old), (nbytes))
#endif

#define MA_DEFAULT_CHUNK_COUNT 4
#define MA_DEFAULT_CHUNK_SIZE  (4*1024*1024) // 4MiB

// @NOTE: According to the ISO C standard (C11 §7.19/2):
//   max_align_t is an object type whose alignment is the greatest fundamental alignment.
// This is the same alignment that malloc uses.
#define MA_ALIGNMENT           alignof(max_align_t)

// Once allocated a chunk must not be realloc'd only free'd
typedef struct arena_chunk {
    size_t    size;
    size_t    len;
    uintptr_t data[];
} ma_chunk;

typedef struct mini_arena {
    ma_chunk** chunks;
    size_t     count;
} mini_arena;

// Creating the Arena
mini_arena ma_new(/* 4 chunks of 4MiB each */);
mini_arena ma_new_with_size(size_t count, size_t chunk_size);

// Expanding the arena (NULL to indicate an Error)
mini_arena* ma_expand(mini_arena*); // Creates a new chunk with size MA_DEFAULT_CHUNK_SIZE
mini_arena* ma_expand_with_size(mini_arena*, size_t nbytes); // Creates a new chunk with size <nbytes> 
mini_arena* ma_expand_n_with_size(mini_arena*, size_t count, size_t nbytes); // Creates a n chunks with size <nbytes> 

// Allocating in the arena (NULL to indicate an Error)
void* ma_alloc(mini_arena*, size_t nbytes); // aligned based on MA_ALIGNMENT
void* ma_realloc(mini_arena*, void* old, size_t old_size, size_t new_nbytes); // aligned based on MA_ALIGNMENT

// Freeing\Clearing the arena
void ma_free(mini_arena*);  // Frees the entire arena
void ma_clear(mini_arena*); // Sets all Arena chunks.len to 0

//////////////////////////////////////////////////////
// Basic example
#if 0
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
#endif

#endif

/**
 * @brief MINIARENA Implementation section
 */
#if defined(MA_IMPLEMENTATION)  && !defined(__MA_IMPLEMENTED)
#define __MA_IMPLEMENTED

#include <stddef.h>
#include <string.h>

static_assert((MA_ALIGNMENT & (MA_ALIGNMENT - 1)) == 0, "MA_ALIGNMENT must be a power of 2");

static ma_chunk* ma__new_chunk(size_t chunk_size) {
    ma_chunk* chunk = MA_MALLOC(sizeof(ma_chunk) + chunk_size);
    MA_ASSERT(chunk != NULL);

    chunk->size = chunk_size;
    chunk->len =  0;

    return chunk;
}

mini_arena ma_new( ) {
    return ma_new_with_size(MA_DEFAULT_CHUNK_COUNT, MA_DEFAULT_CHUNK_SIZE);
}

mini_arena ma_new_with_size(size_t count, size_t chunk_size) {
    mini_arena ma = {0};

    if(count == 0) {
        return ma;
    }

    ma.count = count;
    ma.chunks = (ma_chunk**)MA_MALLOC(sizeof(uintptr_t)*count);
    MA_ASSERT(ma.chunks != NULL);

    for(size_t i = 0; i < count; ++i) {
        ma.chunks[i] = ma__new_chunk(chunk_size);
    }

    return ma;
}

mini_arena* ma_expand(mini_arena* a) {
    return ma_expand_n_with_size(a, 1, MA_DEFAULT_CHUNK_SIZE);
}

mini_arena* ma_expand_with_size(mini_arena* a, size_t nbytes) {
    return ma_expand_n_with_size(a, 1, nbytes);
}

mini_arena* ma_expand_n_with_size(mini_arena* a, size_t count, size_t nbytes) {
    if(a == NULL || count == 0 || nbytes == 0) {
        return NULL;
    }

    a->chunks = (ma_chunk**)MA_REALLOC(a->chunks, sizeof(uintptr_t)*(count + a->count));
    MA_ASSERT(a->chunks != NULL);

    for(size_t i = a->count; i < (a->count + count); ++i) {
        a->chunks[i] = ma__new_chunk(nbytes);
    }

    a->count += count;

    return a;
}

void* ma_alloc(mini_arena* a, size_t nbytes) {
    if(a == NULL || nbytes == 0) {
        return NULL;
    }

    int retry = 0;
    uintptr_t address = 0x0;

retry:
    for(size_t i = 0; i < a->count; ++i) {
        ma_chunk* chunk = a->chunks[i];

        // A bit of weird pointer logic
        // How it works:
        // alignment - 1: mask of lower bits. For an 8-byte alignment: (0x7: 0b0111).
        // + (alignment - 1): Pushes any unaligned value across the boundary into the next multiple 
        // & ~(alignment - 1): Clears the lower bits, snapping down to the start of the aligned block.
        // becouse of how it works MA_ALIGNMENT must be a power of 2
        uintptr_t aligned = (((uintptr_t)&chunk->data[chunk->len]) + (MA_ALIGNMENT - 1)) & 
                                    ~(uintptr_t)(MA_ALIGNMENT - 1);

        uintptr_t last_byte = (uintptr_t)&chunk->data[chunk->size];

        if((last_byte - aligned) > nbytes) {
            address = (uintptr_t)&chunk->data[chunk->len];
            chunk->len += ((address - (uintptr_t)&chunk->data[chunk->len]) + nbytes);
            break;
        }
    }

    if(address == 0x0) {
        // To have a little bit of space we try to allocate double the required sized 
        // if it doesn't exceed 64MiB of memory otherwise we add a little padding of 1KiB
        const size_t cs = nbytes * 2 <= (64*1024*1024)? nbytes * 2: nbytes + (1*1024);
        void* d = ma_expand_with_size(a, cs);
        
        // We can't directly use the function inside the assert becouse if it was "disable" 
        // the function wouldn't be called anymore
        MA_ASSERT(d != NULL);

        if(retry == 0) {
            goto retry;
        } /* else return NULL */
    }

    return (void*)address;
}

void* ma_realloc(mini_arena* a, void* old, size_t old_size, size_t new_nbytes){
    if(a == NULL || old == NULL || new_nbytes <= old_size) {
        return old;
    }

    void* mem = ma_alloc(a, new_nbytes); // ??
    if(mem == NULL) {
        return NULL;
    }

    memcpy(mem, old, old_size);

    return mem;
}

void ma_free(mini_arena* a) {
    if(a == NULL) return;

    for(size_t i = 0; i < a->count; ++i) {
        MA_FREE(a->chunks[i]);
    }

    MA_FREE(a->chunks);
    
    memset(a, 0, sizeof(*a));
}

void ma_clear(mini_arena* a) {
    if(a == NULL) return;

    for(size_t i = 0; i < a->count; ++i) {
        a->chunks[i]->len = 0;
    }
}

#endif

/*
    Revision history:

    0.1.0 [2026-10-26] First minor release

    We are following https://semver.org/ so the version has a format MAJOR.MINOR.PATCH:
      - Modifying comments does not update the version.
      - PATCH is incremented in case of a bug fix or refactoring without touching the API.
      - MINOR is incremented when new functions and/or types are added in a way that does
        not break any existing user code. We want to do this in the majority of the situation.
        If we want to delete a certain function or type in favor of another one we should
        just add the new function/type and deprecate the old one in a backward compatible way
        and let them co-exist for a while.
      - MAJOR update should be just a periodic cleanup of the DEPRECATED functions and types
        without really modifying any existing functionality.
      - Breaking backward compatibility in a MINOR release should be considered a bug and
        should be promptly fixed in the next PATCH release.
*/