/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXTEST_H
#define CXML_CXTEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>


#ifndef CXML_T_SUITE_TEST_THRESHOLD
#define CXML_T_SUITE_TEST_THRESHOLD     (2048)
#endif


// run all tests in all suites provided, and also provide summary test report
#define CXML_TEST_RUNNER(s, n, ...)    \
    _cxml_t_run_suites(s, n, __VA_ARGS__);

// run all tests in all suites provided, but provide no summary test report
#define CXML_TEST_RUNNER_NR(s, n, ...)    \
    _cxml_t_run_suites_nr(s, n, __VA_ARGS__);


#define _BOLD     "\x1B[1m"
#define _GREEN   "\033[32m"
#define _RED     "\033[31m"
#define _YELLOW  "\033[33m"
#define _BLUE    "\033[34m"
#define _RESET   "\033[0m"

#define _cxml_t_fail(_test_name, err_msg)          \
    printf(_RED "[FAILED] - " __FILE__ "::%s at line %d [%s]\n" _RESET, _test_name, __LINE__, err_msg);

#define _cxml_t_todo(_test_name, _msg)          \
    printf(_RED "[FAILED] - " _RESET __FILE__ "::%s at line %d [TODO: %s]\n", _test_name, __LINE__, _msg);

#define _cxml_t_skip(_test_name)     \
    printf(_YELLOW "[SKIPPED] - " _RESET __FILE__ "::%s\n", _test_name);

#define _cxml_t_pass(_test_name)                 \
    printf(_GREEN "[PASSED] - " _RESET __FILE__ "::%s\n", _test_name);

// should be called at the end of the function, in the function body.
#define cxml_pass()  \
    _cxml_t_pass(__func__)     \
    return CXML_T_PASS;

// should be called at the end of the function, in the function body.
#define cxml_todo(_msg)  \
    _cxml_t_todo(__func__, _msg)     \
    return CXML_T_TODO;

// should be called at the end of the function, in the function body.
#define cxml_skip()  \
    _cxml_t_skip(__func__)     \
    return CXML_T_SKIP;

// asserts | helpers
#define cxml_assert_true(_v, err_msg)                   \
    if ((_v) != true) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert(_v)                   \
    if (!(_v)) {_cxml_t_fail(__func__, #_v  " is not true") return CXML_T_FAIL;}

#define cxml_assert__true(_v)                   \
    if ((_v) != true) {_cxml_t_fail(__func__, #_v  " != true") return CXML_T_FAIL;}

#define cxml_assert_false(_v, err_msg)                  \
    if ((_v) != false) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__false(_v)                  \
    if ((_v) != false) {_cxml_t_fail(__func__, #_v " != false") return CXML_T_FAIL;}

#define cxml_assert_eq(_v1, _v2, err_msg)               \
    if ((_v1) != (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__eq(_v1, _v2)               \
    if ((_v1) != (_v2)) {_cxml_t_fail(__func__, #_v1 " != " #_v2) return CXML_T_FAIL;}

#define cxml_assert_neq(_v1, _v2, err_msg)              \
    if ((_v1) == (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__neq(_v1, _v2)              \
    if ((_v1) == (_v2)) {_cxml_t_fail(__func__, #_v1 " == " #_v2) return CXML_T_FAIL;}

#define cxml_assert_gt(_v1, _v2, err_msg)               \
    if ((_v1) <= (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__gt(_v1, _v2)               \
    if ((_v1) <= (_v2)) {_cxml_t_fail(__func__, #_v1 " <= " #_v2) return CXML_T_FAIL;}

#define cxml_assert_lt(_v1, _v2, err_msg)               \
    if ((_v1) >= (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__lt(_v1, _v2)               \
    if ((_v1) >= (_v2)) {_cxml_t_fail(__func__, #_v1 " >= " #_v2) return CXML_T_FAIL;}

#define cxml_assert_geq(_v1, _v2, err_msg)              \
    if ((_v1) < (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__geq(_v1, _v2)              \
    if ((_v1) < (_v2)) {_cxml_t_fail(__func__, #_v1 " < " #_v2) return CXML_T_FAIL;}

#define cxml_assert_leq(_v1, _v2, err_msg)              \
    if ((_v1) > (_v2)) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__leq(_v1, _v2)              \
    if ((_v1) > (_v2)) {_cxml_t_fail(__func__, #_v1 " > " #_v2) return CXML_T_FAIL;}

#define cxml_assert_null(_v, err_msg)                   \
    if ((_v) != NULL) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__null(_v)                   \
    if ((_v) != NULL) {_cxml_t_fail(__func__, #_v " is not NULL") return CXML_T_FAIL;}

#define cxml_assert_not_null(_v, err_msg)               \
    if (_v == NULL) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__not_null(_v)               \
    if (_v == NULL) {_cxml_t_fail(__func__, #_v " is NULL") return CXML_T_FAIL;}

#define cxml_assert_zero(_v, err_msg)                   \
    if (_v != 0) {_cxml_t_fail(__func__, err_msg) return CXML_T_FAIL;}

#define cxml_assert__zero(_v)                   \
    if (_v != 0) {_cxml_t_fail(__func__, #_v " != 0") return CXML_T_FAIL;}

#define cxml_assert__one(_v)                   \
    if (_v != 1) {_cxml_t_fail(__func__, #_v " != 1") return CXML_T_FAIL;}

#define cxml_assert__two(_v)                   \
    if (_v != 2) {_cxml_t_fail(__func__, #_v " != 2") return CXML_T_FAIL;}

#define _CXML_T_GRACEFUL_EXIT(_msg)                     \
    fprintf(stderr, "--CXML TEST Fatal error--%s"      \
            "Exiting gracefully\n", _msg);             \
    exit(EXIT_FAILURE);


typedef enum cxml_test_status{
    CXML_T_PASS=50,
    CXML_T_FAIL,
    CXML_T_SKIP,
    CXML_T_TODO
}cts;

typedef struct{
    int count;
    int capacity;
    int passed;
    int failed;
    int skipped;
    int todo;
    double time_taken;
    const char *name;
    const char *file_name;
    void (*test_setup)();
    void (*test_teardown)();
    cts (*tests[CXML_T_SUITE_TEST_THRESHOLD])(void);
}cxml_t_suite;

struct _cxml_g_runner{
    int passed;
    int failed;
    int skipped;
    int todo;
    int suites_run;
    int sta_tests_run;
    double time_taken;
    cxml_t_suite *current_suite;
};

struct _cxml_g_runner _g_runner;

static void _cxml_t_init_suite(
        cxml_t_suite *suite,
        const char *suit_name,
        const char *file_name)
{
    suite->count = 0;
    suite->capacity = CXML_T_SUITE_TEST_THRESHOLD;
    suite->failed = 0;
    suite->passed = 0;
    suite->skipped = 0;
    suite->todo = 0;
    suite->time_taken = 0;
    suite->test_setup = NULL;
    suite->test_teardown = NULL;
    suite->name = suit_name;
    suite->file_name = file_name;
    _g_runner.current_suite = suite;
}

static void _cxml_t_init_g_runner(){
    _g_runner.passed = 0;
    _g_runner.failed = 0;
    _g_runner.skipped = 0;
    _g_runner.todo = 0;
    _g_runner.suites_run = 0;
    _g_runner.sta_tests_run = 0;
    _g_runner.time_taken = 0;
    _g_runner.current_suite = NULL;
}

static void _cxml_t_clear_g_runner(){
    _cxml_t_init_g_runner();
}

static void _cxml_t_add_test(cxml_t_suite *suite, cts (*test_fn)(void)) {
    if (suite->count >= suite->capacity) {
        _CXML_T_GRACEFUL_EXIT("Too many tests in suite")
    }
    suite->tests[suite->count] = test_fn;
    suite->count++;
}

static void _cxml_t_add_mtest(cxml_t_suite *suite, int n, ...) {
    if (suite->count + n >= suite->capacity) {
        _CXML_T_GRACEFUL_EXIT("Too many tests in suite!")
    }
    va_list ap;
    va_start(ap, n);
    for (int i=0; i<n; i++){
        suite->tests[suite->count] = va_arg(ap, cts(*)(void));
        suite->count++;
    }
    va_end(ap);
}

static void _cxml_t_add_fixture(cxml_t_suite *suite, void (*fixture)(), int is_setup){
    if (!suite || !fixture) return;
    if (is_setup) suite->test_setup = fixture;
    else suite->test_teardown = fixture;
}

static void _cxml_t_do_runner_stat(cts stat) {
    switch (stat)
    {
        case CXML_T_PASS:
            _g_runner.passed++;
            break;
        case CXML_T_FAIL:
            _g_runner.failed++;
            break;
        case CXML_T_SKIP:
            _g_runner.skipped++;
            break;
        case CXML_T_TODO:
            _g_runner.todo++;
        default:
            break;
    }
}

static void _cxml_t_do_suite_stat(cxml_t_suite *suite, cts stat) {
    switch (stat)
    {
        case CXML_T_PASS:
            suite->passed++;
            break;
        case CXML_T_FAIL:
            suite->failed++;
            break;
        case CXML_T_SKIP:
            suite->skipped++;
            break;
        case CXML_T_TODO:
            suite->todo++;
        default:
            break;
    }
}

static void _cxml_t_suite_report(cxml_t_suite *suite) {
    // good news before bad news
    // passed, failed, to-do, skipped
    printf("\n==Suite %s Tests Summary==\n", suite->name);
    printf("Total tests run: %d\n", suite->count);
    printf(_GREEN "%d [PASSED]\n" _RESET, suite->passed);
    printf(_RED "%d [FAILED]\n" _RESET, suite->failed);
    printf(_YELLOW "%d [SKIPPED]\n" _RESET, suite->skipped);
    printf(_BLUE "%d [TODO]\n" _RESET, suite->todo);
    printf("%d passed, %d failed, %d skipped, %d todo. Finished in %f sec.\n\n",
           suite->passed, suite->failed, suite->skipped, suite->todo, suite->time_taken);
}

static void _cxml_t_g_runner_report() {
    printf("\n==All Tests Summary==\n");
    printf("Total Standalone Tests Run: %d\n", _g_runner.sta_tests_run);
    printf("Total Suites Run: %d\n", _g_runner.suites_run);
    printf("Total tests " _GREEN _BOLD  "[PASSED]:\t%d\n" _RESET, _g_runner.passed);
    printf("Total tests " _RED _BOLD "[FAILED]:\t%d\n" _RESET, _g_runner.failed);
    printf("Total tests " _YELLOW _BOLD "[SKIPPED]:\t%d\n" _RESET, _g_runner.skipped);
    printf("Total tests " _BLUE _BOLD "[TODO]:\t%d\n" _RESET, _g_runner.todo);
    printf("%d passed, %d failed, %d skipped, %d todo. Finished in %f sec.\n",
           _g_runner.passed, _g_runner.failed, _g_runner.skipped, _g_runner.todo, _g_runner.time_taken);
    if (_g_runner.passed && !_g_runner.failed && !_g_runner.todo){
        printf(_GREEN _BOLD "All tests passed in %f sec.\n\n" _RESET, _g_runner.time_taken);
    }else{
        printf("\n");
    }
    _cxml_t_clear_g_runner();
}

// run a standalone test - a test not added to any suite
static void _cxml_t_run_sta_test(
        cts (*test_fn)(void),
        int report)
{
    _g_runner.sta_tests_run++;
    clock_t _start, _end;
    _start = clock();
    cts stat = test_fn();
    _end = clock();
    _cxml_t_do_runner_stat(stat);
    double t_t = ((double) (_end - _start)) / CLOCKS_PER_SEC;
    if (report) printf("Test finished in %f sec.\n\n", t_t);
    else _g_runner.time_taken += t_t;
}

static void _cxml_t_shuffle(cts (*arr[])(void), int count){
    // using the Fisher and Yates' algorithm
    int j;
    cts (*tmp)();
    srand(time(NULL));
    // i -> n - 1, do:
    for (int i = (count - 1); i > 0; i--){
        // j -> 0 <= random <= i
        j = rand() % (i + 1);
        // swap
        tmp = arr[j];
        arr[j] = arr[i];
        arr[i] = tmp;
    }
}

inline static void _cxml_run__suite_tests(cxml_t_suite *suite){
    cts stat;
    clock_t start, end;
    double t_t;
    printf("\nRunning Suite %s::%s\n", suite->name, suite->file_name);
    for (int i = 0; i < suite->count; i++) {
        if (suite->test_setup) suite->test_setup();
        start = clock();
        stat = ((cts (*)(void)) suite->tests[i])();
        end = clock();
        if (suite->test_teardown) suite->test_teardown();
        _cxml_t_do_runner_stat(stat);
        _cxml_t_do_suite_stat(suite, stat);
        t_t = ((double) (end - start)) / CLOCKS_PER_SEC;
        suite->time_taken += t_t;
        _g_runner.time_taken += t_t;
    }
}

// run tests in a given suite
static void _cxml_t_run_suite_tests(cxml_t_suite *suite) {
#ifndef CXML_T_NO_SUITE_TEST_SHUFFLE
    _cxml_t_shuffle(suite->tests, suite->count);
#endif
    _cxml_run__suite_tests(suite);
    _g_runner.suites_run++;
}

// run all tests in a given suite shuffling the tests each time
static void _cxml_t_run_suite_tests_randomly(cxml_t_suite *suite) {
    _cxml_t_shuffle(suite->tests, suite->count);
    _cxml_run__suite_tests(suite);
    _g_runner.suites_run++;
}

static void _cxml_t_run__suites(int *status, int show_report, int n, ...){
    _cxml_t_init_g_runner();
    va_list ap;
    va_start(ap, n);
    for (int i=0; i<n; i++){
        ((void (*)(void))va_arg(ap, cts(*)(void)))();
    }
    va_end(ap);
    *status = _g_runner.failed;
    if (show_report) _cxml_t_g_runner_report();
    else _cxml_t_clear_g_runner();
}

// runs all suites without displaying the entire test summary report at the end
// i.e. runs all suites without displaying the global runner report
#define _cxml_t_run_suites_nr(s, n, ...)  \
_cxml_t_run__suites(s, 0, n, __VA_ARGS__)

// runs all suites, displaying the entire test summary report at the end
#define _cxml_t_run_suites(s, n, ...)  \
_cxml_t_run__suites(s, 1, n, __VA_ARGS__)

// function containing all tests -> runs them
#define cxml_suite(_suite_name)     \
    cxml_t_suite _Suite_##_suite_name;            \
    _cxml_t_init_suite(&_Suite_##_suite_name, #_suite_name, __FILE__);

// add test to suite -> adds test to suite's list of test functions
#define cxml_add_test(_test_fn)     \
    _cxml_t_add_test(_g_runner.current_suite, _test_fn);

// add multiple tests to suite -> adds test to suite's list of test functions
#define cxml_add_m_test(argc, ...)     \
    _cxml_t_add_mtest(_g_runner.current_suite, argc, __VA_ARGS__);

#define cxml_add_test_setup(_setup)    \
    _cxml_t_add_fixture(_g_runner.current_suite, _setup, 1);

#define cxml_add_test_teardown(_teardown)    \
    _cxml_t_add_fixture(_g_runner.current_suite, _teardown, 0);

// runs a specific STANDALONE test .
#define cxml_run_test(_test_name, _report)                 \
    _cxml_t_run_sta_test(_test_name, _report);

// runs a specific suite of tests - with optional shuffling
#define cxml_run_suite()    \
    _cxml_t_run_suite_tests(_g_runner.current_suite);

// runs a specific suite of tests  - shuffling the tests randomly.
#define cxml_run_suite_s()    \
    _cxml_t_run_suite_tests_randomly(_g_runner.current_suite);

#define cxml_suite_report()  \
    _cxml_t_suite_report(_g_runner.current_suite);

#endif //CXML_CXTEST_H
