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

void dtree_property_add(struct dtree_dev_t *dev, const char *name, const void *ptr_value, int length, int type)
{
	struct dtree_dev_priv_t *pdev = (struct dtree_dev_priv_t *) dev;
	struct dtree_hash_properties *p;

	p = calloc(1, sizeof(struct dtree_hash_properties));

	switch (type) {
	case DTREE_DATA:
		p->d.d.p = (uintptr_t *) ptr_value;
		break;
	case DTREE_DATA_INT:
		p->d.d.v = *((uintptr_t *) ptr_value);
		break;
	case DTREE_DATA_STRING:
		p->d.d.s = (char *)ptr_value;
		break;
	}

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

	    if (current->d.length > 0) {
	    	free((void *) current->d.d.s);
	    	current->d.d.s = NULL;
	    }

	    free(current);
	}
}

const char *dtree_dev_get_string_property(const struct dtree_dev_t *d, const char *name)
{
	struct dtree_data data;

	if (dtree_property_find(d, name, &data) < 0) {
		return NULL;
	}

	return data.d.s;
}

uint32_t dtree_dev_get_integer_property(const struct dtree_dev_t *d, const char *name, int *error)
{
	struct dtree_data data;

	*error = 0;

	if (dtree_property_find(d, name, &data) < 0) {
		*error = -1;

		return 0;
	}

	return data.d.v;
}

int dtree_dev_get_int_data_property(const struct dtree_dev_t *d, const char *name,
		uintptr_t *pdata, int pdata_size)
{
	struct dtree_data data;

	if (dtree_property_find(d, name, &data) < 0) {
		return -1;
	}

	if (data.length != pdata_size)
		return -1;

	memcpy(pdata, data.d.p, pdata_size);

	return 0;
}

