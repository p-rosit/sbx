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

