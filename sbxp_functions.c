#ifndef SBXP_FUNCTIONS_H
#define SBXP_FUNCTIONS_H

#include <stdlib.h>
#include <string.h>
#include "sbxp_data.c"
#include "sbxp_arguments.c"

void sbxp_free_function_array(sbxp_function_array_t);
void sbxp_free_function(sbxp_function_t);

sbxp_function_t*    sbxp_find_function(sbxp_function_array_t*, sbxp_function_name_t);
void                sbxp_append_function(sbxp_function_array_t*, sbxp_function_t);
int                 sbxp_remove_function(sbxp_function_array_t*, sbxp_function_name_t);

void sbxp_free_function_array(sbxp_function_array_t functions) {
    for (int i = 0; i < functions.nfuncs; i++) {
        sbxp_free_function(functions.funcs[i]);
    }
    free(functions.funcs);
}

void sbxp_free_function(sbxp_function_t function) {
    free(function.name);
}

void sbxp_append_function(sbxp_function_array_t* functions, sbxp_function_t function) {
    sbxp_function_array_t new_funcs;

    new_funcs.nfuncs = functions->nfuncs + 1;
    new_funcs.funcs = malloc(new_funcs.nfuncs * sizeof(sbxp_function_t));
    memcpy(new_funcs.funcs, functions->funcs, functions->nfuncs * sizeof(sbxp_function_t));

    free(functions->funcs);
    new_funcs.funcs[functions->nfuncs] = function;

    functions->nfuncs = new_funcs.nfuncs;
    functions->funcs = new_funcs.funcs;
}

int sbxp_remove_function(sbxp_function_array_t* functions, sbxp_function_name_t name) {
    int i, index, exists;

    for (i = 0, exists = 0; i < functions->nfuncs; i++) {
        if (strcmp(name, functions->funcs[i].name) == 0) {
            exists = 1;
            index = i;
            break;
        }
    }

    if (exists) {
        sbxp_free_function(functions->funcs[index]);
        for (i = index + 1; i < functions->nfuncs; i++) {
            functions->funcs[i - 1] = functions->funcs[i];
        }
        functions->nfuncs -= 1;
    }
    
    return exists;
}

sbxp_function_t* sbxp_find_function(sbxp_function_array_t* functions, sbxp_function_name_t name) {
    int i;

    for (i = 0; i < functions->nfuncs; i++) {
        if (strcmp(name, functions->funcs[i].name) == 0) {
            return &functions->funcs[i];
        }
    }

    return NULL;
}

#endif

