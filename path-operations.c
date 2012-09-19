#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define VERBOSE_SILENT  0
#define VERBOSE_NORMAL  1
#define VERBOSE_VERBOSE 2

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define RESTRICT restrict
#else
#define RESTRICT
#endif

static int s_verbose_level = VERBOSE_VERBOSE;

/*
 * normalised path matches language:
 *    '/'? ([^/]+ ('/' [^/]+)*)?
 */

void normalise_path(char* path)
{
   assert(path);
   const char* from = path;
   char *to = path;
   if (*from == '/') { *to++ = *from++; }
   while (*from == '/') { ++from; }
   while (*from) {
      while (*from && *from != '/') { *to++ = *from++; }
      while (*from == '/') { ++from; }
      if (*from) {
         assert(*from != '/');
         assert(from > to);
         *to++ = '/';
      }
   }
   assert(!*from);
   *to = '\0';
}

void dirname(char* RESTRICT buf, size_t bufsize, const char* RESTRICT path)
{
   assert(path);
   assert(buf);
   assert(bufsize >= 2);
   assert(bufsize > strlen(path));
   strcpy(buf, path);
   normalise_path(buf);

   char* end = strrchr(buf, '/');
   if (end) {
      if (end == buf) {
         buf[0] = '/';
         buf[1] = '\0';
      } else {
         *end = '\0';
      }
   } else {
      buf[0] = '.';
      buf[1] = '\0';
   }
}

void basename(char* RESTRICT buf, size_t bufsize, const char* RESTRICT path)
{
   assert(path);
   assert(buf);
   assert(bufsize >= 2);
   assert(bufsize > strlen(path));

   const char* begin = strrchr(path, '/');
   if (!begin) { begin = path; }
   if (*begin) {
      // skip trailing slashes
      if (begin[1] == '\0') {
         while (begin > path && *begin == '/') { --begin; }
         // if the entire path consists of trailing slashes, then it's the root directory
         // (so the basename is '/')
         if (*begin == '/') {
            assert(begin == path);
            *buf++ = '/';
            // null terminator added below
         } else {
            // otherwise, scan back to just before the basename
            while (begin > path && *begin != '/') { --begin; }
         }
      }
      if (*begin == '/') { ++begin; }
      // copy the basename (excluding any trailing slashes)
      while (*begin && *begin != '/') { *buf++ = *begin++; }
   } else {
      *buf++ = '.';
   }
   *buf = '\0';
}

int test_check(const char* input, const char* expected, const char* output)
{
   int success = (0 == strcmp(output, expected));
   if (success) {
      if (s_verbose_level >= VERBOSE_VERBOSE) {
         printf("GOOD '%s' -> '%s'\n", input, output);
      }
   } else {
      if (s_verbose_level > VERBOSE_SILENT) {
         printf(" BAD '%s' -> '%s' (expected '%s')\n", input, output, expected);
      }
   }
   return success;
}

int test_inplace(void (*f)(char*), const char* input, const char* expected)
{
   char buf[64];
   assert(strlen(input) < sizeof(buf));
   assert(strlen(expected) < sizeof(buf));
   strcpy(buf, input);
   f(buf);
   return test_check(input, expected, buf);
}

int test_outplace(void (*f)(char* RESTRICT, size_t, const char* RESTRICT),
      const char* input, const char* expected)
{
   char buf[64];
   assert(strlen(input) < sizeof(buf));
   assert(strlen(expected) < sizeof(buf));
   f(buf, sizeof(buf), input);
   return test_check(input, expected, buf);
}

typedef struct {
   const char* input;
   const char* expected;
} TestCase;

static const TestCase NORMPATH_TEST_CASES[] = {
   { "", "" },
   { "/", "/" },
   { "aaa", "aaa" },
   { "/aaa", "/aaa" },
   { "aaa/", "aaa" },
   { "/aaa/", "/aaa" },
   { "/aaa/bbb", "/aaa/bbb" },
   { "/aaa/bbb/", "/aaa/bbb" },
   { "aaa/bbb", "aaa/bbb" },
   { "aaa/bbb/", "aaa/bbb" },
   { "aaa/bbb/", "aaa/bbb" },
   { "//", "/" },
   { "//aaa", "/aaa" },
   { "//aaa/", "/aaa" },
   { "//aaa/bbb", "/aaa/bbb" },
   { "//aaa/bbb/", "/aaa/bbb" },
   { "/", "/" },
   { "/aaa//", "/aaa" },
   { "/aaa//bbb", "/aaa/bbb" },
   { "/aaa//bbb/", "/aaa/bbb" },
   { "/aaa/bbb//", "/aaa/bbb" },
   { NULL, NULL }
};

static const TestCase DIRNAME_TEST_CASES[] = {
   { "", "." },
   { "/", "/" },
   { "aaa", "." },
   { "/aaa", "/" },
   { "aaa/bbb", "aaa" },
   { "aaa/bbb/ccc/ddd", "aaa/bbb/ccc" },
   { "/aaa/bbb", "/aaa" },
   { "/aaa/bbb/ccc/ddd", "/aaa/bbb/ccc" },
   { "//", "/" },
   { "aaa//", "." },
   { "///aaa//", "/" },
   { "aaa///bbb//", "aaa" },
   { "aaa//bbb///ccc///ddd//", "aaa/bbb/ccc" },
   { "///aaa//bbb//", "/aaa" },
   { "/aaa//bbb/ccc///ddd//", "/aaa/bbb/ccc" },
   { NULL, NULL }
};

static const TestCase BASENAME_TEST_CASES[] = {
   { "", "." },
   { "/", "/" },
   { "aaa", "aaa" },
   { "/aaa", "aaa" },
   { "aaa/bbb", "bbb" },
   { "aaa/bbb/ccc/ddd", "ddd" },
   { "/aaa/bbb", "bbb" },
   { "/aaa/bbb/ccc/ddd", "ddd" },
   { "//", "/" },
   { "aaa//", "aaa" },
   { "///aaa", "aaa" },
   { "///aaa//", "aaa" },
   { "aaa///bbb//", "bbb" },
   { "aaa//bbb///ccc///ddd//", "ddd" },
   { "///aaa//bbb//", "bbb" },
   { "/aaa//bbb/ccc///ddd//", "ddd" },
   { NULL, NULL }
};

void run_tests_inplace(int* count, int* good_count, const char* title, const TestCase* cases,
      void (*f)(char*))
{
   if (s_verbose_level > VERBOSE_SILENT) {
      printf("%s:\n", title);
   }
   for (int i = 0; cases[i].input; ++i) {
      ++*count;
      if (test_inplace(f, cases[i].input, cases[i].expected))
         ++*good_count;
   }
}

void run_tests_outplace(int* count, int* good_count, const char* title, const TestCase* cases,
      void (*f)(char* RESTRICT, size_t, const char* RESTRICT))
{
   if (s_verbose_level > VERBOSE_SILENT) {
      printf("%s:\n", title);
   }
   for (int i = 0; cases[i].input; ++i) {
      ++*count;
      if (test_outplace(f, cases[i].input, cases[i].expected))
         ++*good_count;
   }
}

int main(int argc, char** argv)
{
   int count = 0, good_count = 0;

   run_tests_inplace(&count, &good_count, "normalise_path", NORMPATH_TEST_CASES, &normalise_path);
   run_tests_outplace(&count, &good_count, "dirname", DIRNAME_TEST_CASES, &dirname);
   run_tests_outplace(&count, &good_count, "basename", BASENAME_TEST_CASES, &basename);

   if (s_verbose_level > VERBOSE_SILENT) {
      printf("%d / %d passed%s\n", good_count, count, (good_count == count) ? " (ALL OK)" : "");
   }
   return (good_count < count) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/* vim: set sts=3 sw=3 et: */
