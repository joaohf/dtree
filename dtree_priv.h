/*
 * dtree_priv.h
 *
 *  Created on: Jan 9, 2014
 *      Author: jfreitas
 */

#ifndef DTREE_PRIV_H_
#define DTREE_PRIV_H_

struct dtree_procfs_t;
struct dtree_list_t;
struct dtree_dev_t;

struct dtree_t {
	/**
	 * Error state.
	 */
	int error;

	/**
	 * Holds errno. It is valid when error
	 * is less then zero.
	 */
	int xerrno;

	struct dtree_procfs_t *procfs;
	struct dtree_list_t *head;
	struct dtree_list_t *head_orphan;
	struct dtree_dev_t *curr;
};

typedef struct dtree_hash_properties dtree_hash_properties;

struct dtree_dev_priv_t {
	struct dtree_dev_t base;

	const char *fpath;
	dtree_hash_properties *properties;
};

#endif /* DTREE_PRIV_H_ */
