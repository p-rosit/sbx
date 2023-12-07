#ifndef SBXP_SESSION_H
#define SBXP_SESSION_H

// SBX_MAKE_STATE
// SBX_FREE_STATE
// SBX_STATE

#include <stdio.h>
#include <string.h>

#include "sbxp_data.c"
#include "sbxp_arguments.c"
#include "sbxp_functions.c"
#include "sbxp_history.c"

sbxp_session_t sbx_make_session();
void           sbx_free_session(sbxp_session_t*);

sbxp_code_t sbx_register_command(sbxp_session_t*, sbxp_command_t, sbxp_function_name_t);
sbxp_code_t sbx_unregister_command(sbxp_session_t*, sbxp_function_name_t);
sbxp_code_t sbxp_call_command_with_args(sbxp_session_t*, sbxp_function_name_t, sbxp_argument_string_t);
sbxp_function_t* sbxp_get_current_function(sbxp_session_t*);

sbxp_session_t sbx_make_session(char* prefix, int history_length) {
    sbxp_session_t session;
    sbxp_internal_state_t internal_state;
    sbxp_function_array_t functions;
    sbxp_history_t history;

    internal_state.code = SBXP_OK;
    internal_state.msg[0] = '\0';

    internal_state.curr_func = -1;
    internal_state.get_signature = 0;
    functions.nfuncs = 0;
    functions.funcs = NULL;

    history.size = history_length;
    history.first_index = -1;
    history.last_index = -1;
    history.items = calloc(history_length, sizeof(sbxp_history_item_t));

    internal_state.functions = functions;
    internal_state.history = history;
    session.internal_state = internal_state;

    session.state.prefix = strdup(prefix);

#ifdef SBX_MAKE_STATE
    SBX_MAKE_STATE(&session.state);
#endif

    return session;
}

void sbx_free_session(sbxp_session_t* session) {
    sbxp_free_history(&session->internal_state.history);
    sbxp_free_function_array(session->internal_state.functions);
    free(session->state.prefix);

#ifdef SBX_FREE_STATE
    SBX_FREE_STATE(session->state.public_state);
#endif
}

sbxp_code_t sbx_register_command(sbxp_session_t* session, sbxp_command_t command, sbxp_function_name_t name) {
    int i, n_state_args;
    char existing_signature[SBXP_MAX_ARGS * (SBXP_MAX_TYPE_LEN + 2)];
    sbxp_code_t code;
    sbxp_function_array_t functions;
    sbxp_function_t function, *existing;
    sbxp_signature_t signature;
    sbxp_arguments_t args;

    functions = session->internal_state.functions;
    existing = sbxp_find_function(&session->internal_state.functions, name);
    if (existing != NULL) {
        signature = existing->signature;
        sbxp_unparse_types(existing_signature, signature.ntypes, signature.types);
        sprintf(session->internal_state.msg, "command with name %s already exists with signature (%s).", name, existing_signature);
        session->internal_state.code = SBXP_COMMAND_EXISTS_ERROR;
        return SBXP_COMMAND_EXISTS_ERROR;
    }

    session->internal_state.msg[0] = '\0';
    session->internal_state.get_signature = 1;
    session->internal_state.code = SBXP_OK;
    args = (sbxp_arguments_t) {.nargs=0};

    code = (*command)(session, &signature, args);

    if (code != SBXP_OK) {
        return code;
    }

    n_state_args = 0;
    for (i = 0; i < signature.ntypes; i++) {
        n_state_args += signature.types[i] == SBXP_STATE_ARG;
    }
    if (n_state_args > 1) {
        sprintf(session->internal_state.msg, "sbx_state_t can only be included as an argument at most one time");
        session->internal_state.code = SBXP_ARG_ERROR;
        return SBXP_ARG_ERROR;
    }

    function.name = strdup(name);
    function.function = command;
    function.signature = signature;

    sbxp_append_function(&session->internal_state.functions, function);

    return SBXP_OK;
}

sbxp_code_t sbx_unregister_command(sbxp_session_t* session, sbxp_function_name_t name) {
    int exists;

    exists = sbxp_remove_function(&session->internal_state.functions, name);

    if (!exists) {
        sprintf(session->internal_state.msg, "No command exists");
        session->internal_state.code = SBXP_NO_COMMAND_ERROR;
        return SBXP_NO_COMMAND_ERROR;
    }

    return SBXP_OK;
}

sbxp_code_t sbxp_call_command(sbxp_session_t* session, char* command_str) {
    int len;
    char *name, *argument_str;
    sbxp_code_t code;

    command_str = strdup(command_str);
    argument_str = sbxp_strip_start(command_str);

    name = sbxp_get_next_token(&argument_str);

    code = sbxp_call_command_with_args(session, name, argument_str);

    free(command_str);
    return code;
}

sbxp_code_t sbxp_call_command_with_args(sbxp_session_t* session, sbxp_function_name_t name, sbxp_argument_string_t arg_str) {
    int i, index, exists;
    sbxp_code_t code;
    sbxp_function_array_t functions;
    sbxp_function_t function;
    sbxp_arguments_t args;

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
        session->internal_state.code = SBXP_NO_COMMAND_ERROR;
        return SBXP_NO_COMMAND_ERROR;
    }
    session->internal_state.curr_func = index;

    args = sbxp_make_arguments(function.signature.ntypes);
    for (i = 0; i < args.nargs; i++) args.args[i].type = function.signature.types[i];

    code = sbxp_parse_arguments(session, function, &args, arg_str);
    if (code != SBXP_OK) {
        sbxp_free_arguments(args);
        return code;
    }

    session->internal_state.get_signature = 0;
    code = (*(function.function))(session, NULL, args);

    sbxp_free_arguments(args);
    return SBXP_OK;
}

sbxp_function_t* sbxp_get_current_function(sbxp_session_t* session) {
    if (!(0 <= session->internal_state.curr_func && session->internal_state.curr_func < session->internal_state.functions.nfuncs))
        return NULL;
    return &session->internal_state.functions.funcs[session->internal_state.curr_func];
}

int sbxp_compare_input_to_signature(sbxp_arguments_t args, const int ntypes, const sbxp_type_t* types) {
    int identical = 1;
    if (args.nargs != ntypes) return 1;

    for (int i = 0; i < args.nargs; i++) {
        identical = identical && (args.args[i].type == types[i]);
    }
    return !identical;
}

sbxp_code_t sbxp_signature_error(sbxp_session_t* session, sbxp_arguments_t args, const int ntypes, const sbxp_type_t* types) {
    char in_sig[SBXP_MAX_ARGS * (SBXP_MAX_TYPE_LEN + 2)];
    char real_sig[SBXP_MAX_ARGS * (SBXP_MAX_TYPE_LEN + 2)];
    sbxp_type_t in_types[SBXP_MAX_ARGS];
    sbxp_function_t* function = sbxp_get_current_function(session);

    if (function == NULL) {return SBXP_UNKNOWN_ERROR;}

    for (int i = 0; i < SBXP_MAX_ARGS; i++) in_types[i] = args.args[i].type;
    sbxp_unparse_types(in_sig, args.nargs, in_types);
    sbxp_unparse_types(real_sig, ntypes, (sbxp_type_t*) types);

    sprintf(session->internal_state.msg, "%s has signature (%s) but recieved inputs of type (%s).", function->name, real_sig, in_sig);

    if (args.nargs != ntypes) {
        session->internal_state.code = SBXP_ARG_ERROR;
        return SBXP_ARG_ERROR;
    } else {
        session->internal_state.code = SBXP_TYPE_ERROR;
        return SBXP_TYPE_ERROR;
    }
}

#endif

