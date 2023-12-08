#ifndef SBXP_TERMINAL_H
#define SBXP_TERMINAL_H

#include <string.h>
#include <termios.h>

#include "sbxp_data.c"
#include "sbxp_history.c"
#include "sbxp_session.c"

void sbxp_intepreter(sbxp_session_t*);
sbxp_code_t sbxp_get_command(sbxp_session_t*, char*);
int sbxp_command_from_partial(sbxp_session_t*, char*, char**);
int sbxp_is_directional_key(int, sbxp_key_press_t*);
int sbxp_getchar(sbxp_key_press_t*);

void sbxp_write_char(char, char*, int, int);
void sbxp_backspace(char*, int, int);
void sbxp_clear_line(int);
int sbxp_max_match(char*, char*);


void sbxp_intepreter(sbxp_session_t* session) {
    char command[SBXP_COMMAND_SIZE + 1], **cmd;
    sbxp_code_t code;

    command[0] = '\0';
    while (!session->internal_state.exit) {
        printf("%s", session->state.prefix);
        fflush(stdout);

        code = sbxp_get_command(session, command);

        printf("\n");
        if (*command == '\0') {continue;}

        code = sbxp_call_command(session, command);
        if (code != SBXP_OK) {
            printf("%s\n", session->internal_state.msg);
        }
    }
}

sbxp_code_t sbxp_get_command(sbxp_session_t* session, char* command) {
    int index, len, i, ntok, history_index, history_size;
    char c, entry[SBXP_COMMAND_SIZE + 1], *completion;
    sbxp_key_press_t key;
    sbxp_history_item_t *item, *temp;

    index = 0;
    len = 0;
    command[0] = '\0';
    entry[0] = '\0';

    item = sbxp_new_history_item(&session->internal_state.history);
    item->command = entry;
    history_index = 0;
    history_size = sbxp_history_size(&session->internal_state.history);
    while ((c = sbxp_getchar(&key)) != '\n') {
        switch (key) {
            case (SBXP_UP_KEY):
                if (history_index < history_size - 1) {
                    if (history_index == 0) {
                        strcpy(item->command, command);
                    }

                    history_index += 1;
                    temp = sbxp_get_history_item(&session->internal_state.history, history_index);

                    sbxp_clear_line(strlen(session->state.prefix) + strlen(command));
                    printf("%s%s", session->state.prefix, temp->command);
                    strcpy(command, temp->command);
                    index = len = strlen(command);
                }
                continue;
            case (SBXP_DOWN_KEY):
                if (history_index > 0) {
                    history_index -= 1;
                    temp = sbxp_get_history_item(&session->internal_state.history, history_index);

                    sbxp_clear_line(strlen(session->state.prefix) + strlen(command));
                    printf("%s%s", session->state.prefix, temp->command);
                    strcpy(command, temp->command);
                    index = len = strlen(command);
                }
                continue;
            case (SBXP_RIGHT_KEY):
                if (index < len) {
                    printf("%c", command[index++]);
                }
                continue;
            case (SBXP_LEFT_KEY):
                if (index > 0) {
                    index--;
                    printf("\b");
                }
                continue;
            case (SBXP_UNKNOWN_KEY):
                continue;
            case (SBXP_NORMAL_KEY):
                break;
        }

        if (c == 127 || c == 8) {
            if (index > 0) {
                sbxp_backspace(command, index, len);
                len--; index--;
            }
        } else if (c == '\t') {
            ntok = sbxp_count_tokens(command);
            if (ntok == 1) {
                int completion_len = sbxp_command_from_partial(session, command, &completion);
                if (completion != NULL) {
                    for (i = len; i < completion_len; i++) {
                        printf("%c", completion[i]);
                        command[i] = completion[i];
                        len++; index++;
                    }
                } else {
                    /* Tab completion failed */
                }
            }
        } else if (len < SBXP_COMMAND_SIZE) {
            sbxp_write_char(c, command, index, len);
            len++; index++;
        } else {
            /* command buffer filled */
        }

        command[len] = '\0';
        fflush(stdout);
    }
    
    command[len] = '\0';
    item->command = strdup(command);

    return SBXP_OK;
}

int sbxp_command_from_partial(sbxp_session_t* session, char* command, char** completion) {
    int i, index, matches, len, max_len, temp;
    sbxp_function_array_t functions;

    len = strlen(command);
    functions = session->internal_state.functions;
    max_len = 0;
    *completion = NULL;
    for (i = 0, matches = 0; i < functions.nfuncs; i++) {
        if (strncmp(functions.funcs[i].name, command, len) == 0) {
            index = i;
            matches += 1;

            if (*completion == NULL) {
                *completion = functions.funcs[i].name;
                max_len = strlen(*completion);
            } else {
                temp = sbxp_max_match(*completion, functions.funcs[i].name);
                max_len = temp < max_len ? temp : max_len;
            }
        }
    }

    return max_len;
}

int sbxp_max_match(char* s1, char* s2) {
    int match, s1_len, s2_len;

    for (int i = 0; s1[i] != '\0' && s2[i] != '\0'; i++) {
        if (s1[i] == s2[i]) {
            match++;
        } else {
            break;
        }
    }

    return match;
}

int sbxp_getchar(sbxp_key_press_t* key) {
   struct termios oldtc;
   struct termios newtc;
   int c, is_directional;

   tcgetattr(fileno(stdin), &oldtc);
   newtc = oldtc;

   newtc.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(fileno(stdin), TCSANOW, &newtc);

   c = getchar();

   *key = SBXP_NORMAL_KEY;

   if (sbxp_is_directional_key(c, key)) {
       c = '\0';
   }

   tcsetattr(fileno(stdin), TCSANOW, &oldtc);
   return c;
}

int sbxp_is_directional_key(int c, sbxp_key_press_t* key) {
    if (c != 27) {return 0;}

    if (getchar() != 91) {*key = SBXP_UNKNOWN_KEY; return 0;}

    switch (getchar()) {
        case (65):
            *key = SBXP_UP_KEY;
            break;
        case (66):
            *key = SBXP_DOWN_KEY;
            break;
        case (67):
            *key = SBXP_RIGHT_KEY;
            break;
        case (68):
            *key = SBXP_LEFT_KEY;
            break;
        default:
            *key = SBXP_UNKNOWN_KEY;
            return 0;
    }

    return 1;
}

void sbxp_write_char(char c, char* command, int index, int len) {
    int i;

    printf("%c%s", c, command + index);
    for (i = index; i < len; i++) {
        printf("\b");
    }

    for (i = len - 1; i >= index; i--) {
        command[i + 1] = command[i];
    }

    command[index] = c;
}

void sbxp_backspace(char* command, int index, int len) {
    printf("\b%s ", command + index);
    for (int i = index - 1; i < len; i++) {
        command[i] = command[i + 1];
        printf("\b");
    }
}

void sbxp_clear_line(int len) {
    printf("\r%*c\r", len, ' ');
}

#endif

