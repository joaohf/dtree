#include "dtree.h"
#include "test.h"

static struct dtree_t *dt = NULL;

void test_all_dev(const int expect)
{
	test_start();

	struct dtree_dev_t *curr = NULL;
	int count = 0;

	while((curr = dtree_next(dt))) {
		const char *name  = dtree_dev_name(curr);
		dtree_addr_t base = dtree_dev_base(curr);
		dtree_addr_t high = dtree_dev_high(curr);

		printf("DEV '%s' at 0x%08X .. 0x%08X\n", name, base, high);
		print_compat(curr);

		dtree_dev_free(curr);
		count += 1;
	}

	fail_on_true(dt, dtree_iserror(dt), "An error occured during traversing the device tree");
	fail_on_true(dt, count < expect, "Some device were not traversed");
	fail_on_true(dt, count > expect, "More devices were traversed then expected");

	test_end();
}

int main(void)
{
	const int expect = 8; // magic number, see device-tree/ in current dir, number of name@addr dirs
	int err = dtree_open("device-tree", &dt);
	halt_on_error(dt, err, "Can not open testing device-tree");

	test_all_dev(expect);

	dtree_close(dt);
}

