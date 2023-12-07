#include "sbxp_macros.c"
#include "sbxp_data.c"

#include "sbxp_arguments.c"
#include "sbxp_functions.c"
#include "sbxp_session.c"
#include "sbxp_history.c"
#include "sbxp_terminal.c"

#define SBX_FUNC(name, ...) \
    sbxp_code_t name(sbxp_session_t* sbxp_session,                              \
                     sbxp_signature_t* sbxp_signature,                          \
                     sbxp_arguments_t sbxp_function_args) {                     \
        SBXP_CHECK_SIGNATURE(__VA_ARGS__);                                      \
        int sbxp_function_arg_index = 0;                                        \
        sbxp_code_t sbxp_return_code;                                           \
        sbxp_type_t sbxp_argument_type;                                         \
        if (sbxp_session->internal_state.get_signature) {                       \
            sbxp_signature->ntypes = SBXP_COUNT_ARGS(__VA_ARGS__);              \
            sbxp_return_code = sbxp_parse_types(                                \
                sbxp_session,                                                   \
                sbxp_signature->types,                                          \
                #__VA_ARGS__                                                    \
            );                                                                  \
            return sbxp_return_code;                                            \
        }                                                                       \
        SBXP_MAKE_ARGS_(SBXP_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__);

#define SBX_END \
        }                                                                       \
    sbxp_session->internal_state.code = SBXP_OK;                                \
    return SBXP_OK;

#define SBX_REGISTER_COMMAND(session, func) \
    sbx_register_command(session, func, #func)
#define SBX_UNREGISTER_COMMAND(session, func) \
    sbx_unregister_command(session, #func)

#define SBXP_ARG(arg_type, name) \
    sbxp_return_code = sbxp_parse_type(sbxp_session, &sbxp_argument_type, #arg_type); \
    if (sbxp_return_code != SBXP_OK) {                                          \
        return sbxp_return_code;                                                \
    }                                                                           \
    if (sbxp_function_args.args[sbxp_function_arg_index].type != sbxp_argument_type) { \
        sprintf(                                                                \
            sbxp_session->internal_state.msg,                                   \
            "argument %d expected " #arg_type ", got %s instead",               \
            sbxp_function_arg_index + 1,                                        \
            sbxp_unparse_type(sbxp_function_args.args[sbxp_function_arg_index].type) \
        );                                                                      \
        sbxp_session->internal_state.code = SBXP_TYPE_ERROR;                    \
        return SBXP_TYPE_ERROR;                                                 \
    } \
    arg_type name = *((arg_type*) sbxp_unpack_argument(                         \
        &sbxp_function_args.args[sbxp_function_arg_index]                       \
    ));                                                                         \
    sbxp_function_arg_index += 1

SBX_FUNC(sbxp_exit) {
    sbxp_session->internal_state.exit = 1;
    SBX_END;
}

SBX_FUNC(sbxp_list_commands) {
    int i, ncommands;
    sbxp_function_array_t functions;

    functions = sbxp_session->internal_state.functions;
    ncommands = functions.nfuncs;

    printf("Available commands:\n");
    for (i = 0; i < ncommands; i++) {
        printf("    %d: %s\n", i, functions.funcs[i].name);
    }
    SBX_END;
}

SBX_FUNC(sbxp_tell, char*, command) {
    int i, ncommands, exists;
    char sig_str[SBXP_MAX_ARGS * (SBXP_MAX_TYPE_LEN + 2)];
    sbxp_function_array_t functions;
    sbxp_signature_t sig;
    
    functions = sbxp_session->internal_state.functions;
    ncommands = functions.nfuncs;
    for (i = 0, exists = 0; i < ncommands; i++) {
        if (strcmp(functions.funcs[i].name, command) == 0) {
            exists = 1;
            sig = functions.funcs[i].signature;
            sbxp_unparse_types(sig_str, sig.ntypes, sig.types);
        }
    }

    if (exists) {
        printf("%s has signature (%s)\n", command, sig_str);
    } else {
        printf("Command with name \"%s\" not found.\n", command);
    }

    SBX_END;
}


void sbx_start_session(sbxp_session_t* session) {
    sbxp_code_t has_exit, has_tell, has_list;
    has_list = sbx_register_command(session, sbxp_list_commands, "list_commands");
    has_tell = sbx_register_command(session, sbxp_tell, "tell");
    has_exit = sbx_register_command(session, sbxp_exit, "exit");

    sbxp_intepreter(session);

    if (has_exit != SBXP_COMMAND_EXISTS_ERROR) {
        sbx_unregister_command(session, "exit");
    }
    if (has_tell != SBXP_COMMAND_EXISTS_ERROR) {
        sbx_unregister_command(session, "tell");
    }
    if (has_list != SBXP_COMMAND_EXISTS_ERROR) {
        sbx_unregister_command(session, "list_commands");
    }
}

