#ifndef TRMP_TERMINAL_H
#define TRMP_TERMINAL_H

#include <string.h>
#include <termios.h>

#include "trmp_data.c"
#include "trmp_history.c"
#include "trmp_session.c"

void trmp_intepreter(trmp_session_t* session);
trmp_code_t trmp_get_command(trmp_session_t* session, char* command);
int trmp_is_directional_key(int, trmp_key_press_t*);
int trmp_getchar(trmp_key_press_t*);


void trmp_intepreter(trmp_session_t* session) {
    char command[TRMP_COMMAND_SIZE + 1], **cmd;
    trmp_code_t code;

    command[0] = '\0';
    while (!session->internal_state.exit) {
        printf("%s", session->state.prefix);
        fflush(stdout);

        code = trmp_get_command(session, command);

        printf("\n");
        if (*command == '\0') {continue;}

        code = trmp_call_command(session, command);
        if (code != TRMP_OK) {
            printf("%s\n", session->internal_state.msg);
        }
    }
}

trmp_code_t trmp_get_command(trmp_session_t* session, char* command) {
    int index, len, i, history_index, history_size;
    char c, curr_command[TRMP_COMMAND_SIZE + 1], entry[TRMP_COMMAND_SIZE + 1];
    trmp_key_press_t key;
    trmp_history_item_t *item, *temp;

    index = 0;
    len = 0;
    command[0] = '\0';
    entry[0] = '\0';

    item = trmp_new_history_item(&session->internal_state.history);
    item->command = entry;
    history_index = 0;
    history_size = trmp_history_size(&session->internal_state.history);
    while ((c = trmp_getchar(&key)) != '\n') {
        switch (key) {
            case (TRMP_UP_KEY):
                if (history_index < history_size - 1) {
                    if (history_index == 0) {
                        strcpy(item->command, command);
                    }

                    history_index += 1;
                    temp = trmp_get_history_item(&session->internal_state.history, history_index);
                    printf("\r%*c%*c\r", (int) strlen(session->state.prefix), ' ', (int) strlen(command), ' ');
                    printf("%s%s", session->state.prefix, temp->command);
                    strcpy(command, temp->command);
                    index = len = strlen(command);
                }
                continue;
            case (TRMP_DOWN_KEY):
                if (history_index > 0) {
                    history_index -= 1;
                    temp = trmp_get_history_item(&session->internal_state.history, history_index);
                    printf("\r%*c%*c\r", (int) strlen(session->state.prefix), ' ', (int) strlen(command), ' ');
                    printf("%s%s", session->state.prefix, temp->command);
                    strcpy(command, temp->command);
                    index = len = strlen(command);
                }
                continue;
            case (TRMP_RIGHT_KEY):
                if (index < len) {
                    printf("%c", command[index++]);
                }
                continue;
            case (TRMP_LEFT_KEY):
                if (index > 0) {
                    index--;
                    printf("\b");
                }
                continue;
            case (TRMP_UNKNOWN_KEY):
                continue;
            case (TRMP_NORMAL_KEY):
                break;
        }

        if (c == 127 || c == 8) {
            if (index > 0) {
                printf("\b%s ", command + index);
                for (i = index - 1; i < len; i++) {
                    command[i] = command[i + 1];
                    printf("\b");
                }
                len--; index--;
            }
        } else if (len < TRMP_COMMAND_SIZE) {
            printf("%c%s", c, command + index);
            for (i = index; i < len; i++) {
                printf("\b");
            }

            for (i = len - 1; i >= index; i--) {
                command[i + 1] = command[i];
            }
            command[index++] = c;
            len++;
        } else {
            // command buffer filled
        }

        command[len] = '\0';
        fflush(stdout);
    }
    
    command[len] = '\0';
    item->command = strdup(command);

    return TRMP_OK;
}

int trmp_getchar(trmp_key_press_t* key) {
   struct termios oldtc;
   struct termios newtc;
   int c, is_directional;

   tcgetattr(fileno(stdin), &oldtc);
   newtc = oldtc;

   newtc.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(fileno(stdin), TCSANOW, &newtc);

   c = getchar();

   *key = TRMP_NORMAL_KEY;

   if (trmp_is_directional_key(c, key)) {
       c = '\0';
   }

   tcsetattr(fileno(stdin), TCSANOW, &oldtc);
   return c;
}

int trmp_is_directional_key(int c, trmp_key_press_t* key) {
    if (c != 27) {return 0;}

    if (getchar() != 91) {*key = TRMP_UNKNOWN_KEY; return 0;}

    switch (getchar()) {
        case (65):
            *key = TRMP_UP_KEY;
            break;
        case (66):
            *key = TRMP_DOWN_KEY;
            break;
        case (67):
            *key = TRMP_RIGHT_KEY;
            break;
        case (68):
            *key = TRMP_LEFT_KEY;
            break;
        default:
            *key = TRMP_UNKNOWN_KEY;
            return 0;
    }

    return 1;
}

#endif

