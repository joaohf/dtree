/**
 * Access to device-tree in embedded Linux.
 * Public API.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 *
 * The main principle if to be able to iterate
 * over all devices or search among them.
 *
 * The library is not reentrant nor thread safe.
 */

#ifndef DTREE_H
#define DTREE_H

#include "dtree_properties.h"

#include <stdint.h>
#include <stdio.h>

//
// Module initialization & destruction
//

/**
 * Opens device tree using the given root directory
 * (typically /proc/device-tree). Setups internal
 * structures. Clears error state.
 *
 * It is safe to call dtree_reset() after dtree_open()
 * but it has no effect.
 * It is an error to call dtree_open() twice (without
 * dtree_close() in between).
 *
 * Returns 0 on success. On error sets error state.
 */
int dtree_open(const char *rootd);

/**
 * Free's resources of the module.
 * It is an error to call it when dtree_open()
 * has failed or to call it twice.
 */
void dtree_close(void);


//
// Data types
//

/**
 * Representation of address.
 */
typedef uint32_t dtree_addr_t;

struct dtree_hash_properties;

/**
 * Device info representation.
 *
 * Consists of the name, address (base and highest address)
 * and array of compatible devices. Last pointer in compat is NULL.
 */
struct dtree_dev_t {
	const char  *name;
	dtree_addr_t base;
	dtree_addr_t high;
	const char  **compat;
	struct dtree_hash_properties *properties;
};

struct dtree_binding_t {
	const char *name;
	int (*dev_parse)(struct dtree_dev_t *dev, FILE *f, const char *fname);
};

#define DTREE_GETTER static inline

/**
 * Get name of the device.
 */
DTREE_GETTER
const char *dtree_dev_name(const struct dtree_dev_t *d)
{
	return d->name;
}

/**
 * Get base address of the device.
 */
DTREE_GETTER
dtree_addr_t dtree_dev_base(const struct dtree_dev_t *d)
{
	return d->base;
}

/**
 * Get highest address of the device.
 * Eg.
 *    base  := 0x81000000
 *    high  := 0x8100FFFF
 *    range := 0x00010000
 *
 * If dev.high <= dev.base then high is invalid (eg. not provided by implementation).
 */
DTREE_GETTER
dtree_addr_t dtree_dev_high(const struct dtree_dev_t *d)
{
	return d->high;
}

/**
 * Get the list of compatible devices. Last entry
 * points to NULL.
 */
DTREE_GETTER
const char **dtree_dev_compat(const struct dtree_dev_t *d)
{
	return d->compat;
}

DTREE_GETTER
const char *dtree_dev_get_string_property(const struct dtree_dev_t *d, const char *name)
{
	struct dtree_data data;

	dtree_property_find(d, name, &data);

	return data.d.s;
}

DTREE_GETTER
uint32_t dtree_dev_get_integer_property(const struct dtree_dev_t *d, const char *name)
{
	struct dtree_data data;

	dtree_property_find(d, name, &data);

	return data.d.v;
}

//
// Iteration routines
//

/**
 * Returns next available device entry.
 * The entry should be free'd by dtree_dev_free().
 *
 * Uses shared internal iterator.
 * To search from beginning call dtree_reset().
 *
 * When no more entries are available or and error occoures
 * returns NULL. On error sets error state.
 */
struct dtree_dev_t *dtree_next(void);

/**
 * Look up for device by name. Returns the first occurence
 * of device with the given name.
 * The entry should be free'd by dtree_dev_free().
 *
 * Uses shared internal iterator.
 * To search from beginning call dtree_reset().
 *
 * Returns NULL when not found or on error.
 * On error sets error state.
 */
struct dtree_dev_t *dtree_byname(const char *name);

/**
 * Looks up for device compatible with the given type.
 * The entry should be free'd by dtree_dev_free().
 *
 * Uses shared internal iterator.
 * To search from beginning call dtree_reset().
 *
 * Returns NULL when not found or on error.
 * On error sets error state.
 */
struct dtree_dev_t *dtree_bycompat(const char *compat);

/**
 * Resets the iteration over devices.
 * Eg. after this call dtree_next() will return the first
 * device again. Affects the internal shared iterator.
 * 
 * Returns 0 on success. On error sets error state.
 */
int dtree_reset(void);


//
// Common functions
//

/**
 * Frees the given device entry (returned mostly by iterators).
 * It is recommended to free every dev instance before next
 * iterator call (or as soon as possible).
 */
void dtree_dev_free(struct dtree_dev_t *dev);

/**
 * Tests whether the module is in an error state.
 * When it is in an error state the behaviour of all
 * operations except dtree_close() and dtree_errstr()
 * is undefined.
 *
 * Returns true when the module is in an error state.
 *
 * Error handling has to be done before next possible
 * failing call. That call would overwrite current
 * error state.
 */
int dtree_iserror(void);

/**
 * When an error occures this function should
 * return a description of what happend.
 *
 * The pointer points to static memory.
 */
const char *dtree_errstr(void);

#endif
