#include "dtree.h"
#include "dtree_error.h"
#include "dtree_procfs.h"
#include "utlist.h"

#include <string.h>
#include <stdlib.h>

struct dtree_list_t {
	struct dtree_t *dt;

	struct dtree_list_t *next;
	struct dtree_list_t *prev;
};

static struct dtree_t *dtree_new()
{
	struct dtree_t *dt = calloc(1, sizeof(struct dtree_t));

	return dt;
}

static void dtree_destroy(struct dtree_t *dt)
{
	memset(dt, 0, sizeof(struct dtree_t));
	free((void *) dt);
}

static struct dtree_t *dtree_new_from_dev(struct dtree_dev_t *dev)
{
	struct dtree_t *dt = dtree_new();

	dt->curr = dev;

	return dt;
}

int dtree_open(const char *rootd, struct dtree_t **dt)
{
	struct dtree_t *dt_priv = dtree_new();

	int err = dtree_procfs_open(rootd, dt_priv);

	if(err == 0) {
		dtree_error_clear(dt_priv);
		*dt = dt_priv;
		return 0;
	}

	dtree_close(dt_priv);

	return err;
}

void dtree_close(struct dtree_t *dt)
{
	dtree_procfs_close(dt);
	dtree_destroy(dt);
}

struct dtree_dev_t *dtree_next(struct dtree_t *dt)
{
	return dtree_procfs_next(dt);
}

struct dtree_t *dtree_next_dev_match(struct dtree_t *dt)
{
	struct dtree_list_t *next  = NULL;
	struct dtree_list_t *tmp = NULL;

	DL_FOREACH_SAFE(dt->head, next, tmp) {
		DL_DELETE(dt->head, next);

		int err = dtree_procfs_open(next->dt->curr->fpath, next->dt);

		if(err != 0) {
			dtree_error_clear(next->dt);
			return NULL;
		}

		return next->dt;
	}

	return NULL;
}

void dtree_next_dev_free(struct dtree_t *dt)
{
	if (dt->curr)
		dtree_dev_free(dt->curr);
}

void dtree_dev_free(struct dtree_dev_t *dev)
{
	dtree_procfs_dev_free(dev);
}

int dtree_reset(struct dtree_t *dt)
{
	return dtree_procfs_reset(dt);
}

struct dtree_t *dtree_bymatch(struct dtree_t *dt, const char *match)
{
	struct dtree_dev_t *curr = NULL;
	struct dtree_t *tmp = NULL;
	struct dtree_list_t *el_tmp = NULL;
	int length;

	if(match == NULL || strlen(match) == 0)
		return NULL;

	length = strlen(match);

	while((curr = dtree_next(dt)) != NULL) {
		if(!strncmp(match, curr->name, length)) {
			tmp = dtree_new_from_dev(curr);

			el_tmp = calloc(1, sizeof(struct dtree_list_t));

			el_tmp->dt = tmp;

			DL_APPEND(dt->head, el_tmp);
		}
	}

	return dt;
}

struct dtree_dev_t *dtree_byname(struct dtree_t *dt, const char *name)
{
	struct dtree_dev_t *curr = NULL;

	if(name == NULL || strlen(name) == 0)
		return NULL;

	while((curr = dtree_next(dt)) != NULL) {
		if(!strcmp(name, curr->name))
			break;

		dtree_dev_free(curr);
	}

	return curr;
}

static
int is_compatible(const struct dtree_dev_t *dev, const char *compat)
{
	const char **dev_compat = dtree_dev_compat(dev);

	for(int i = 0; dev_compat[i] != NULL; ++i) {
		if(!strcmp(dev_compat[i], compat))
			return 1;
	}

	return 0;
}

struct dtree_dev_t *dtree_bycompat(struct dtree_t *dt, const char *compat)
{
	struct dtree_dev_t *curr = NULL;

	if(compat == NULL || strlen(compat) == 0)
		return NULL;

	while((curr = dtree_next(dt)) != NULL) {
		if(is_compatible(curr, compat))
			break;

		dtree_dev_free(curr);
	}

	return curr;
}
