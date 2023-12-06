#ifndef TRMP_FUNCTIONS_H
#define TRMP_FUNCTIONS_H

#include <stdlib.h>
#include <string.h>
#include "trmp_data.c"
#include "trmp_arguments.c"

void trmp_free_function_array(trmp_function_array_t);
void trmp_free_function(trmp_function_t);

trmp_function_t*    trmp_find_function(trmp_function_array_t*, trmp_function_name_t);
void                trmp_append_function(trmp_function_array_t*, trmp_function_t);
int                 trmp_remove_function(trmp_function_array_t*, trmp_function_name_t);

void trmp_free_function_array(trmp_function_array_t functions) {
    for (int i = 0; i < functions.nfuncs; i++) {
        trmp_free_function(functions.funcs[i]);
    }
    free(functions.funcs);
}

void trmp_free_function(trmp_function_t function) {
    free(function.name);
}

void trmp_append_function(trmp_function_array_t* functions, trmp_function_t function) {
    trmp_function_array_t new_funcs;

    new_funcs.nfuncs = functions->nfuncs + 1;
    new_funcs.funcs = malloc(new_funcs.nfuncs * sizeof(trmp_function_t));
    memcpy(new_funcs.funcs, functions->funcs, functions->nfuncs * sizeof(trmp_function_t));

    free(functions->funcs);
    new_funcs.funcs[functions->nfuncs] = function;

    functions->nfuncs = new_funcs.nfuncs;
    functions->funcs = new_funcs.funcs;
}

int trmp_remove_function(trmp_function_array_t* functions, trmp_function_name_t name) {
    int i, index, exists;

    for (i = 0, exists = 0; i < functions->nfuncs; i++) {
        if (strcmp(name, functions->funcs[i].name) == 0) {
            exists = 1;
            index = i;
            break;
        }
    }

    if (exists) {
        trmp_free_function(functions->funcs[index]);
        for (i = index + 1; i < functions->nfuncs; i++) {
            functions->funcs[i - 1] = functions->funcs[i];
        }
        functions->nfuncs -= 1;
    }
    
    return exists;
}

trmp_function_t* trmp_find_function(trmp_function_array_t* functions, trmp_function_name_t name) {
    int i, exists;

    for (i = 0, exists = 0; i < functions->nfuncs; i++) {
        if (strcmp(name, functions->funcs[i].name) == 0) {
            exists = 1;
            return &functions->funcs[i];
        }
    }

    return NULL;
}

#endif

