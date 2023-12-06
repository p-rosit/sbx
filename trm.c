#include "trmp_macros.c"
#include "trmp_data.c"

#include "trmp_arguments.c"
#include "trmp_functions.c"
#include "trmp_session.c"
#include "trmp_history.c"
#include "trmp_terminal.c"

#define TRM_FUNC(name, ...) \
    trmp_code_t name(trmp_session_t* trmp_session,                              \
                     trmp_signature_t* trmp_signature,                          \
                     trmp_arguments_t trmp_function_args) {                     \
        TRMP_CHECK_SIGNATURE(__VA_ARGS__);                                      \
        int trmp_function_arg_index = 0;                                        \
        trmp_code_t trmp_return_code;                                           \
        trmp_type_t trmp_argument_type;                                         \
        if (trmp_session->internal_state.get_signature) {                       \
            trmp_signature->ntypes = TRMP_COUNT_ARGS(__VA_ARGS__);              \
            trmp_return_code = trmp_parse_types(                                \
                trmp_session,                                                   \
                trmp_signature->types,                                          \
                #__VA_ARGS__                                                    \
            );                                                                  \
            return trmp_return_code;                                            \
        }                                                                       \
        TRMP_MAKE_ARGS_(TRMP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__);             \

#define TRM_END } return TRMP_OK;

#define TRM_REGISTER_COMMAND(session, func) \
    trm_register_command(session, func, #func)
#define TRM_UNREGISTER_COMMAND(session, func) \
    trm_unregister_command(session, #func)

#define TRMP_ARG(arg_type, name) \
    trmp_return_code = trmp_parse_type(trmp_session, &trmp_argument_type, #arg_type); \
    if (trmp_return_code != TRMP_OK) {                                          \
        return trmp_return_code;                                                \
    }                                                                           \
    if (trmp_function_args.args[trmp_function_arg_index].type != trmp_argument_type) { \
        sprintf(                                                                \
            trmp_session->internal_state.msg,                                   \
            "argument %d expected " #arg_type ", got %s instead",               \
            trmp_function_arg_index + 1,                                        \
            trmp_unparse_type(trmp_function_args.args[trmp_function_arg_index].type) \
        );                                                                      \
        trmp_session->internal_state.code = TRMP_TYPE_ERROR;                    \
        return TRMP_TYPE_ERROR;                                                 \
    } \
    arg_type name = *((arg_type*) trmp_unpack_argument(                         \
        &trmp_function_args.args[trmp_function_arg_index]                       \
    ));                                                                         \
    trmp_function_arg_index += 1

TRM_FUNC(trmp_exit) {
    trmp_session->internal_state.exit = 1;
    TRM_END;
}

TRM_FUNC(trmp_list_commands) {
    int i, ncommands;
    trmp_function_array_t functions;

    functions = trmp_session->internal_state.functions;
    ncommands = functions.nfuncs;

    printf("Available commands:\n");
    for (i = 0; i < ncommands; i++) {
        printf("    %d: %s\n", i, functions.funcs[i].name);
    }
    TRM_END;
}

TRM_FUNC(trmp_tell, char*, command) {
    int i, ncommands, exists;
    char sig_str[TRMP_MAX_ARGS * (TRMP_MAX_TYPE_LEN + 2)];
    trmp_function_array_t functions;
    trmp_signature_t sig;
    
    functions = trmp_session->internal_state.functions;
    ncommands = functions.nfuncs;
    for (i = 0, exists = 0; i < ncommands; i++) {
        if (strcmp(functions.funcs[i].name, command) == 0) {
            exists = 1;
            sig = functions.funcs[i].signature;
            trmp_unparse_types(sig_str, sig.ntypes, sig.types);
        }
    }

    if (exists) {
        printf("%s has signature (%s)\n", command, sig_str);
    } else {
        printf("Command with name \"%s\" not found.\n", command);
    }

    TRM_END;
}


void trm_start_session(trmp_session_t* session) {
    trmp_code_t has_exit, has_tell, has_list;
    has_list = trm_register_command(session, trmp_list_commands, "list_commands");
    has_tell = trm_register_command(session, trmp_tell, "tell");
    has_exit = trm_register_command(session, trmp_exit, "exit");

    trmp_intepreter(session);

    if (has_exit != TRMP_COMMAND_EXISTS_ERROR) {
        trm_unregister_command(session, "exit");
    }
    if (has_tell != TRMP_COMMAND_EXISTS_ERROR) {
        trm_unregister_command(session, "tell");
    }
    if (has_list != TRMP_COMMAND_EXISTS_ERROR) {
        trm_unregister_command(session, "list_commands");
    }
}

