/*
 * dtree_properties.h
 *
 *  Created on: 06/01/2014
 *      Author: joaohf
 */

#ifndef DTREE_PROPERTIES_H_
#define DTREE_PROPERTIES_H_

#include <stdint.h>

struct dtree_dev_t;

struct dtree_data {
	int length;
	union {
		uint32_t v;
		char *s;
	} d;
};

void dtree_property_add(struct dtree_dev_t *dev, const char *name, const void *ptr_value, int length);

int dtree_property_find(const struct dtree_dev_t *dev, const char *name, struct dtree_data *d);

void dtree_property_empty(struct dtree_dev_t * const dev);

#endif /* DTREE_PROPERTIES_H_ */
