

#ifndef PNIO_CONF_TEST_H
#define PNIO_CONF_TEST_H


/* define compile-time assert */
#define CASSERT_merge(a, b)     a##b
#define CASSERT_cond(p, l)      typedef char CASSERT_merge(assertion_failed_, l)[2*!!(p)-1]
#define CASSERT(p)              CASSERT_cond(p, __LINE__)


#endif /* PNIO_CONF_TEST_H */
