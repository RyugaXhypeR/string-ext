#ifndef DBG_H
#define DBG_H

#ifdef DEBUG
#define DBG(...)                                                                         \
    printf("[%s:%d] ", __FILE__, __LINE__);                                              \
    printf(__VA_ARGS__);                                                                 \
    puts("");
#else
#define DBG(...)
#endif

#endif /* DBG_H */
