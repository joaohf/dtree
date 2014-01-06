/*
 * dtree_properties.c
 *
 *  Created on: 06/01/2014
 *      Author: joaohf
 */
#include "dtree.h"
#include "uthash.h"

#include <stdint.h>
#include <string.h>

struct dtree_hash_properties {
	char name[32];
	struct dtree_data d;
	UT_hash_handle hh;
};

void dtree_property_add(struct dtree_dev_t *dev, const char *name, const void *ptr_value, int length)
{
	struct dtree_hash_properties *p;
	//struct dtree_hash_properties *pp = dev->properties;

	p = calloc(1, sizeof(struct dtree_hash_properties));

	if (length > 0)
		p->d.d.s = (char *)ptr_value;
	else
		p->d.d.v = *((uintptr_t *) ptr_value);

	p->d.length = length;
	strcpy(p->name, name);

	HASH_ADD_STR(dev->properties, name, p);
}

int dtree_property_find(const struct dtree_dev_t *dev, const char *name, struct dtree_data *d)
{
	struct dtree_hash_properties *p;

	HASH_FIND_STR(dev->properties, name, p);

	if (!p)
		return -1;

	*d = p->d;

	return 0;
}

void dtree_property_empty(struct dtree_dev_t * const dev)
{
	struct dtree_hash_properties *current, *tmp;

	HASH_ITER(hh, dev->properties, current, tmp) {
	    HASH_DEL(dev->properties, current);

	    if (current->d.length > 0)
	    	free((void *) current->d.d.s);
	}
}
