#include <stdio.h>

#define TRM_STATE \
    int flag; \
    double another_flag; \

#include "trm.c"

TRM_FUNC(print) {
    printf("Printing thing!\n");
    TRM_END;
}

TRM_FUNC(print_thing, _Bool, b, int, n, double, d, char*, s) {
    printf("Input: (%d) (%d) (%lf) (%s)\n", b, n, d, s);
    TRM_END;
}

TRM_FUNC(function, trm_state_t*, state, int, i, double, d) {
    state->flag = i;
    state->another_flag = d;
    TRM_END;
}

TRM_FUNC(read_state, trm_state_t*, state) {
    printf("Integer flag: %d\n", state->flag);
    printf("Double flag: %lf\n", state->another_flag);
    TRM_END;
}

void trm_err(trmp_session_t* session) {
    printf("%d\n", session->internal_state.code);
    if (session->internal_state.code != TRMP_OK) {
        printf("%d: %s\n", session->internal_state.code, session->internal_state.msg);
    }
}


int main() {
    trmp_session_t session;

    session = trm_make_session("\x1b[32mtrm\x1b[0m> ", 50);

    TRM_REGISTER_COMMAND(&session, print);
    TRM_REGISTER_COMMAND(&session, print_thing);
    TRM_REGISTER_COMMAND(&session, function);
    TRM_REGISTER_COMMAND(&session, read_state);
    TRM_UNREGISTER_COMMAND(&session, trmp_exit);
    trm_err(&session);

    trm_start_session(&session);

    trm_free_session(&session);

    return 0;
}
