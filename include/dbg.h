#ifndef DBG_H
#define DBG_H

#ifdef DEBUG
#define DBG(...)                                                                         \
    printf("[%s:%d] ", __FILE__, __LINE__);                                              \
    printf(__VA_ARGS__);                                                                 \
    puts("");
#else
#define DBG(...)

#define ERR(...)                                                                         \
    fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__);                                     \
    fprintf(stderr, __VA_ARGS__);                                                        \
    fprintf(stderr, "\n");                                                               \
    exit(EXIT_FAILURE);
#endif

#endif /* DBG_H */
