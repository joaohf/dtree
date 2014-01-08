/**
 * dtree_procfs.c
 * Copyright (C) 2013 Jan Viktorin
 */

#define _BSD_SOURCE

#include "dtree.h"
#include "dtree_error.h"
#include "dtree_util.h"
#include "dtree_procfs.h"
#include "dtree_properties.h"
#include "stack.h"

#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

struct dtree_procfs_t {
	struct stack *path;
	DIR *dir;
};

static struct dtree_binding_t *dtree_bindings = NULL;

static const char *NULL_ENTRY = NULL;

static
int stack_push_fname(struct stack **path, const char *fname)
{
	return stack_push_dup(path, fname, strlen(fname) + 1);
}

static
void *stack_pop_fname(struct stack **path)
{
	return stack_pop(path);
}

void dtree_procfs_set_bindings(struct dtree_binding_t *dt_b)
{
	dtree_bindings = dt_b;
}

void dtree_procfs_unset_bindings()
{
	dtree_bindings = NULL;
}

/**
 * This implementation doesn't accept a regular
 * file as rootd.
 */
int dtree_procfs_open(const char *rootd, struct dtree_t *dt)
{
	struct dtree_procfs_t *procfs;

	procfs = calloc(1, sizeof(struct dtree_procfs_t));
	if (procfs == NULL)
		goto exit_no_clean;

	dt->procfs = procfs;

	procfs->dir = NULL;
	procfs->path = NULL;

	if(rootd == NULL) {
		dtree_error_set(dt, EINVAL);
		goto exit_no_clean;
	}

	if(!stack_empty(&procfs->path)) {
		dtree_error_set(dt, EBUSY); // call close first
		goto clean_and_exit;
	}

	procfs->dir = opendir(rootd);
	if(procfs->dir == NULL) {
		dtree_error_from_errno(dt);
		goto clean_and_exit;
	}

	if(stack_push_fname(&procfs->path, rootd)) {
		dtree_error_from_errno(dt);
		closedir(procfs->dir);
		procfs->dir = NULL;
		goto clean_and_exit;
	}

	return 0;

clean_and_exit:
    dtree_procfs_close(dt);
    return -1;

exit_no_clean:
	dt->procfs = procfs;
	return -1;
}

void dtree_procfs_close(struct dtree_t *dt)
{
	struct dtree_procfs_t *procfs = dt->procfs;

	if (!procfs)
		return;

	if(procfs->dir != NULL) {
		closedir(procfs->dir);
		procfs->dir = NULL;
	}

	while(!stack_empty(&procfs->path)) {
		void *d = stack_pop_fname(&procfs->path);
		free(d);
	}

	free((void *) procfs);
}

int dtree_procfs_reset(struct dtree_t *dt)
{
	struct dtree_procfs_t *procfs = dt->procfs;

	if(procfs->dir != NULL) {
		closedir(procfs->dir);
		procfs->dir = NULL;
	}

	while(stack_depth(&procfs->path) > 1) {
		void *p = stack_pop_fname(&procfs->path);
		free(p);
	}

	assert(!stack_empty(&procfs->path));
	procfs->dir = opendir(stack_top(&procfs->path));

	return procfs->dir == NULL;
}

static inline
int st_is_dir(int st_mode)
{
	return S_ISDIR(st_mode);
}

static inline
int st_is_file(int st_mode)
{
	return S_ISREG(st_mode);
}

static
const char *file_path_from_stack(struct stack **path, const char *fname)
{
	assert(fname != NULL);

	const size_t fnamelen = strlen(fname);
	struct stack *tmp = NULL;
	size_t plen  = 0;

	while(!stack_empty(path)) {
		const void *frag = stack_move(path, &tmp);
		assert(frag != NULL);

		plen  += strlen((const char *) frag) + 1;
	}

	char *full = malloc(plen + fnamelen + 1);
	if(full == NULL) {
		while(!stack_empty(&tmp))
			stack_move(&tmp, path);

		return NULL;
	}

	char *p = full;
	while(!stack_empty(&tmp)) {
		const void *frag  = stack_move(&tmp, path);
		const size_t flen = strlen((const char *) frag);

		memcpy(p, frag, flen);
		p[flen] = '/';
		p += flen + 1;
	}

	assert((size_t) (p - full) == plen);

	if(fnamelen > 0) {
		memcpy(p, fname, fnamelen);
		p[fnamelen] = '\0';
	}
	else {
		full[plen - 1] = '\0';
	}

	return full;
}

static
FILE *path_fopen(struct dtree_t *dt, struct stack **path, const char *fname, const char *mode)
{
	const char *fpath = file_path_from_stack(path, fname);
	if(fpath == NULL)
		goto clean_and_exit;

	FILE *file = fopen(fpath, mode);
	if(file == NULL)
		goto clean_and_exit;

	free((void *) fpath);
	return file;

clean_and_exit:
	dtree_error_from_errno(dt);
	if(fpath != NULL)
		free((void *) fpath);
	return NULL;
}

static
int path_stat(struct dtree_t *dt, struct stack **path, const char *fname, struct stat *st)
{
	const char *fpath = file_path_from_stack(path, fname);
	if(fpath == NULL)
		goto clean_and_exit;

	if(stat(fpath, st))
		goto clean_and_exit;

	free((void *) fpath);
	return 0;

clean_and_exit:
	dtree_error_from_errno(dt);
	if(fpath != NULL)
		free((void *) fpath);
	return 1;
}

static
int path_is_file(struct dtree_t *dt, struct stack **path, const char *fname)
{
	struct stat st;
	if(path_stat(dt, path, fname, &st))
		return 0;

	return st_is_file(st.st_mode);
}

static
int path_is_dir(struct dtree_t *dt, struct stack **path, const char *fname)
{
	struct stat st;
	if(path_stat(dt, path, fname, &st))
		return 0;

	return st_is_dir(st.st_mode);
}

static
DIR *opendir_on_stack(struct dtree_t *dt, struct stack **path)
{
	const char *fpath = file_path_from_stack(path, "");
	if(fpath == NULL)
		goto clean_and_exit;

	DIR *dir = opendir(fpath);
	if(dir == NULL)
		goto clean_and_exit;

	free((void *) fpath);
	return dir;

clean_and_exit:
	dtree_error_from_errno(dt);
	if(fpath != NULL)
		free((void *) fpath);
	return NULL;
}

static
int dir_has_file(struct dtree_t *dt, DIR *curr, struct stack **path, const char *fname)
{
	rewinddir(curr);
	struct dirent *d;

	while((d = readdir(curr)) != NULL) {
		if(strcmp(d->d_name, fname))
			continue;

		if(path_is_file(dt, path, fname))
			return 1;

		if(dtree_iserror(dt))
			return 0;
	}

	return 0;
}

static
DIR *open_dir_from_dirent(struct dtree_t *dt, struct dirent *d, struct stack **path)
{
	if(d == NULL)
		return NULL;

	if(stack_push_fname(path, d->d_name)) {
		dtree_error_from_errno(dt);
		return NULL;
	}

	return opendir_on_stack(dt, path);
}

static
DIR *go_next_dir(struct dtree_t *dt, DIR *curr, struct stack **path)
{
	struct dirent *d;

	while((d = readdir(curr)) != NULL) {
		if(!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;

		if(path_is_dir(dt, path, d->d_name))
			break;

		if(dtree_iserror(dt))
			return NULL;
	}

	return open_dir_from_dirent(dt, d, path);
}

static
DIR *go_up_next_dir(struct dtree_t *dt)
{
	struct dtree_procfs_t *procfs = dt->procfs;
	DIR *next = NULL;

	do {
		if(stack_depth(&procfs->path) == 1) // never loose the rootd
			return NULL;

		const char *dname = (const char *) stack_pop_fname(&procfs->path);

		DIR *dir = opendir_on_stack(dt, &procfs->path);
		if(dir == NULL) {
			dtree_error_from_errno(dt);
			free((void *) dname);
			return NULL;
		}

		struct dirent *d;
		while((d = readdir(dir)) != NULL) {
			if(!strcmp(d->d_name, dname))
				break;
		}
		free((void *) dname);

		next = go_next_dir(dt, dir, &procfs->path);
		if(next == NULL && dtree_iserror(dt)) {
			closedir(dir);
			return NULL;
		}

		closedir(dir);
	} while(next == NULL);

	return next;
}

static
void *file_read_and_close(struct dtree_t *dt, FILE *file, size_t *flen)
{
	struct stat file_stat;
	if(fstat(fileno(file), &file_stat)) {
		dtree_error_from_errno(dt);
		fclose(file);
		return NULL;
	}

	const size_t fsize = file_stat.st_size;

	void *m = malloc(fsize + 1);
	if(m == NULL) {
		dtree_error_from_errno(dt);
		fclose(file);
		return NULL;
	}
	
	size_t rlen = fread(m, 1, fsize, file);
	if(rlen < fsize) {
		dtree_error_from_errno(dt);
		free(m);
		fclose(file);
		return NULL;
	}

	((char *) m)[fsize] = '\0';
	fclose(file);

	*flen = fsize;
	return m;
}

static
uint32_t convert_raw32(const char *s) {
	uint32_t value = 0;

	for(int i = 0; i < 4; ++i) {
		uint32_t val = (uint32_t) s[3 - i];	
		val &= 0xFF;
		value += val << i * 8;
	}

	return value;
}

static
int dev_parse_reg(struct dtree_t *dt, struct dtree_dev_t *dev, struct stack **path, const char *fname)
{
	FILE *regf = path_fopen(dt, path, fname, "r");
	if(regf == NULL)
		return 1;

	size_t length = 0;
	const char *content = file_read_and_close(dt, regf, &length);
	if(content == NULL)
		return 2;

	if(length != 8) {
		dtree_error_clear(dt);
		free((void *) content);
		return 3;
	}

	dev->base = convert_raw32(content);
	dev->high = dev->base + convert_raw32(content + 4) - 1;

	free((void *) content);
	return 0;
}

static
const char **convert_compat(const char *compat, const size_t len)
{
	// count entries
	int entries = 0;

	// each '\0' is end of an entry
	for(const char *p = compat; (size_t) (p - compat) < len; ++p) {
		if(*p == '\0')
			entries += 1;
	}

	// alloc pointers for entries and one pointer for last NULL
	const char **array = (const char **) malloc((entries + 1) * sizeof(char *));
	if(array == NULL)
		return NULL;

	// assign pointers to point into the compat string
	int off = 0;

	for(int i = 0; i < entries; ++i) {
		array[i] = compat + off;

		// find next end of entry
		while(compat[off] != '\0')
			off += 1;

		off += 1; // skip '\0', point to the next entry
	}

	array[entries] = NULL;
	return array;
}

static
int dev_parse_compat(struct dtree_t *dt, struct dtree_dev_t *dev, struct stack **path, const char *fname)
{
	FILE *regf = path_fopen(dt, path, fname, "r");
	if(regf == NULL)
		return 1;

	size_t length = 0;
	const char *content = file_read_and_close(dt, regf, &length);
	if(content == NULL)
		return 2;

	dev->compat = convert_compat(content, length);
	if(dev->compat == NULL) {
		dtree_error_from_errno(dt);
		return 1;
	}

	return 0;
}

int dev_parse_helper_string(struct dtree_t *dt, struct dtree_dev_t *dev, FILE *f, const char *fname)
{
	if(f == NULL)
		return 1;

	size_t length = 0;
	const char *content = file_read_and_close(dt, f, &length);
	if(content == NULL)
		return 2;

	dtree_property_add(dev, fname, content, length);

	return 0;
}

int dev_parse_helper_integer(struct dtree_t *dt, struct dtree_dev_t *dev, FILE *f, const char *fname)
{
	if(f == NULL)
		return 1;

	size_t length = 0;
	const char *content = file_read_and_close(dt, f, &length);
	if(content == NULL)
		return 2;

	if(length > 4) {
		//dtree_error_clear();
		free((void *) content);
		return 3;
	}

	uint32_t value = convert_raw32(content);

	free((void *) content);

	dtree_property_add(dev, fname, &value, 0);

	return 0;
}

static
int dev_parse_bindings(struct dtree_t *dt, struct dtree_dev_t *dev, struct stack **path, char *fname)
{
	(void) dt;

	struct dtree_binding_t *dtree_binding;

	for (dtree_binding = dtree_bindings; dtree_binding->name != NULL; dtree_binding++) {
		if(!strcmp(fname, dtree_binding->name)) {
			FILE *f = path_fopen(dt, path, fname, "r");

			if (dtree_binding->dev_parse(dt, dev, f, dtree_binding->name))
				fclose(f);
		}
	}

	return 0;
}

static
struct dtree_dev_t *dev_from_dir(struct dtree_t *dt, DIR *curr, struct stack **path)
{
	struct dtree_dev_t *dev = malloc(sizeof(struct dtree_dev_t));
	if(dev == NULL) {
		dtree_error_from_errno(dt);
		return NULL;
	}

//	if (!dt->flag_disable_root_dev)
//		assert(stack_depth(path) > 1); // the root is never a device

	dev->properties = NULL;
	dev->compat = &NULL_ENTRY;
	dev->name = strdup((char *) stack_top(path));
	if(dev->name == NULL) {
		dtree_error_from_errno(dt);
		free(dev);
		return NULL;
	}

	dev->fpath = file_path_from_stack(path, "");

	uint32_t isnode = 0;
	if (stack_depth(path) == 1) {
		uint32_t isnode = 1;
		dtree_property_add(dev, "isnode", &isnode, 0);
	} else {
		dtree_property_add(dev, "isnode", &isnode, 0);
	}

	long name_max;
	struct dirent *d, *buf = NULL;

	name_max = pathconf(dev->name, _PC_NAME_MAX);
	if (name_max == -1)
		name_max = 255;

	buf = (struct dirent *)malloc(
            offsetof(struct dirent, d_name) + name_max + 1);

	rewinddir(curr);
	while((readdir_r(curr, buf, &d) == 0) && d) {
		if(!path_is_file(dt, path, d->d_name))
			continue;

		if(!strcmp(d->d_name, "reg")) {
			if(dev_parse_reg(dt, dev, path, "reg"))
				goto clean_and_exit;
		}
		if(!strcmp(d->d_name, "compatible")) {
			if(dev_parse_compat(dt, dev, path, "compatible"))
				goto clean_and_exit;
		}

		if (dtree_bindings) {
			if(dev_parse_bindings(dt, dev, path, d->d_name))
				goto clean_and_exit;
		}
	}

	free((void *) buf);

	return dev;

clean_and_exit:
	free((void *) buf);
	dtree_procfs_dev_free(dev);
	return NULL;
}

struct dtree_dev_t *dtree_procfs_next(struct dtree_t *dt)
{
	struct dtree_procfs_t *procfs = dt->procfs;

	if(procfs->dir == NULL)
		return NULL;

	struct dtree_dev_t *dev = NULL;

	while(dev == NULL && procfs->dir != NULL) {
		if(dir_has_file(dt, procfs->dir, &procfs->path, "reg")) {
			dev = dev_from_dir(dt, procfs->dir, &procfs->path);

			if(dev == NULL && dtree_iserror(dt))
				return NULL;
		}

		rewinddir(procfs->dir);
		DIR *dir = go_next_dir(dt, procfs->dir, &procfs->path);
		if(dir == NULL && dtree_iserror(dt)) {
			return NULL;
		}
		
		if(dir == NULL)
			dir = go_up_next_dir(dt);

		if(dir == NULL && dtree_iserror(dt)) {
			return NULL;
		}

		closedir(procfs->dir);
		procfs->dir = dir;
	}

	return dev;
}

void dtree_procfs_dev_free(struct dtree_dev_t *dev)
{
	assert(dev != NULL);

	if(dev->name != NULL)
		free((void *) dev->name);

	assert(dev->compat != NULL);
	if(dev->compat != &NULL_ENTRY) {
		free((void *) dev->compat[0]);
		free((void *) dev->compat);
	}

	dev->name   = NULL;
	dev->compat = NULL;
	dev->base = 0;
	dev->high = 0;

	if (dev->properties) {
		dtree_property_empty(dev);
		dev->properties = NULL;
	}

	free(dev);
}
