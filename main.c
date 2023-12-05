#include <stdio.h>

#include "trm.c"

TRM_FUNC(print) {
    printf("Printing thing!\n");
    TRM_END;
}

TRM_FUNC(print_thing, _Bool, b, int, n, double, d, char*, s) {
    printf("Input: (%d) (%d) (%lf) (%s)\n", b, n, d, s);
    TRM_END;
}

void trm_err(trmp_session_t* session) {
    if (session->internal_state.code != TRMP_OK) {
        printf("%d: %s\n", session->internal_state.code, session->internal_state.msg);
    }
}


int main() {
    trmp_session_t session;

    session = trm_make_session("trm> ", 50);

    trm_register_command(&session, trmp_exit, "exit");
    TRM_REGISTER_COMMAND(&session, print);
    TRM_REGISTER_COMMAND(&session, print_thing);

    trmp_intepreter(&session);

    trm_free_session(&session);

    return 0;
}
