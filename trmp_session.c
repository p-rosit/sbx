#ifndef TRMP_SESSION_H
#define TRMP_SESSION_H

// TRM_MAKE_STATE
// TRM_FREE_STATE
// TRM_STATE

#include <stdio.h>
#include <string.h>

#include "trmp_data.c"
#include "trmp_arguments.c"
#include "trmp_functions.c"
#include "trmp_history.c"

trmp_session_t trm_make_session();
void           trm_free_session(trmp_session_t*);

trmp_code_t trm_register_command(trmp_session_t*, trmp_command_t, trmp_function_name_t);
trmp_code_t trm_unregister_command(trmp_session_t*, trmp_function_name_t);
trmp_code_t trmp_call_command_with_args(trmp_session_t*, trmp_function_name_t, trmp_argument_string_t);
trmp_function_t* trmp_get_current_function(trmp_session_t*);

trmp_session_t trm_make_session(char* prefix, int history_length) {
    trmp_session_t session;
    trmp_internal_state_t internal_state;
    trmp_function_array_t functions;
    trmp_history_t history;

    internal_state.code = TRMP_OK;
    internal_state.msg[0] = '\0';

    internal_state.curr_func = -1;
    internal_state.get_signature = 0;
    functions.nfuncs = 0;
    functions.funcs = NULL;

    history.size = history_length;
    history.first_index = -1;
    history.last_index = -1;
    history.items = calloc(history_length, sizeof(trmp_history_item_t));

    internal_state.functions = functions;
    internal_state.history = history;
    session.internal_state = internal_state;

    session.state.prefix = strdup(prefix);

#ifdef TRM_MAKE_STATE
    TRM_MAKE_STATE(&session.state);
#endif

    return session;
}

void trm_free_session(trmp_session_t* session) {
    trmp_free_history(&session->internal_state.history);
    trmp_free_function_array(session->internal_state.functions);
    free(session->state.prefix);

#ifdef TRM_FREE_STATE
    TRM_FREE_STATE(session->state.public_state);
#endif
}

trmp_code_t trm_register_command(trmp_session_t* session, trmp_command_t command, trmp_function_name_t name) {
    int i, n_state_args;
    char existing_signature[TRMP_MAX_ARGS * (TRMP_MAX_TYPE_LEN + 2)];
    trmp_code_t code;
    trmp_function_array_t functions;
    trmp_function_t function, *existing;
    trmp_signature_t signature;
    trmp_arguments_t args;

    functions = session->internal_state.functions;
    existing = trmp_find_function(&session->internal_state.functions, name);
    if (existing != NULL) {
        signature = existing->signature;
        trmp_unparse_types(existing_signature, signature.ntypes, signature.types);
        sprintf(session->internal_state.msg, "command with name %s already exists with signature (%s).", name, existing_signature);
        session->internal_state.code = TRMP_COMMAND_EXISTS_ERROR;
        return TRMP_COMMAND_EXISTS_ERROR;
    }

    session->internal_state.msg[0] = '\0';
    session->internal_state.get_signature = 1;
    session->internal_state.code = TRMP_OK;
    args = (trmp_arguments_t) {.nargs=0};

    code = (*command)(session, &signature, args);

    if (code != TRMP_OK) {
        return code;
    }

    n_state_args = 0;
    for (i = 0; i < signature.ntypes; i++) {
        n_state_args += signature.types[i] == TRMP_STATE_ARG;
    }
    if (n_state_args > 1) {
        sprintf(session->internal_state.msg, "trm_state_t can only be included as an argument at most one time");
        session->internal_state.code = TRMP_ARG_ERROR;
        return TRMP_ARG_ERROR;
    }

    function.name = strdup(name);
    function.function = command;
    function.signature = signature;

    trmp_append_function(&session->internal_state.functions, function);

    return TRMP_OK;
}

trmp_code_t trm_unregister_command(trmp_session_t* session, trmp_function_name_t name) {
    int exists;

    exists = trmp_remove_function(&session->internal_state.functions, name);

    if (!exists) {
        sprintf(session->internal_state.msg, "No command exists");
        return TRMP_NO_COMMAND_ERROR;
    }

    return TRMP_OK;
}

trmp_code_t trmp_call_command(trmp_session_t* session, char* command_str) {
    int len;
    char *name, *argument_str;
    trmp_code_t code;

    command_str = strdup(command_str);
    argument_str = trmp_strip_start(command_str);

    name = trmp_get_next_token(&argument_str);

    code = trmp_call_command_with_args(session, name, argument_str);

    free(command_str);
    return code;
}

trmp_code_t trmp_call_command_with_args(trmp_session_t* session, trmp_function_name_t name, trmp_argument_string_t arg_str) {
    int i, index, exists;
    trmp_code_t code;
    trmp_function_array_t functions;
    trmp_function_t function;
    trmp_arguments_t args;

    session->internal_state.msg[0] = '\0';
    functions = session->internal_state.functions;
    for (i = 0, exists = 0; i < functions.nfuncs; i++) {
        if (strcmp(name, functions.funcs[i].name) == 0) {
            exists = 1;
            index = i;
            function = session->internal_state.functions.funcs[i];
            break;
        }
    }

    if (!exists) {
        sprintf(session->internal_state.msg, "command with name %s not found", name);
        session->internal_state.code = TRMP_NO_COMMAND_ERROR;
        return TRMP_NO_COMMAND_ERROR;
    }
    session->internal_state.curr_func = index;

    args = trmp_make_arguments(function.signature.ntypes);
    for (i = 0; i < args.nargs; i++) args.args[i].type = function.signature.types[i];

    code = trmp_parse_arguments(session, function, &args, arg_str);
    if (code != TRMP_OK) {
        trmp_free_arguments(args);
        return code;
    }

    session->internal_state.get_signature = 0;
    code = (*(function.function))(session, NULL, args);

    trmp_free_arguments(args);
    return TRMP_OK;
}

trmp_function_t* trmp_get_current_function(trmp_session_t* session) {
    if (!(0 <= session->internal_state.curr_func && session->internal_state.curr_func < session->internal_state.functions.nfuncs))
        return NULL;
    return &session->internal_state.functions.funcs[session->internal_state.curr_func];
}

int trmp_compare_input_to_signature(trmp_arguments_t args, const int ntypes, const trmp_type_t* types) {
    int identical = 1;
    if (args.nargs != ntypes) return 1;

    for (int i = 0; i < args.nargs; i++) {
        identical = identical && (args.args[i].type == types[i]);
    }
    return !identical;
}

trmp_code_t trmp_signature_error(trmp_session_t* session, trmp_arguments_t args, const int ntypes, const trmp_type_t* types) {
    char in_sig[TRMP_MAX_ARGS * (TRMP_MAX_TYPE_LEN + 2)];
    char real_sig[TRMP_MAX_ARGS * (TRMP_MAX_TYPE_LEN + 2)];
    trmp_type_t in_types[TRMP_MAX_ARGS];
    trmp_function_t* function = trmp_get_current_function(session);

    if (function == NULL) {return TRMP_UNKNOWN_ERROR;}

    for (int i = 0; i < TRMP_MAX_ARGS; i++) in_types[i] = args.args[i].type;
    trmp_unparse_types(in_sig, args.nargs, in_types);
    trmp_unparse_types(real_sig, ntypes, (trmp_type_t*) types);

    sprintf(session->internal_state.msg, "%s has signature (%s) but recieved inputs of type (%s).", function->name, real_sig, in_sig);

    if (args.nargs != ntypes) {
        session->internal_state.code = TRMP_ARG_ERROR;
        return TRMP_ARG_ERROR;
    } else {
        session->internal_state.code = TRMP_TYPE_ERROR;
        return TRMP_TYPE_ERROR;
    }
}

#endif

