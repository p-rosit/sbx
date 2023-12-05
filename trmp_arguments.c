#ifndef TRMP_PARSE_H
#define TRMP_PARSE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "trmp_data.c"

#define TRMP_MAX_TYPE_LEN (11)

#define TRMP_IS_ARG(string, type) strncmp(string, type, sizeof type - 1) == 0
const char trmp_bool_type[]   = "bool";
const char trmp_int_type[]    = "int";
const char trmp_double_type[] = "double";
const char trmp_string_type[] = "char*";
const char trmp_state_type[]  = "trm_state_t*";
const char trmp_unknown[]     = "error: could not parse type";

void*       trmp_unpack_argument(trmp_argument_t*);

trmp_arguments_t trmp_make_arguments(int);
void             trmp_free_arguments(trmp_arguments_t);

int         trmp_count_types(const char *str_types);
trmp_code_t trmp_parse_types(trmp_session_t*, trmp_type_t*, const char*);
trmp_code_t trmp_parse_type(trmp_session_t*, trmp_type_t*, const char*);
void        trmp_unparse_types(char*, int, trmp_type_t*);
const char* trmp_unparse_type(trmp_type_t);

trmp_code_t trmp_parse_arguments(trmp_session_t*, trmp_function_t, trmp_arguments_t*, trmp_argument_string_t);
trmp_code_t trmp_parse_argument(trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
trmp_code_t trmp_parse_bool(     trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
trmp_code_t trmp_parse_int(      trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
trmp_code_t trmp_parse_double(   trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
trmp_code_t trmp_parse_string(   trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
trmp_code_t trmp_parse_state_arg(trmp_session_t*, trmp_argument_t*, trmp_argument_string_t*);
int trmp_count_arguments(trmp_argument_string_t);

char* trmp_strip_start(char*);
char* trmp_get_next_token(char**);


void* trmp_unpack_argument(trmp_argument_t* arg) {
    switch (arg->type) {
        case TRMP_BOOL:
            return &arg->data.bool_type;
        case TRMP_INT:
            return &arg->data.int_type;
        case TRMP_DOUBLE:
            return &arg->data.double_type;
        case TRMP_STRING:
            return &arg->data.string_type;
        case TRMP_STATE_ARG:
            return &arg->data.trm_state;
    }
}

trmp_arguments_t trmp_make_arguments(int nargs) {
    trmp_arguments_t args;
    args.nargs = nargs;
    
    for (int i = 0; i < TRMP_MAX_ARGS; i++) {
        args.args[i].data.string_type = NULL;
    }

    return args;
}

void trmp_free_arguments(trmp_arguments_t args) {
    for (int i = 0; i < args.nargs; i++) {
        if (args.args[i].type == TRMP_STRING) {
            free(args.args[i].data.string_type);
        }
    }
}

trmp_code_t trmp_parse_arguments(trmp_session_t* session, trmp_function_t function, trmp_arguments_t* args, trmp_argument_string_t arg_str) {
    int i, nargs;
    char *token, func_sig[TRMP_MAX_ARGS * (TRMP_MAX_TYPE_LEN + 2)];
    trmp_type_t types[TRMP_MAX_ARGS];
    trmp_code_t code;
    
    nargs = trmp_count_arguments(arg_str);
    for (i = 0; i < args->nargs; i++) nargs += TRMP_STATE_ARG == args->args[i].type;

    if (nargs != args->nargs) {
        for (i = 0; i < TRMP_MAX_ARGS; i++) types[i] = args->args[i].type;
        trmp_unparse_types(func_sig, args->nargs, types);
        sprintf(session->internal_state.msg, "%s has signature (%s) but recieved %d arguments", function.name, func_sig, nargs);
        session->internal_state.code = TRMP_ARG_ERROR;
        return TRMP_ARG_ERROR;
    }

    for (i = 0; i < nargs; i++) {
        code = trmp_parse_argument(session, &args->args[i], &arg_str);
        if (code != TRMP_OK) {
            char* copy = strdup(session->internal_state.msg);
            sprintf(session->internal_state.msg, "error parsing argument %d, %s", i, copy);
            session->internal_state.code = TRMP_ARG_ERROR;
            free(copy);
            return code;
        }
    }

    return TRMP_OK;
}

trmp_code_t trmp_parse_argument(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    if (**arg_str == '\0' && arg->type != TRMP_STATE_ARG) {
        sprintf(session->internal_state.msg, "unxpected EOL");
        session->internal_state.code = TRMP_PARSE_ERROR;
        return TRMP_PARSE_ERROR;
    }

    *arg_str = trmp_strip_start(*arg_str);

    if (**arg_str == ',') {
        sprintf(session->internal_state.msg, "unexpected character \",\"");
        session->internal_state.code = TRMP_PARSE_ERROR;
        return TRMP_PARSE_ERROR;
    }
    
    switch (arg->type) {
        case (TRMP_BOOL):
            return trmp_parse_bool(session, arg, arg_str);
        case (TRMP_INT):
            return trmp_parse_int(session, arg, arg_str);
        case (TRMP_DOUBLE):
            return trmp_parse_double(session, arg, arg_str);
        case (TRMP_STRING):
            return trmp_parse_string(session, arg, arg_str);
        case (TRMP_STATE_ARG):
            return trmp_parse_state_arg(session, arg, arg_str);
        case (TRMP_UNKNOWN):
            break;
    }
    sprintf(session->internal_state.msg, "internal error, could not parse unknown type: %s", *arg_str);
    session->internal_state.code = TRMP_UNKNOWN_ERROR;
    return TRMP_UNKNOWN_ERROR;
}

trmp_code_t trmp_parse_bool(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    if (strncmp(*arg_str, "true", 4) == 0 || strncmp(*arg_str, "True", 4) == 0) {
        *arg_str += 4;
        arg->data.bool_type = 1;
    } else if (strncmp(*arg_str, "false", 5) == 0 || strncmp(*arg_str, "False", 5) == 0) {
        *arg_str += 5;
        arg->data.bool_type = 0;
    } else {
        sprintf(session->internal_state.msg, "could not parse bool, expected true or false, got \"%s\"", *arg_str);
        session->internal_state.code = TRMP_PARSE_ERROR;
        return TRMP_PARSE_ERROR;
    }

    return TRMP_OK;
}

trmp_code_t trmp_parse_int(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    // TODO: safe parsing
    int res = 0;

    while (**arg_str != ' ' && **arg_str != '\0') {
        if (!isdigit(**arg_str)) {
            sprintf(session->internal_state.msg, "unexpected non-digit character encountered while trying to parse int\n");
            session->internal_state.code = TRMP_PARSE_ERROR;
            return TRMP_PARSE_ERROR;
        }
        res = 10 * res + (**arg_str - '0');

        (*arg_str)++;
    }

    arg->data.int_type = res;

    return TRMP_OK;
}

trmp_code_t trmp_parse_double(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    char *end_ptr = *arg_str;
    arg->data.double_type = strtod(*arg_str, &end_ptr);
    if (*arg_str == end_ptr) {
        sprintf(session->internal_state.msg, "could not parse double");
        session->internal_state.code = TRMP_PARSE_ERROR;
        return TRMP_PARSE_ERROR;
    }
    *arg_str = end_ptr;

    return TRMP_OK;
}

trmp_code_t trmp_parse_string(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    int len, has_end;
    char* string;

    if (**arg_str != '\"') {
        sprintf(session->internal_state.msg, "expected string to start with \", got %c", **arg_str);
        return TRMP_PARSE_ERROR;
    }
    *arg_str += 1;

    len = 0;
    while ((*arg_str)[len] != '"' && (*arg_str)[len] != '\0') {len++;}
    if ((*arg_str)[len] == '\0') {
        sprintf(session->internal_state.msg, "unexpected EOL, expected string to end with \"");
        return TRMP_PARSE_ERROR;
    }

    string = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        string[i] = **arg_str;
        *arg_str += 1;
    }
    string[len] = '\0';

    arg->data.string_type = string;

    return TRMP_OK;
}

trmp_code_t trmp_parse_state_arg(trmp_session_t* session, trmp_argument_t* arg, trmp_argument_string_t* arg_str) {
    arg->data.trm_state = &session->state;
    return TRMP_OK;
}

int trmp_count_types(const char *str_types) {
    int total = 0;

    if (*str_types == '\0') return total;

    total = 1;
    for (char c = *str_types++; c != '\0'; c = *str_types++) {
        total += c == ' ';
    }

    return total;
}

trmp_code_t trmp_parse_types(trmp_session_t* session, trmp_type_t* types, const char* str_types) {
    int index, is_type, is_end;
    char *copy, *type, *temp;
    trmp_code_t code;
    index = 0; is_type = 1;
    copy = type = strdup(str_types);


    while (*type != '\0') {
        temp = type;
        while (isalpha(*temp) || *temp == '_' || *temp == '*') temp += 1;
        is_end = *temp == '\0';
        *temp = '\0';

        if (is_type) {
            code = trmp_parse_type(session, &types[index++], type);
            if (code != TRMP_OK) {
                free(copy);
                return code;
            }
        }

        if (is_end) {
            break;
        }

        is_type = !is_type;

        temp += 1;
        while (!isalpha(*temp) && *temp != '_' && *temp != '*' && *temp != '\0') temp += 1;
        type = temp;
    }

    free(copy);

    return TRMP_OK;
}

trmp_code_t trmp_parse_type(trmp_session_t* session, trmp_type_t* type, const char* str_type) {
    if (TRMP_IS_ARG(str_type, "bool") || TRMP_IS_ARG(str_type, "_Bool")) {
        *type = TRMP_BOOL;
    } else if (TRMP_IS_ARG(str_type, trmp_int_type)) {
        *type = TRMP_INT;
    } else if (TRMP_IS_ARG(str_type, trmp_double_type)) {
        *type = TRMP_DOUBLE;
    } else if (TRMP_IS_ARG(str_type, "char*") || TRMP_IS_ARG(str_type, "string")) {
        *type = TRMP_STRING;
    } else if (TRMP_IS_ARG(str_type, trmp_state_type)) {
        *type = TRMP_STATE_ARG;
    } else {
        *type = TRMP_UNKNOWN;
        sprintf(session->internal_state.msg, "could not parse type \"%s\"", str_type);
        session->internal_state.code = TRMP_PARSE_ERROR;
        return TRMP_PARSE_ERROR;
    }
    return TRMP_OK;
}

void trmp_unparse_types(char* return_str, int ntypes, trmp_type_t* types) {
    char *str_types;
    const char *type;
    return_str[0] = '\0';

    str_types = return_str;
    for (int i = 0; i < ntypes; i++) {
        type = trmp_unparse_type(types[i]);
        sprintf(str_types, "%s, ", type);
        str_types += strlen(type) + 2;
    }

    if (ntypes > 0) *(str_types - 2) = '\0';
}

const char* trmp_unparse_type(trmp_type_t type) {
    switch (type) {
        case (TRMP_BOOL):
            return trmp_bool_type;
        case (TRMP_INT):
            return trmp_int_type;
        case (TRMP_DOUBLE):
            return trmp_double_type;
        case (TRMP_STRING):
            return trmp_string_type;
        case (TRMP_STATE_ARG):
            return trmp_state_type;
        case (TRMP_UNKNOWN):
            return trmp_unknown;
    }
}

char* trmp_strip_start(char* string) {
    while (isspace(*string)) {
        string += 1;
    }
    return string;
}

char* trmp_get_next_token(char** string_ptr) {
    char *token;
    
    *string_ptr = token = trmp_strip_start(*string_ptr);
    while (!isspace(**string_ptr) && **string_ptr != '\0') {
        *string_ptr += 1;
    }

    if (**string_ptr != '\0') {
        **string_ptr = '\0';
        *string_ptr += 1;
    }

    return token;
}

int trmp_count_arguments(trmp_argument_string_t arg_str) {
    int was_space, is_space, in_string, nargs;
    char c;

    in_string = 0;
    is_space = 1;
    nargs = 0;

    for (c = *arg_str++; c != '\0'; c = *arg_str++) {
        if (c == '"') {
            nargs += !in_string;
            in_string = !in_string;
        }

        if (!in_string) {
            was_space = is_space;
            is_space = isspace(c);

            nargs += was_space && !is_space;
        } else {
            was_space = 0;
            is_space = 0;
        }
    }

    return nargs;
}

#endif

