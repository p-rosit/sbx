#include <stdio.h>

#define SBX_STATE \
    int flag; \
    double another_flag; \

#include "sbx.c"

SBX_FUNC(print) {
    printf("Printing thing!\n");
    SBX_END;
}

SBX_FUNC(print_thing, _Bool, b, int, n, double, d, char*, s) {
    printf("Input: (%d) (%d) (%lf) (%s)\n", b, n, d, s);
    SBX_END;
}

SBX_FUNC(function, sbx_state_t*, state, int, i, double, d) {
    state->flag = i;
    state->another_flag = d;
    SBX_END;
}

SBX_FUNC(read_state, sbx_state_t*, state) {
    printf("Integer flag: %d\n", state->flag);
    printf("Double flag: %lf\n", state->another_flag);
    SBX_END;
}

void sbx_err(sbxp_session_t* session) {
    if (session->internal_state.code != SBXP_OK) {
        printf("%d: %s\n", session->internal_state.code, session->internal_state.msg);
    }
}


int main() {
    sbxp_session_t session;

    session = sbx_make_session("\x1b[32msbx\x1b[0m> ", 50);

    SBX_REGISTER_COMMAND(&session, print);
    SBX_REGISTER_COMMAND(&session, print_thing);
    SBX_REGISTER_COMMAND(&session, function);
    SBX_REGISTER_COMMAND(&session, read_state);

    sbx_start_session(&session);

    sbx_free_session(&session);

    return 0;
}
