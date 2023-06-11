#ifndef DBG_H
#define DBG_H

#ifdef DEBUG
#define DBG(...)                                                                         \
    do {                                                                                 \
        printf("[%s:%d] ", __FILE__, __LINE__);                                          \
        printf(__VA_ARGS__);                                                             \
        puts("");                                                                        \
    } while (0)
#else
#define DBG(...)

#define ERR(...)                                                                         \
    do {                                                                                 \
        fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);                                 \
        fprintf(stderr, __VA_ARGS__);                                                    \
        fprintf(stderr, "\n");                                                           \
        exit(EXIT_FAILURE);                                                              \
    } while (0)
#endif

#endif /* DBG_H */
