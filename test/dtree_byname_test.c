#include "dtree.h"
#include "dtree_procfs.h"
#include "test.h"

void test_list_all(void)
{
	test_start();
	struct dtree_dev_t *dev = NULL;

	while((dev = dtree_next()) != NULL) {
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
	
	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname("ethernet@81000000");
	fail_on_true(dev == NULL, "Could not find the device 'ethernet@81000000'");

	const char  *name = dtree_dev_name(dev);
	dtree_addr_t base = dtree_dev_base(dev);

	printf("DEV '%s' at 0x%08X\n", name, base);
	dtree_dev_free(dev);

	test_end();
}

void test_find_non_existent(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname("@not-implemented-device");
	fail_on_true(dev != NULL, "Device '@not-implemented-device' was found!");

	test_end();
}

void test_find_null(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname(NULL);
	fail_on_false(dev == NULL, "Device NULL was found!");

	test_end();
}

void test_find_empty(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname("");
	fail_on_false(dev == NULL, "Device '' was found!");

	test_end();
}

void test_find_with_discriminator(void)
{
	test_start();

	struct dtree_binding_t binds[] = {
		{ "instance", dev_parse_helper_string },
		{ "value", dev_parse_helper_integer },
		{ NULL, NULL }
	};

	dtree_procfs_set_bindings(binds);

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname("serial@84000000");
	fail_on_true(dev == NULL, "Could not find the device 'serial@84000000'");

	const char  *name = dtree_dev_name(dev);
	dtree_addr_t base = dtree_dev_base(dev);
	dtree_addr_t high = dtree_dev_high(dev);
	const char *prop1 = dtree_dev_get_string_property(dev, "instance");
	const uint32_t prop2 = dtree_dev_get_integer_property(dev, "value");

	fail_on_false(high - base == 0xFFFF, "Invalid high detected for serial@84000000)");

	printf("DEV '%s' at 0x%08X properties: instance '%s' value '%d'\n", name, base, prop1, prop2);
	dtree_dev_free(dev);
	dtree_procfs_unset_bindings();

	test_end();
}

void test_find_debug(void)
{
	test_start();

	struct dtree_dev_t *dev = NULL;
	dev = dtree_byname("debug@84400000");
	fail_on_true(dev == NULL, "Could not find the device 'debug@84400000'");

	const char  *name = dtree_dev_name(dev);
	dtree_addr_t base = dtree_dev_base(dev);
	dtree_addr_t high = dtree_dev_high(dev);

	printf("DEV '%s' at 0x%08X .. 0x%08X\n", name, base, high);
	dtree_dev_free(dev);

	fail_on_false(high - base == 0xFFFF, "Invalid high has been read for debug@84400000");
	test_end();
}

int main(void)
{
	int err = dtree_open("device-tree");
	halt_on_error(err, "Can not open testing device-tree");

	test_list_all();
	dtree_reset();

	test_find_existent();
	dtree_reset();

	test_find_non_existent();
	dtree_reset();

	test_find_null();
	dtree_reset();

	test_find_empty();
	dtree_reset();

	test_find_with_discriminator();
	dtree_reset();

	test_find_debug();
	dtree_reset();

	dtree_close();
}

