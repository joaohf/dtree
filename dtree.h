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

#include <stdint.h>
#include <stdio.h>

//
// Handle Data types
//

/**
 * dtree handle
 */
struct dtree_t;

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
int dtree_open(const char *rootd, struct dtree_t **dt);

/**
 * Free's resources of the module.
 * It is an error to call it when dtree_open()
 * has failed or to call it twice.
 */
void dtree_close(struct dtree_t *dt);

//
// Data types
//

/**
 * Representation of address.
 */
typedef uint32_t dtree_addr_t;

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
};

struct dtree_binding_t {
	const char *name;
	int (*dev_parse)(struct dtree_t *dt, struct dtree_dev_t *dev, FILE *f, const char *fname);
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

/**
 * Get a string property from dev
 */
const char *dtree_dev_get_string_property(const struct dtree_dev_t *d, const char *name);

/**
 * Get a integer property from dev
 */
uint32_t dtree_dev_get_integer_property(const struct dtree_dev_t *d, const char *name, int *error);

//
// Iteration routines
//

/**
 * Returns next available device entry.
 * The entry should be free'd by dtree_dev_free().
 *
 * To search from beginning call dtree_reset().
 *
 * When no more entries are available or and error occoures
 * returns NULL. On error sets error state.
 */
struct dtree_dev_t *dtree_next(struct dtree_t *dt);

/**
 * Returns next available device entry that matched.
 * The entry should be free'd by dtree_next_dev_free().
 *
 * To search from beginning call dtree_reset().
 *
 * When no more entries are available or and error occoures
 * returns NULL. On error sets error state.
 */
struct dtree_t *dtree_next_dev_match(struct dtree_t *dt);

/**
 * Look up for device by match name. Returns all occurences
 * of device with the given match name.
 * The entry should be free'd by dtree_dev_free().
 *
 * To search from beginning call dtree_reset().
 *
 * Returns NULL when not found or on error.
 * On error sets error state.
 */
struct dtree_t *dtree_bymatch(struct dtree_t *dt, const char *match);

/**
 * Look up for device by name. Returns the first occurence
 * of device with the given name.
 * The entry should be free'd by dtree_dev_free().
 *
 * To search from beginning call dtree_reset().
 *
 * Returns NULL when not found or on error.
 * On error sets error state.
 */
struct dtree_dev_t *dtree_byname(struct dtree_t *dt, const char *name);

/**
 * Looks up for device compatible with the given type.
 * The entry should be free'd by dtree_dev_free().
 *
 * To search from beginning call dtree_reset().
 *
 * Returns NULL when not found or on error.
 * On error sets error state.
 */
struct dtree_dev_t *dtree_bycompat(struct dtree_t *dt, const char *compat);

/**
 * Resets the iteration over devices.
 * Eg. after this call dtree_next() will return the first
 * device again. Affects the internal shared iterator.
 * 
 * Returns 0 on success. On error sets error state.
 */
int dtree_reset(struct dtree_t *dt);

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
 * Frees the given device tree entry (returned mostly by matched iterators).
 * It is recommended to free every dev instance before next
 * iterator call (or as soon as possible).
 */
void dtree_next_dev_free(struct dtree_t *dt);

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
int dtree_iserror(struct dtree_t *dt);

/**
 * When an error occures this function should
 * return a description of what happend.
 *
 * The pointer points to static memory.
 */
const char *dtree_errstr(struct dtree_t *dt);

/**
 * Set dtree_binding array to get extra properties
 */
void dtree_procfs_set_bindings(struct dtree_binding_t *dt_b);

/**
 * Unset dtree_binding array
 */
void dtree_procfs_unset_bindings();

/**
 * Helper function to parse a string
 */
int dev_parse_helper_string(struct dtree_t *dt, struct dtree_dev_t *dev, FILE *f, const char *fname);

/**
 * Helper function to parse a integer
 */
int dev_parse_helper_integer(struct dtree_t *dt, struct dtree_dev_t *dev, FILE *f, const char *fname);

#endif
