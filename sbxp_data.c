#ifndef SBXP_DATA_H
#define SBXP_DATA_H

#define SBXP_MAX_ARGS     (8)
#define SBXP_MESSAGE_SIZE (2048)
#define SBXP_COMMAND_SIZE (100)

typedef struct sbxp_arguments           sbxp_arguments_t;
typedef struct sbxp_argument            sbxp_argument_t;
typedef union  sbxp_data                sbxp_data_t;

typedef struct sbx_state                sbx_state_t;
typedef struct sbxp_internal_state      sbxp_internal_state_t;
typedef struct sbxp_session             sbxp_session_t;

typedef struct sbxp_function_array      sbxp_function_array_t;
typedef struct sbxp_function            sbxp_function_t;
typedef struct sbxp_signature           sbxp_signature_t;

typedef struct sbxp_history             sbxp_history_t;
typedef struct sbxp_history_item        sbxp_history_item_t;

typedef enum sbxp_code {
    SBXP_OK,
    SBXP_COMMAND_EXISTS_ERROR,
    SBXP_NO_COMMAND_ERROR,
    SBXP_PARSE_ERROR,
    SBXP_ARG_ERROR,
    SBXP_TYPE_ERROR,
    SBXP_INPUT_ERROR,
    SBXP_UNKNOWN_ERROR,
} sbxp_code_t;

typedef enum sbxp_type {
    SBXP_BOOL,
    SBXP_INT,
    SBXP_DOUBLE,
    SBXP_STRING,
    SBXP_STATE_ARG,
    SBXP_UNKNOWN,
} sbxp_type_t;

typedef enum sbxp_key_press {
    SBXP_NORMAL_KEY,
    SBXP_UP_KEY,
    SBXP_DOWN_KEY,
    SBXP_LEFT_KEY,
    SBXP_RIGHT_KEY,
    SBXP_UNKNOWN_KEY,
} sbxp_key_press_t;

typedef sbxp_code_t (*sbxp_command_t)(sbxp_session_t*, sbxp_signature_t*, sbxp_arguments_t);
typedef char* sbxp_function_name_t;
typedef char* sbxp_command_signature_t;
typedef char* sbxp_argument_string_t;

union sbxp_data {
    _Bool bool_type;
    int int_type;
    double double_type;
    char* string_type;
    sbx_state_t* sbx_state;
};

struct sbxp_argument {
    sbxp_type_t type;
    sbxp_data_t data;
};

struct sbxp_arguments {
    int nargs;
    sbxp_argument_t args[SBXP_MAX_ARGS];
};

struct sbxp_signature {
    int ntypes;
    sbxp_type_t types[SBXP_MAX_ARGS];
};

struct sbxp_function {
    sbxp_function_name_t name;
    sbxp_signature_t signature;
    sbxp_command_t function;
};

struct sbxp_function_array {
    int nfuncs;
    sbxp_function_t* funcs;
};

struct sbxp_history_item {
    char* function_name;
    char* command;
};

struct sbxp_history {
    int size;
    int first_index;
    int last_index;
    sbxp_history_item_t* items;
};

struct sbxp_internal_state {
    int exit;
    sbxp_code_t code;
    char msg[SBXP_MESSAGE_SIZE];

    int curr_func;
    int get_signature;
    sbxp_function_array_t functions;

    sbxp_history_t history;
};

struct sbx_state {
    char* prefix;
#ifdef SBX_STATE
    SBX_STATE
#endif
};

struct sbxp_session {
    sbxp_internal_state_t internal_state;
    sbx_state_t state;
};

#endif

