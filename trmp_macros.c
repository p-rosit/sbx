#ifndef TRMP_MACROS_H

#define TRMP_SIG_ERR TRM_ERROR_the_macro_TRMP_SIGNATURE_expects_no_arguments_or_arguments_of_the_form__type1_identifier1_type2_identifier2__up_to__type8_identifier8

#define TRMP_MAKE_ARGS_0(...) {}
#define TRMP_MAKE_ARGS_1(type, name, ...) \
    TRMP_ARG(type, name);
#define TRMP_MAKE_ARGS_2(type, name, ...);\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_1(__VA_ARGS__);
#define TRMP_MAKE_ARGS_3(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_2(__VA_ARGS__);
#define TRMP_MAKE_ARGS_4(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_3(__VA_ARGS__);
#define TRMP_MAKE_ARGS_5(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_4(__VA_ARGS__);
#define TRMP_MAKE_ARGS_6(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_5(__VA_ARGS__);
#define TRMP_MAKE_ARGS_7(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_6(__VA_ARGS__);
#define TRMP_MAKE_ARGS_8(type, name, ...)\
    TRMP_ARG(type, name);\
    TRMP_MAKE_ARGS_7(__VA_ARGS__);

#define TRMP_CONCAT(arg1, arg2)             TRMP_CONCAT1(arg1, arg2)
#define TRMP_CONCAT1(arg1, arg2)            TRMP_CONCAT2(arg1, arg2)
#define TRMP_CONCAT2(arg1, arg2)            arg1##arg2

#define TRMP_CHECK_SIGNATURE(...)           TRMP_CHECK_SIGNATURE_(__VA_ARGS__, TRMP_CHECK_SIGNATURE_RSEQ_N())
#define TRMP_CHECK_SIGNATURE_(...)          TRMP_CHECK_SIGNATURE_ARG_N(__VA_ARGS__) 
#define TRMP_CHECK_SIGNATURE_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N 
#define TRMP_CHECK_SIGNATURE_RSEQ_N()       {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, TRMP_SIG_ERR, {}, {}

#define TRMP_COUNT_ARGS(...)                TRMP_COUNT_ARGS_(__VA_ARGS__, TRMP_COUNT_ARGS_RSEQ_N())
#define TRMP_COUNT_ARGS_(...)               TRMP_COUNT_ARGS_N(__VA_ARGS__)
#define TRMP_COUNT_ARGS_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define TRMP_COUNT_ARGS_RSEQ_N()            8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0

#define TRMP_MAKE_ARGS_(N, ...)             TRMP_CONCAT(TRMP_MAKE_ARGS_, N)(__VA_ARGS__)

#endif

