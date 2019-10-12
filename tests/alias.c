#include <stdint.h>
#include <setjmp.h>
#include <stdio.h>
#include <cmocka.h>
#include "util.h"
#include "alias.h"
#include "test-log.h"

#include "globals.c"
#include "../libmultipath/alias.c"

#if INT_MAX == 0x7fffffff
/* user_friendly_name for map #INT_MAX */
#define MPATH_ID_INT_MAX "fxshrxw"
/* ... and one less */
#define MPATH_ID_INT_MAX_m1 "fxshrxv"
/* ... and one more */
#define MPATH_ID_INT_MAX_p1 "fxshrxx"
#endif

void __wrap_rewind(FILE *stream)
{}

char *__wrap_fgets(char *buf, int n, FILE *stream)
{
	char *val = mock_ptr_type(char *);
	if (!val)
		return NULL;
	strlcpy(buf, val, n);
	return buf;
}

static void fd_mpatha(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 1, sizeof(buf), "FOO");
	assert_int_equal(rc, 4);
	assert_string_equal(buf, "FOOa");
}

static void fd_mpathz(void **state)
{
	/* This also tests a "short" buffer, see fd_mpath_short1 */
	char buf[5];
	int rc;

	rc = format_devname(buf, 26, sizeof(buf), "FOO");
	assert_int_equal(rc, 4);
	assert_string_equal(buf, "FOOz");
}

static void fd_mpathaa(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26 + 1, sizeof(buf), "FOO");
	assert_int_equal(rc, 5);
	assert_string_equal(buf, "FOOaa");
}

static void fd_mpathzz(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26*26 + 26, sizeof(buf), "FOO");
	assert_int_equal(rc, 5);
	assert_string_equal(buf, "FOOzz");
}

static void fd_mpathaaa(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26*26 + 27, sizeof(buf), "FOO");
	assert_int_equal(rc, 6);
	assert_string_equal(buf, "FOOaaa");
}

static void fd_mpathzzz(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26*26*26 + 26*26 + 26, sizeof(buf), "FOO");
	assert_int_equal(rc, 6);
	assert_string_equal(buf, "FOOzzz");
}

static void fd_mpathaaaa(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26*26*26 + 26*26 + 27, sizeof(buf), "FOO");
	assert_int_equal(rc, 7);
	assert_string_equal(buf, "FOOaaaa");
}

static void fd_mpathzzzz(void **state)
{
	char buf[32];
	int rc;

	rc = format_devname(buf, 26*26*26*26 + 26*26*26 + 26*26 + 26,
			    sizeof(buf), "FOO");
	assert_int_equal(rc, 7);
	assert_string_equal(buf, "FOOzzzz");
}

#ifdef MPATH_ID_INT_MAX
static void fd_mpath_max(void **state)
{
	char buf[32];
	int rc;

	rc  = format_devname(buf, INT_MAX, sizeof(buf), "");
	assert_int_equal(rc, strlen(MPATH_ID_INT_MAX));
	assert_string_equal(buf, MPATH_ID_INT_MAX);
}
#endif

static void fd_mpath_max1(void **state)
{
	char buf[32];
	int rc;

	rc  = format_devname(buf, INT_MIN, sizeof(buf), "");
	assert_int_equal(rc, -1);
}

static void fd_mpath_short(void **state)
{
	char buf[4];
	int rc;

	rc = format_devname(buf, 1, sizeof(buf), "FOO");
	assert_int_equal(rc, -1);
}

static void fd_mpath_short1(void **state)
{
	char buf[5];
	int rc;

	rc = format_devname(buf, 27, sizeof(buf), "FOO");
	assert_int_equal(rc, -1);
}

static int test_format_devname(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(fd_mpatha),
		cmocka_unit_test(fd_mpathz),
		cmocka_unit_test(fd_mpathaa),
		cmocka_unit_test(fd_mpathzz),
		cmocka_unit_test(fd_mpathaaa),
		cmocka_unit_test(fd_mpathzzz),
		cmocka_unit_test(fd_mpathaaaa),
		cmocka_unit_test(fd_mpathzzzz),
#ifdef MPATH_ID_INT_MAX
		cmocka_unit_test(fd_mpath_max),
#endif
		cmocka_unit_test(fd_mpath_max1),
		cmocka_unit_test(fd_mpath_short),
		cmocka_unit_test(fd_mpath_short1),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

static void sd_mpatha(void **state)
{
	int rc = scan_devname("MPATHa", "MPATH");

	assert_int_equal(rc, 1);
}

/*
 * Text after whitespace is ignored. But an overlong input
 * errors out, even if it's just whitespace.
 * It's kind of strange that scan_devname() treats whitespace
 * like this. But I'm not sure if some corner case depends
 * on this behavior.
 */
static void sd_mpatha_spc(void **state)
{
	int rc = scan_devname("MPATHa  00", "MPATH");

	assert_int_equal(rc, 1);
}

static void sd_mpatha_tab(void **state)
{
	int rc = scan_devname("MPATHa\t00", "MPATH");

	assert_int_equal(rc, 1);
}

static void sd_overlong(void **state)
{
	int rc = scan_devname("MPATHa       ", "MPATH");

	assert_int_equal(rc, -1);
}

static void sd_overlong1(void **state)
{
	int rc = scan_devname("MPATHabcdefgh", "MPATH");

	assert_int_equal(rc, -1);
}

static void sd_noprefix(void **state)
{
	int rc = scan_devname("MPATHa", NULL);

	assert_int_equal(rc, -1);
}

static void sd_nomatchprefix(void **state)
{
	int rc = scan_devname("MPATHa", "mpath");

	assert_int_equal(rc, -1);
}

static void sd_eq_prefix(void **state)
{
	int rc = scan_devname("MPATH", "MPATH");

	assert_int_equal(rc, -1);
}

static void sd_bad_1(void **state)
{
	int rc = scan_devname("MPATH0", "MPATH");

	assert_int_equal(rc, -1);
}

static void sd_bad_2(void **state)
{
	int rc = scan_devname("MPATHa0c", "MPATH");

	assert_int_equal(rc, -1);
}

#ifdef MPATH_ID_INT_MAX
static void sd_max(void **state)
{
	int rc = scan_devname("MPATH" MPATH_ID_INT_MAX, "MPATH");

	assert_int_equal(rc, INT_MAX);
}

static void sd_max_p1(void **state)
{
	int rc = scan_devname("MPATH" MPATH_ID_INT_MAX_p1, "MPATH");

	assert_int_equal(rc, -1);
}
#endif

static int test_scan_devname(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(sd_mpatha),
		cmocka_unit_test(sd_mpatha_spc),
		cmocka_unit_test(sd_mpatha_tab),
		cmocka_unit_test(sd_overlong),
		cmocka_unit_test(sd_overlong1),
		cmocka_unit_test(sd_noprefix),
		cmocka_unit_test(sd_nomatchprefix),
		cmocka_unit_test(sd_eq_prefix),
		cmocka_unit_test(sd_bad_1),
		cmocka_unit_test(sd_bad_2),
#ifdef MPATH_ID_INT_MAX
		cmocka_unit_test(sd_max),
		cmocka_unit_test(sd_max_p1),
#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

static void lb_empty(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID0] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID0", &alias, NULL);
	assert_int_equal(rc, 1);
	assert_ptr_equal(alias, NULL);
}

static void lb_match_a(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	expect_condlog(3, "Found matching wwid [WWID0] in bindings file."
		       " Setting alias to MPATHa\n");
	rc = lookup_binding(NULL, "WWID0", &alias, "MPATH");
	assert_int_equal(rc, 0);
	assert_ptr_not_equal(alias, NULL);
	assert_string_equal(alias, "MPATHa");
	free(alias);
}

static void lb_nomatch_a(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID1] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID1", &alias, "MPATH");
	assert_int_equal(rc, 2);
	assert_ptr_equal(alias, NULL);
}

static void lb_match_c(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, "MPATHc WWID1\n");
	expect_condlog(3, "Found matching wwid [WWID1] in bindings file."
		       " Setting alias to MPATHc\n");
	rc = lookup_binding(NULL, "WWID1", &alias, "MPATH");
	assert_int_equal(rc, 0);
	assert_ptr_not_equal(alias, NULL);
	assert_string_equal(alias, "MPATHc");
	free(alias);
}

static void lb_nomatch_a_c(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, "MPATHc WWID1\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, 2);
	assert_ptr_equal(alias, NULL);
}

static void lb_nomatch_c_a(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHc WWID1\n");
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, 2);
	assert_ptr_equal(alias, NULL);
}

static void lb_nomatch_a_b(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, "MPATHz WWID26\n");
	will_return(__wrap_fgets, "MPATHb WWID1\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, 3);
	assert_ptr_equal(alias, NULL);
}

static void lb_nomatch_a_b_bad(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, "MPATHz WWID26\n");
	will_return(__wrap_fgets, "MPATHb\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "Ignoring malformed line 3 in bindings file\n");
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, 3);
	assert_ptr_equal(alias, NULL);
}

static void lb_nomatch_b_a(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHb WWID1\n");
	will_return(__wrap_fgets, "MPATHz WWID26\n");
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, 27);
	assert_ptr_equal(alias, NULL);
}

#ifdef MPATH_ID_INT_MAX
static void lb_nomatch_int_max(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHb WWID1\n");
	will_return(__wrap_fgets, "MPATH" MPATH_ID_INT_MAX " WWIDMAX\n");
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(0, "no more available user_friendly_names\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, -1);
	assert_ptr_equal(alias, NULL);
}

static void lb_nomatch_int_max_m1(void **state)
{
	int rc;
	char *alias;

	will_return(__wrap_fgets, "MPATHb WWID1\n");
	will_return(__wrap_fgets, "MPATH" MPATH_ID_INT_MAX_m1 " WWIDMAX\n");
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching wwid [WWID2] in bindings file.\n");
	rc = lookup_binding(NULL, "WWID2", &alias, "MPATH");
	assert_int_equal(rc, INT_MAX);
	assert_ptr_equal(alias, NULL);
}
#endif

static int test_lookup_binding(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(lb_empty),
		cmocka_unit_test(lb_match_a),
		cmocka_unit_test(lb_nomatch_a),
		cmocka_unit_test(lb_match_c),
		cmocka_unit_test(lb_nomatch_a_c),
		cmocka_unit_test(lb_nomatch_c_a),
		cmocka_unit_test(lb_nomatch_a_b),
		cmocka_unit_test(lb_nomatch_a_b_bad),
		cmocka_unit_test(lb_nomatch_b_a),
#ifdef MPATH_ID_INT_MAX
		cmocka_unit_test(lb_nomatch_int_max),
		cmocka_unit_test(lb_nomatch_int_max_m1),
#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

static void rl_empty(void **state)
{
	int rc;
	char buf[WWID_SIZE];

	buf[0] = '\0';
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching alias [MPATHa] in bindings file.\n");
	rc = rlookup_binding(NULL, buf, "MPATHa");
	assert_int_equal(rc, -1);
	assert_string_equal(buf, "");
}

static void rl_match_a(void **state)
{
	int rc;
	char buf[WWID_SIZE];

	buf[0] = '\0';
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	expect_condlog(3, "Found matching alias [MPATHa] in bindings file.\n"
		       "Setting wwid to WWID0\n");
	rc = rlookup_binding(NULL, buf, "MPATHa");
	assert_int_equal(rc, 0);
	assert_string_equal(buf, "WWID0");
}

static void rl_nomatch_a(void **state)
{
	int rc;
	char buf[WWID_SIZE];

	buf[0] = '\0';
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "No matching alias [MPATHb] in bindings file.\n");
	rc = rlookup_binding(NULL, buf, "MPATHb");
	assert_int_equal(rc, -1);
	assert_string_equal(buf, "");
}

static void rl_malformed_a(void **state)
{
	int rc;
	char buf[WWID_SIZE];

	buf[0] = '\0';
	will_return(__wrap_fgets, "MPATHa     \n");
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "Ignoring malformed line 1 in bindings file\n");
	expect_condlog(3, "No matching alias [MPATHa] in bindings file.\n");
	rc = rlookup_binding(NULL, buf, "MPATHa");
	assert_int_equal(rc, -1);
	assert_string_equal(buf, "");
}

static void rl_overlong_a(void **state)
{
	int rc;
	char buf[WWID_SIZE];
	char line[WWID_SIZE + 10];

	snprintf(line, sizeof(line), "MPATHa ");
	memset(line + strlen(line), 'W', sizeof(line) - 2 - strlen(line));
	snprintf(line + sizeof(line) - 2, 2, "\n");

	buf[0] = '\0';
	will_return(__wrap_fgets, line);
	will_return(__wrap_fgets, NULL);
	expect_condlog(3, "Ignoring too large wwid at 1 in bindings file\n");
	expect_condlog(3, "No matching alias [MPATHa] in bindings file.\n");
	rc = rlookup_binding(NULL, buf, "MPATHa");
	assert_int_equal(rc, -1);
	assert_string_equal(buf, "");
}

static void rl_match_b(void **state)
{
	int rc;
	char buf[WWID_SIZE];

	buf[0] = '\0';
	will_return(__wrap_fgets, "MPATHa WWID0\n");
	will_return(__wrap_fgets, "MPATHz WWID26\n");
	will_return(__wrap_fgets, "MPATHb WWID2\n");
	expect_condlog(3, "Found matching alias [MPATHb] in bindings file.\n"
		       "Setting wwid to WWID2\n");
	rc = rlookup_binding(NULL, buf, "MPATHb");
	assert_int_equal(rc, 0);
	assert_string_equal(buf, "WWID2");
}

static int test_rlookup_binding(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(rl_empty),
		cmocka_unit_test(rl_match_a),
		cmocka_unit_test(rl_nomatch_a),
		cmocka_unit_test(rl_malformed_a),
		cmocka_unit_test(rl_overlong_a),
		cmocka_unit_test(rl_match_b),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}

int main(void)
{
	int ret = 0;

	ret += test_format_devname();
	ret += test_scan_devname();
	ret += test_lookup_binding();
	ret += test_rlookup_binding();

	return ret;
}