/*
 * dtree_properties.c
 *
 *  Created on: 06/01/2014
 *      Author: joaohf
 */
#include "dtree.h"
#include "dtree_priv.h"
#include "dtree_properties.h"
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
	struct dtree_dev_priv_t *pdev = (struct dtree_dev_priv_t *) dev;
	struct dtree_hash_properties *p;

	p = calloc(1, sizeof(struct dtree_hash_properties));

	if (length > 0)
		p->d.d.s = (char *)ptr_value;
	else
		p->d.d.v = *((uintptr_t *) ptr_value);

	p->d.length = length;
	strcpy(p->name, name);

	HASH_ADD_STR(pdev->properties, name, p);
}

int dtree_property_find(const struct dtree_dev_t *dev, const char *name, struct dtree_data *d)
{
	struct dtree_dev_priv_t *pdev = (struct dtree_dev_priv_t *)dev;
	struct dtree_hash_properties *p;

	HASH_FIND_STR(pdev->properties, name, p);

	if (!p)
		return -1;

	*d = p->d;

	return 0;
}

void dtree_property_empty(struct dtree_dev_t * const dev)
{
	struct dtree_dev_priv_t *pdev = (struct dtree_dev_priv_t *)dev;
	struct dtree_hash_properties *current, *tmp;

	HASH_ITER(hh, pdev->properties, current, tmp) {
	    HASH_DEL(pdev->properties, current);

	    if (current->d.length > 0)
	    	free((void *) current->d.d.s);
	}
}

const char *dtree_dev_get_string_property(const struct dtree_dev_t *d, const char *name)
{
	struct dtree_data data;

	dtree_property_find(d, name, &data);

	return data.d.s;
}

uint32_t dtree_dev_get_integer_property(const struct dtree_dev_t *d, const char *name)
{
	struct dtree_data data;

	dtree_property_find(d, name, &data);

	return data.d.v;
}

