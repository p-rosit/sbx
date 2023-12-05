#ifndef TRMP_FUNCTIONS_H
#define TRMP_FUNCTIONS_H

#include <stdlib.h>
#include <string.h>
#include "trmp_data.c"
#include "trmp_arguments.c"

void trmp_free_function_array(trmp_function_array_t);
void trmp_free_function(trmp_function_t);
void trmp_append_function(trmp_function_array_t*, trmp_function_t);

void trmp_free_function_array(trmp_function_array_t functions) {
    for (int i = 0; i < functions.nfuncs; i++) {
        trmp_free_function(functions.funcs[i]);
    }
    free(functions.funcs);
}

void trmp_free_function(trmp_function_t function) {
    free(function.name);
}

void trmp_append_function(trmp_function_array_t *functions, trmp_function_t function) {
    trmp_function_array_t new_funcs;

    new_funcs.nfuncs = functions->nfuncs + 1;
    new_funcs.funcs = malloc(new_funcs.nfuncs * sizeof(trmp_function_t));
    memcpy(new_funcs.funcs, functions->funcs, functions->nfuncs * sizeof(trmp_function_t));

    free(functions->funcs);
    new_funcs.funcs[functions->nfuncs] = function;

    functions->nfuncs = new_funcs.nfuncs;
    functions->funcs = new_funcs.funcs;
}

#endif

