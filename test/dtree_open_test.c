#include "dtree.h"
#include "test.h"

void test_pass_null(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open(NULL, &dt);
	fail_on_success(dt, err, "Successful when passing NULL");
	fail_on_false(dt, dtree_iserror(dt), "Error is not indicated by dtree_iserror()");
	test_end();
}

void test_nonexistent_dir(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open("/xxx/yyy/zzz", &dt);
	fail_on_success(dt, err, "Successful when passing non-existent dir: /xxx/yyy/zzz");
	fail_on_false(dt, dtree_iserror(dt), "Error is not indicated by dtree_iserror()");
	test_end();
}

void test_pass_file(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open(__FILE__, &dt);
	fail_on_success(dt, err, "Successful when passing " __FILE__ " as root dir");
	warn_on_true(dtree_iserror(dt), "Error is indicated by dtree_iserror(), but should not be");
	dtree_close(dt);
	test_end();
}

/**
 * Test on the real device-tree file system.
 * It should pass on target platform.
 */
void test_pass_mostly_valid(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open(DEVICE_TREE, &dt);
	fail_on_error(dt, err, "Open of " DEVICE_TREE " failed, is it present on the system?");
	warn_on_true(dtree_iserror(dt), "Error is indicated by dtree_iserror(), but should not be");
	dtree_close(dt);
	test_end();
}

/**
 * Gets information from the testing device-tree
 * in current directory.
 */
void test_open_test_dtree(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open("device-tree", &dt);
	fail_on_error(dt, err, "Can not open testing device-tree");
	warn_on_true(dtree_iserror(dt), "Error state is set, but should not be");
	dtree_close(dt);
	test_end();
}

/**
 * Tests whether the error flag is cleared in correct way.
 */
void test_clear_error(void)
{
	test_start();
	struct dtree_t *dt = NULL;
	int err = dtree_open(NULL, &dt);
	fail_on_success(dt, err, "Open of NULL was successful");
	fail_on_false(dt, dtree_iserror(dt), "No error is indicated");

	err = dtree_open("device-tree", &dt);
	fail_on_error(dt, err, "Can not open testing device-tree");
	fail_on_true(dt, dtree_iserror(dt), "The error was not cleared");

	dtree_close(dt);
	test_end();
}

int main(void)
{
	test_pass_null();
	test_nonexistent_dir();
	test_pass_file();
	test_open_test_dtree();
	test_pass_mostly_valid();
	test_clear_error();
}

