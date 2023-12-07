#ifndef SBXP_PARSE_H
#define SBXP_PARSE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "sbxp_data.c"

#define SBXP_MAX_TYPE_LEN (11)

#define SBXP_IS_ARG(string, type) strncmp(string, type, sizeof type - 1) == 0
const char sbxp_bool_type[]   = "bool";
const char sbxp_int_type[]    = "int";
const char sbxp_double_type[] = "double";
const char sbxp_string_type[] = "char*";
const char sbxp_state_type[]  = "sbx_state_t*";
const char sbxp_unknown[]     = "error: could not parse type";

void*       sbxp_unpack_argument(sbxp_argument_t*);

sbxp_arguments_t sbxp_make_arguments(int);
void             sbxp_free_arguments(sbxp_arguments_t);

int         sbxp_count_types(const char *str_types);
sbxp_code_t sbxp_parse_types(sbxp_session_t*, sbxp_type_t*, const char*);
sbxp_code_t sbxp_parse_type(sbxp_session_t*, sbxp_type_t*, const char*);
void        sbxp_unparse_types(char*, int, sbxp_type_t*);
const char* sbxp_unparse_type(sbxp_type_t);

sbxp_code_t sbxp_parse_arguments(sbxp_session_t*, sbxp_function_t, sbxp_arguments_t*, sbxp_argument_string_t);
sbxp_code_t sbxp_parse_argument(sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
sbxp_code_t sbxp_parse_bool(     sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
sbxp_code_t sbxp_parse_int(      sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
sbxp_code_t sbxp_parse_double(   sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
sbxp_code_t sbxp_parse_string(   sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
sbxp_code_t sbxp_parse_state_arg(sbxp_session_t*, sbxp_argument_t*, sbxp_argument_string_t*);
int sbxp_count_arguments(sbxp_argument_string_t);

char* sbxp_strip_start(char*);
char* sbxp_get_next_token(char**);


void* sbxp_unpack_argument(sbxp_argument_t* arg) {
    switch (arg->type) {
        case SBXP_BOOL:
            return &arg->data.bool_type;
        case SBXP_INT:
            return &arg->data.int_type;
        case SBXP_DOUBLE:
            return &arg->data.double_type;
        case SBXP_STRING:
            return &arg->data.string_type;
        case SBXP_STATE_ARG:
            return &arg->data.sbx_state;
    }
}

sbxp_arguments_t sbxp_make_arguments(int nargs) {
    sbxp_arguments_t args;
    args.nargs = nargs;
    
    for (int i = 0; i < SBXP_MAX_ARGS; i++) {
        args.args[i].data.string_type = NULL;
    }

    return args;
}

void sbxp_free_arguments(sbxp_arguments_t args) {
    for (int i = 0; i < args.nargs; i++) {
        if (args.args[i].type == SBXP_STRING) {
            free(args.args[i].data.string_type);
        }
    }
}

sbxp_code_t sbxp_parse_arguments(sbxp_session_t* session, sbxp_function_t function, sbxp_arguments_t* args, sbxp_argument_string_t arg_str) {
    int i, nargs;
    char *token, func_sig[SBXP_MAX_ARGS * (SBXP_MAX_TYPE_LEN + 2)];
    sbxp_type_t types[SBXP_MAX_ARGS];
    sbxp_code_t code;
    
    nargs = sbxp_count_arguments(arg_str);
    for (i = 0; i < args->nargs; i++) nargs += SBXP_STATE_ARG == args->args[i].type;

    if (nargs != args->nargs) {
        for (i = 0; i < SBXP_MAX_ARGS; i++) types[i] = args->args[i].type;
        sbxp_unparse_types(func_sig, args->nargs, types);
        sprintf(session->internal_state.msg, "%s has signature (%s) but recieved %d arguments", function.name, func_sig, nargs);
        session->internal_state.code = SBXP_ARG_ERROR;
        return SBXP_ARG_ERROR;
    }

    for (i = 0; i < nargs; i++) {
        code = sbxp_parse_argument(session, &args->args[i], &arg_str);
        if (code != SBXP_OK) {
            char* copy = strdup(session->internal_state.msg);
            sprintf(session->internal_state.msg, "error parsing argument %d, %s", i, copy);
            session->internal_state.code = SBXP_ARG_ERROR;
            free(copy);
            return code;
        }
    }

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_argument(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    if (**arg_str == '\0' && arg->type != SBXP_STATE_ARG) {
        sprintf(session->internal_state.msg, "unxpected EOL");
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }

    *arg_str = sbxp_strip_start(*arg_str);

    if (**arg_str == ',') {
        sprintf(session->internal_state.msg, "unexpected character \",\"");
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }
    
    switch (arg->type) {
        case (SBXP_BOOL):
            return sbxp_parse_bool(session, arg, arg_str);
        case (SBXP_INT):
            return sbxp_parse_int(session, arg, arg_str);
        case (SBXP_DOUBLE):
            return sbxp_parse_double(session, arg, arg_str);
        case (SBXP_STRING):
            return sbxp_parse_string(session, arg, arg_str);
        case (SBXP_STATE_ARG):
            return sbxp_parse_state_arg(session, arg, arg_str);
        case (SBXP_UNKNOWN):
            break;
    }
    sprintf(session->internal_state.msg, "internal error, could not parse unknown type: %s", *arg_str);
    session->internal_state.code = SBXP_UNKNOWN_ERROR;
    return SBXP_UNKNOWN_ERROR;
}

sbxp_code_t sbxp_parse_bool(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    if (strncmp(*arg_str, "true", 4) == 0 || strncmp(*arg_str, "True", 4) == 0) {
        *arg_str += 4;
        arg->data.bool_type = 1;
    } else if (strncmp(*arg_str, "false", 5) == 0 || strncmp(*arg_str, "False", 5) == 0) {
        *arg_str += 5;
        arg->data.bool_type = 0;
    } else {
        sprintf(session->internal_state.msg, "could not parse bool, expected true or false, got \"%s\"", *arg_str);
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_int(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    // TODO: safe parsing
    int res = 0;

    while (**arg_str != ' ' && **arg_str != '\0') {
        if (!isdigit(**arg_str)) {
            sprintf(session->internal_state.msg, "unexpected non-digit character encountered while trying to parse int\n");
            session->internal_state.code = SBXP_PARSE_ERROR;
            return SBXP_PARSE_ERROR;
        }
        res = 10 * res + (**arg_str - '0');

        (*arg_str)++;
    }

    arg->data.int_type = res;

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_double(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    char *end_ptr = *arg_str;
    arg->data.double_type = strtod(*arg_str, &end_ptr);
    if (*arg_str == end_ptr) {
        sprintf(session->internal_state.msg, "could not parse double");
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }
    *arg_str = end_ptr;

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_string(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    int len, has_end;
    char* string;

    if (**arg_str != '\"') {
        sprintf(session->internal_state.msg, "expected string to start with \", got %c", **arg_str);
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }
    *arg_str += 1;

    len = 0;
    while ((*arg_str)[len] != '"' && (*arg_str)[len] != '\0') {len++;}
    if ((*arg_str)[len] == '\0') {
        sprintf(session->internal_state.msg, "unexpected EOL, expected string to end with \"");
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }

    string = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        string[i] = **arg_str;
        *arg_str += 1;
    }
    string[len] = '\0';

    arg->data.string_type = string;

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_state_arg(sbxp_session_t* session, sbxp_argument_t* arg, sbxp_argument_string_t* arg_str) {
    arg->data.sbx_state = &session->state;
    return SBXP_OK;
}

int sbxp_count_types(const char *str_types) {
    int total = 0;

    if (*str_types == '\0') return total;

    total = 1;
    for (char c = *str_types++; c != '\0'; c = *str_types++) {
        total += c == ' ';
    }

    return total;
}

sbxp_code_t sbxp_parse_types(sbxp_session_t* session, sbxp_type_t* types, const char* str_types) {
    int index, is_type, is_end;
    char *copy, *type, *temp;
    sbxp_code_t code;
    index = 0; is_type = 1;
    copy = type = strdup(str_types);


    while (*type != '\0') {
        temp = type;
        while (isalpha(*temp) || *temp == '_' || *temp == '*') temp += 1;
        is_end = *temp == '\0';
        *temp = '\0';

        if (is_type) {
            code = sbxp_parse_type(session, &types[index++], type);
            if (code != SBXP_OK) {
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

    return SBXP_OK;
}

sbxp_code_t sbxp_parse_type(sbxp_session_t* session, sbxp_type_t* type, const char* str_type) {
    if (SBXP_IS_ARG(str_type, "bool") || SBXP_IS_ARG(str_type, "_Bool")) {
        *type = SBXP_BOOL;
    } else if (SBXP_IS_ARG(str_type, sbxp_int_type)) {
        *type = SBXP_INT;
    } else if (SBXP_IS_ARG(str_type, sbxp_double_type)) {
        *type = SBXP_DOUBLE;
    } else if (SBXP_IS_ARG(str_type, "char*") || SBXP_IS_ARG(str_type, "string")) {
        *type = SBXP_STRING;
    } else if (SBXP_IS_ARG(str_type, sbxp_state_type)) {
        *type = SBXP_STATE_ARG;
    } else {
        *type = SBXP_UNKNOWN;
        sprintf(session->internal_state.msg, "could not parse type \"%s\"", str_type);
        session->internal_state.code = SBXP_PARSE_ERROR;
        return SBXP_PARSE_ERROR;
    }
    return SBXP_OK;
}

void sbxp_unparse_types(char* return_str, int ntypes, sbxp_type_t* types) {
    char *str_types;
    const char *type;
    return_str[0] = '\0';

    str_types = return_str;
    for (int i = 0; i < ntypes; i++) {
        type = sbxp_unparse_type(types[i]);
        sprintf(str_types, "%s, ", type);
        str_types += strlen(type) + 2;
    }

    if (ntypes > 0) *(str_types - 2) = '\0';
}

const char* sbxp_unparse_type(sbxp_type_t type) {
    switch (type) {
        case (SBXP_BOOL):
            return sbxp_bool_type;
        case (SBXP_INT):
            return sbxp_int_type;
        case (SBXP_DOUBLE):
            return sbxp_double_type;
        case (SBXP_STRING):
            return sbxp_string_type;
        case (SBXP_STATE_ARG):
            return sbxp_state_type;
        case (SBXP_UNKNOWN):
            return sbxp_unknown;
    }
}

char* sbxp_strip_start(char* string) {
    while (isspace(*string)) {
        string += 1;
    }
    return string;
}

char* sbxp_get_next_token(char** string_ptr) {
    char *token;
    
    *string_ptr = token = sbxp_strip_start(*string_ptr);
    while (!isspace(**string_ptr) && **string_ptr != '\0') {
        *string_ptr += 1;
    }

    if (**string_ptr != '\0') {
        **string_ptr = '\0';
        *string_ptr += 1;
    }

    return token;
}

int sbxp_count_arguments(sbxp_argument_string_t arg_str) {
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

