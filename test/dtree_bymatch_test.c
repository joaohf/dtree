#include "dtree.h"
#include "test.h"

struct dtree_t *dt = NULL;

void test_list_all(void)
{
	test_start();
	struct dtree_dev_t *dev = NULL;

	while((dev = dtree_next(dt)) != NULL) {
		const char  *name = dtree_dev_name(dev);
		dtree_addr_t base = dtree_dev_base(dev);

		printf("DEV '%s' at 0x%08X\n", name, base);
		print_compat(dev);
		dtree_dev_free(dev);
	}

	test_end();
}

void test_find_existent(void)
{
	test_start();
	
	struct dtree_t *pdt = NULL;
	struct dtree_t *curr_dt = NULL;
	struct dtree_dev_t *dev = NULL;

	pdt = dtree_bymatch(dt, "i2c");
	fail_on_true(pdt, pdt == NULL, "Could not find the device 'ethernet'");

	while ( (curr_dt = dtree_next_dev_match(pdt)) != NULL) {
	    while( (dev = dtree_next(curr_dt)) ) {
			const char  *name = dtree_dev_name(dev);
			dtree_addr_t base = dtree_dev_base(dev);
			int value_error;
			const uint32_t prop1 = dtree_dev_get_integer_property(dev, "isnode", &value_error);
			printf("DEV '%s' at 0x%08X isnode: %s\n", name, base, prop1 ? "TRUE" : "FALSE");
			dtree_dev_free(dev);
	    }

	    dtree_reset(curr_dt);
	    dtree_next_dev_free(curr_dt);
	}

	dtree_reset(pdt);
	dtree_next_dev_free(pdt);

	test_end();
}

void test_find_non_existent(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(dt, "@not-implemented-device");
	fail_on_true(dt, dev != NULL, "Device '@not-implemented-device' was found!");

	test_end();
}

void test_find_null(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(dt, NULL);
	fail_on_false(dt, dev == NULL, "Device NULL was found!");

	test_end();
}

void test_find_empty(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(dt, "");
	fail_on_false(dt, dev == NULL, "Device '' was found!");

	test_end();
}

void test_find_with_discriminator(void)
{
	test_start();

	struct dtree_binding_t binds[] = {
		{ "instance", dev_parse_helper_string },
		{ "value", dev_parse_helper_integer },
		{ "gpios", dev_parse_helper_int_data },
		{ NULL, NULL }
	};

	dtree_procfs_set_bindings(binds);

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(dt, "serial@84000000");
	fail_on_true(dt, dev == NULL, "Could not find the device 'serial@84000000'");

	const char  *name = dtree_dev_name(dev);
	dtree_addr_t base = dtree_dev_base(dev);
	dtree_addr_t high = dtree_dev_high(dev);
	const char *prop1 = dtree_dev_get_string_property(dev, "instance");
	int value_error;
	const uint32_t prop2 = dtree_dev_get_integer_property(dev, "value", &value_error);
	struct prop3 {
	        uint32_t n;
		uint32_t dir;
	} prop3;
	dtree_dev_get_int_data_property(dev, "gpios", (uintptr_t *) &prop3, sizeof(struct prop3));


	fail_on_false(dt, high - base == 0xFFFF, "Invalid high detected for serial@84000000)");

	printf("DEV '%s' at 0x%08X properties: instance '%s' value '%d' n '%d' dir '%d'\n",
	 name, base, prop1, prop2, prop3.n, prop3.dir);
	dtree_dev_free(dev);
	dtree_procfs_unset_bindings();

	test_end();
}

void test_find_debug(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(dt, "debug@84400000");
	fail_on_true(dt, dev == NULL, "Could not find the device 'debug@84400000'");

	const char  *name = dtree_dev_name(dev);
	dtree_addr_t base = dtree_dev_base(dev);
	dtree_addr_t high = dtree_dev_high(dev);

	printf("DEV '%s' at 0x%08X .. 0x%08X\n", name, base, high);
	dtree_dev_free(dev);

	fail_on_false(dt, high - base == 0xFFFF, "Invalid high has been read for debug@84400000");
	test_end();
}

int main(void)
{
	int err = dtree_open("device-tree", &dt);
	halt_on_error(dt, err, "Can not open testing device-tree");

	test_list_all();
	dtree_reset(dt);

	test_find_existent();
	dtree_reset(dt);

	test_find_non_existent();
	dtree_reset(dt);

	test_find_null();
	dtree_reset(dt);

	test_find_empty();
	dtree_reset(dt);

	test_find_with_discriminator();
	dtree_reset(dt);

	test_find_debug();
	dtree_reset(dt);

	dtree_close(dt);
}

