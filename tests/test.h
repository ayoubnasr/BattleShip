#ifndef TEST_H
#define TEST_H

/**
 * A tiny unit-testing helper so the tests need nothing but a C compiler.
 *
 * Every test is a void function. Inside it, use CHECK / CHECK_EQ_INT / CHECK_EQ_STR. A failed check prints where it failed and the test
 * keeps going, so one run reports every failure. RUN_TEST() is called from the test runner (test_main.c).
 */

#include <stdio.h>
#include <string.h>

extern int tests_run;
extern int tests_failed;
extern int checks_failed;

#define CHECK(cond) do { \
        if (!(cond)) { \
            checks_failed++; \
            fprintf(stderr, "    FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while (0)

#define CHECK_EQ_INT(actual, expected) do { \
        long long a_ = (long long)(actual), e_ = (long long)(expected); \
        if (a_ != e_) { \
            checks_failed++; \
            fprintf(stderr, "    FAIL %s:%d: %s == %lld, expected %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
        } \
    } while (0)

#define CHECK_EQ_STR(actual, expected) do { \
        const char *a_ = (actual), *e_ = (expected); \
        if (a_ == NULL || strcmp(a_, e_) != 0) { \
            checks_failed++; \
            fprintf(stderr, "    FAIL %s:%d: %s == \"%s\", expected \"%s\"\n", __FILE__, __LINE__, #actual, a_ ? a_ : "(null)", e_); \
        } \
    } while (0)

#define RUN_TEST(fn) do { \
        int before_ = checks_failed; \
        tests_run++; \
        fn(); \
        if (checks_failed != before_) { tests_failed++; fprintf(stderr, "[FAIL] %s\n", #fn); } \
        else printf("[ ok ] %s\n", #fn); \
    } while (0)

#endif
