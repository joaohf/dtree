#include "dtree.h"
#include "dtree_error.h"
#include "dtree_procfs.h"

#include <string.h>

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

void dtree_dev_free(struct dtree_dev_t *dev)
{
	dtree_procfs_dev_free(dev);
}

int dtree_reset(struct dtree_t *dt)
{
	return dtree_procfs_reset(dt);
}

struct dtree_dev_t *dtree_find_byname(struct dtree_t *dt, const char *match)
{
	struct dtree_dev_t *curr = NULL;
	struct dtree_dev_t *dev = calloc(1, sizeof(struct dtree_dev_t));

	if(match == NULL || strlen(match) == 0)
		return NULL;

	while((curr = dtree_next(dt)) != NULL) {
		if(strstr(match, curr->name)) {
			struct dtree_dir *dir_curr = calloc(1, sizeof(struct dtree_dir));

			dir_curr->dir = curr->dir;

			DL_APPEND(dev->dirlist, dir_curr);
		}
	}

	dev->name = match;

	return dev;
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
