#include "stcunit.h"

TEST_UNIT(pass) {
    TEST_RETURN
}

TEST_UNIT(fail) {
    TEST_THROW_FAIL("failed");
    TEST_RETURN
}

TEST_UNIT(error) {
    TEST_THROW_ERROR("error");
    TEST_RETURN
}

TEST_UNIT(skip) {
    TEST_THROW_SKIP("skip");
    TEST_RETURN
}

TEST_UNIT(assert) {
    TEST_ASSERT(0 == 1, "expected 0, but got not-zero!");
    TEST_RETURN
}

TEST_UNIT(assert_with_formatting) {
    const int value = 1;
    const int expected = 0;
    TEST_ASSERT(value == expected, "expected %d, but got %d!", expected, value);
    TEST_RETURN
}

TEST_UNIT(checkpoint) {
    TEST_ASSERT(0.0 == 0.1, "should be zero");
    TEST_CHECKPOINT("any failures here?");
    TEST_ASSERT(0 == 0, "expected zero");
    TEST_ASSERT(0 == 1, "expected zero\n");

    TEST_ASSERT(1 == 1, "expected one");
    if (TEST_CHECKPOINT_FAILED()) {
        fprintf(stderr, "only assertions after the checkpoint are reported by TEST_CHECKPOINT_FAILED()\n");
        TEST_THROW_FAIL("%d fail(s) detected so far. should not continue.", TEST_CHECKPOINT_FAILED());
    }
    // Below this line will not execute
    TEST_CHECKPOINT("how about here?");
    TEST_ASSERT(2 == 2, "expected two");
    TEST_RETURN
}

TEST_UNIT(redirect_save) {
    printf("test string\n");
    const char *filename = "save.txt";
    TEST_REDIRECT_SAVE(&TEST_LOCAL_REDIRECT, filename);
    struct stat st;
    TEST_ASSERT(stat(filename, &st) == 0, "%s should exist", filename);
    TEST_ASSERT(st.st_size != 0, "%s should not be empty");
    remove(filename);
    TEST_RETURN
}

TEST_UNIT(is_ansi_and_empty) {
    const char *ansi_only = TEST_TERM_COLOR_RESET TEST_TERM_COLOR_WHITE "" TEST_TERM_COLOR_RESET;
    const char *ansi_hello = TEST_TERM_COLOR_RESET TEST_TERM_COLOR_WHITE "Hello" TEST_TERM_COLOR_RESET;
    TEST_ASSERT(is_ansi_and_empty(ansi_only) == true, "should be ansi and empty");
    TEST_ASSERT(is_ansi_and_empty(ansi_hello) == false, "should be ansi and not empty");
    TEST_RETURN
}

TEST_UNIT(strlen_sans_ansi_codes) {
    TEST_MARK("standard");
    const char *ansi_only = TEST_TERM_COLOR_RESET TEST_TERM_COLOR_WHITE "" TEST_TERM_COLOR_RESET;
    const char *ansi_hello = TEST_TERM_COLOR_RESET TEST_TERM_COLOR_WHITE "Hello" TEST_TERM_COLOR_RESET;
    TEST_ASSERT(strlen_sans_ansi_codes(ansi_only) == 0, "should be zero");
    TEST_ASSERT(strlen_sans_ansi_codes(ansi_hello) == 5, "should not be zero");
    TEST_MARK("extended");
    const char *ansi_extended = TEST_TERM_COLOR_RESET "\x1b[38;5;230m" "A little yellow." TEST_TERM_COLOR_RESET;
    TEST_ASSERT(strlen_sans_ansi_codes(ansi_extended) == 16, "should not be zero");
    TEST_RETURN
}

TEST_UNIT(file_as_array) {
    const char *filename = "file_as_array.txt";
    char *words = strdup("The quick brown fox jumps over the lazy dog.");
    char *words_orig = words;
    if (!words) {
        TEST_THROW_ERROR("unable to allocate memory");
    }

    FILE *fp = fopen(filename, "w+");
    if (!fp) {
        TEST_THROW_ERROR("unable to open %s for writing", filename);
    }

    size_t dash_count = 0;
    const size_t step = 4;
    const char *token = NULL;
    for (size_t i = 0; (token = strsep(&words, " ")); i += step) {
        for (size_t n = 0; n < i; n++) {
            fputc('-', fp);
            dash_count++;
        }
        fprintf(fp, "%s\n", token);
    }
    free(words_orig);
    fclose(fp);

    size_t lines_count = 0;
    char **lines = TEST_FILE_AS_ARRAY(filename, &lines_count);
    if (!lines) {
        TEST_THROW_ERROR("unable to read back %s", filename);
    }
    for (size_t i = 0; i != lines_count; i++) {
        printf("line %zu: %s\n", i, lines[i]);
    }
    TEST_ASSERT(dash_count % step == 0, "number of dashes (%zu) not divisible by step (%zu)", dash_count, step);
    TEST_ASSERT(lines_count == 9, "panagram line count should be 9, got %zu", lines_count);
    TEST_ARRAY_FREE(lines, lines_count);
    remove(filename);
    TEST_RETURN
}

TEST_UNIT(file_contains) {
    const char *filename = "file_contains.txt";
    FILE *fp = fopen(filename, "w+");
    if (!fp) {
        TEST_THROW_ERROR("unable to open %s for writing", filename);
    }
    fprintf(fp, "0,1,2,3,4,5,6,7,8,9\n");
    fprintf(fp, "10,11,12,13,14,15,16,17,18,19\n");
    fprintf(fp, "20,21,22,23,24,25,26,27,28,29\n");
    fclose(fp);

    char *match = NULL;
    size_t match_lineno = 0;
    if (TEST_FILE_CONTAINS(filename, "15", -1, -1, &match, &match_lineno) < 0) {
        free(match);
        TEST_THROW_ERROR("unable to read back %s", filename);
    }
    TEST_ASSERT(match != NULL, "match should not be NULL");
    TEST_ASSERT(match_lineno == 1, "expected line 1 but got %zu", match_lineno);
    free(match);
    match = NULL;

    if (TEST_FILE_CONTAINS(filename, "25", 2, -1, &match, &match_lineno) < 0) {
        free(match);
        TEST_THROW_ERROR("unable to read back %s", filename);
    }
    TEST_ASSERT(match != NULL, "match should not be NULL");
    TEST_ASSERT(match_lineno == 2, "expected line 2 but got %zu", match_lineno);
    const char *value = strstr(match, "25");
    TEST_ASSERT(value != NULL, "value should not be NULL");
    TEST_ASSERT(strlen(value) >= 3, "value should be at least 3 characters");
    TEST_ASSERT(*(value + 2) == ',', "expected comma to follow value");
    free(match);
    match = NULL;


    TEST_RETURN
}

TEST_UNIT(get_format_length) {
    const struct {
        const char *format_str;
        const char *expected_end;
        size_t expected_len;

    } testcase[] = {
        {.format_str = "this is not a formatted string", .expected_end = "this is not a formatted string", .expected_len = 0},
        {.format_str = "this is % corrected", .expected_end = " corrected", .expected_len = 1},
        {.format_str = "this is %lluok", .expected_end = "ok", .expected_len = 4},
        {.format_str = "this is %zuok", .expected_end = "ok", .expected_len = 3},
        {.format_str = "this is %sok", .expected_end = "ok", .expected_len = 2},
        {.format_str = "this is %100sok", .expected_end = "ok", .expected_len = 5},
        {.format_str = "this is %10.2dok", .expected_end = "ok", .expected_len = 6},
        {.format_str = "this is %#02xok", .expected_end = "ok", .expected_len = 5},
        {.format_str = "this is %-020iok", .expected_end = "ok", .expected_len = 6},
    };

    for (size_t i = 0; i < sizeof(testcase) / sizeof(testcase[0]); i++) {
        TEST_CHECKPOINT("CASE %zu", i);
        printf("format = %s\n", testcase[i].format_str);
        const char *start = strpbrk(testcase[i].format_str, "%");
        if (!start) {
            start = testcase[i].format_str;
        }
        const size_t format_len = get_format_length(start);
        printf("format_len = %zu\n", format_len);
        const char *format_end = start + format_len;
        printf("format_end = %s\n", format_end);

        TEST_ASSERT(format_len == testcase[i].expected_len, "expected %zu, but got %zu", testcase[i].expected_len, format_len);
        TEST_ASSERT(strcmp(format_end, testcase[i].expected_end) == 0, "expected '%s', but got '%s'", testcase[i].expected_end, format_end);
        const int problems = TEST_CHECKPOINT_FAILED();
        if (problems) {
            printf("CASE %zu failed with %d problem(s)\n", i, problems);
        }
    }
    TEST_RETURN
}

TEST_SUITE("Showcase") {
    const unit_test tests[] = {
        // Result management
        TEST_UNIT_REPR(pass),
        TEST_UNIT_REPR(fail),
        TEST_UNIT_REPR(error),
        TEST_UNIT_REPR(skip),
        // State management
        TEST_UNIT_REPR(checkpoint),
        // Assertions
        TEST_UNIT_REPR(assert),
        TEST_UNIT_REPR(assert_with_formatting),
        // Output redirection
        TEST_UNIT_REPR(redirect_save),
        // File management
        TEST_UNIT_REPR(file_contains),
        TEST_UNIT_REPR(file_as_array),
        // Helpers
        TEST_UNIT_REPR(is_ansi_and_empty),
        TEST_UNIT_REPR(strlen_sans_ansi_codes),
        TEST_UNIT_REPR(get_format_length),
    };
    TEST_SUITE_RUN(tests)
    TEST_SUITE_RETURN
}
