#ifndef TRMP_DATA_H
#define TRMP_DATA_H

#ifndef TRM_STATE
    #define TRM_STATE
#endif

#define TRMP_MAX_ARGS     (8)
#define TRMP_MESSAGE_SIZE (2048)
#define TRMP_COMMAND_SIZE (100)

typedef enum   trmp_type                trmp_type_t;
typedef struct trmp_arguments           trmp_arguments_t;
typedef struct trmp_argument            trmp_argument_t;
typedef union  trmp_data                trmp_data_t;

typedef struct trm_state                trm_state_t;
typedef struct trmp_internal_state      trmp_internal_state_t;
typedef struct trmp_session             trmp_session_t;
typedef enum   trmp_code                trmp_code_t;

typedef struct trmp_function_array      trmp_function_array_t;
typedef struct trmp_function            trmp_function_t;
typedef struct trmp_signature           trmp_signature_t;

typedef enum   trmp_key_press           trmp_key_press_t;
typedef struct trmp_history             trmp_history_t;
typedef struct trmp_history_item        trmp_history_item_t;

typedef trmp_code_t (*trmp_command_t)(trmp_session_t*, trmp_signature_t*, trmp_arguments_t);
typedef char* trmp_function_name_t;
typedef char* trmp_command_signature_t;
typedef char* trmp_argument_string_t;


enum trmp_code {
    TRMP_OK,
    TRMP_COMMAND_EXISTS_ERROR,
    TRMP_NO_COMMAND_ERROR,
    TRMP_PARSE_ERROR,
    TRMP_ARG_ERROR,
    TRMP_TYPE_ERROR,
    TRMP_INPUT_ERROR,
    TRMP_UNKNOWN_ERROR,
};

enum trmp_type {
    TRMP_BOOL,
    TRMP_INT,
    TRMP_DOUBLE,
    TRMP_STRING,
    TRMP_STATE_ARG,
    TRMP_UNKNOWN,
};

enum trmp_key_press {
    TRMP_NORMAL_KEY,
    TRMP_UP_KEY,
    TRMP_DOWN_KEY,
    TRMP_LEFT_KEY,
    TRMP_RIGHT_KEY,
    TRMP_UNKNOWN_KEY,
};

union trmp_data {
    _Bool bool_type;
    int int_type;
    double double_type;
    char* string_type;
    trm_state_t* trm_state;
};

struct trmp_argument {
    trmp_type_t type;
    trmp_data_t data;
};

struct trmp_arguments {
    int nargs;
    trmp_argument_t args[TRMP_MAX_ARGS];
};

struct trmp_signature {
    int ntypes;
    trmp_type_t types[TRMP_MAX_ARGS];
};

struct trmp_function {
    trmp_function_name_t name;
    trmp_signature_t signature;
    trmp_command_t function;
};

struct trmp_function_array {
    int nfuncs;
    trmp_function_t* funcs;
};

struct trmp_history_item {
    char* function_name;
    char* command;
};

struct trmp_history {
    int size;
    int first_index;
    int last_index;
    trmp_history_item_t* items;
};

struct trmp_internal_state {
    int exit;
    trmp_code_t code;
    char msg[TRMP_MESSAGE_SIZE];

    int curr_func;
    int get_signature;
    trmp_function_array_t functions;

    trmp_history_t history;
};

struct trm_state {
    char* prefix;
    TRM_STATE
};

struct trmp_session {
    trmp_internal_state_t internal_state;
    trm_state_t state;
};

#endif

