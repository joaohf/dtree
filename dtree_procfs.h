/**
 * Internal /proc filesystem implementation.
 * Non-public API.
 * Jan Viktorin <xvikto03@stud.fit.vutbr.cz>
 */

#ifndef DTREE_PROC_FS
#define DTREE_PROC_FS

/**
 * Opens the /proc filesystem at the given path.
 * Most common: /proc/device-tree.
 * 
 * Initializes internal structures. Does not
 * clear error flag.
 */
int dtree_procfs_open(const char *rootd);

/**
 * Free's all resources.
 */
void dtree_procfs_close(void);

/**
 * Traversing over procfs.
 */
struct dtree_dev_t *dtree_procfs_next(void);

/**
 * Free of dtree_dev_t returned by procfs functions.
 */
void dtree_procfs_dev_free(struct dtree_dev_t *dev);

/**
 * Reset of iteration over procfs.
 */
int dtree_procfs_reset(void);

/**
 * Set dtree_binding array to get extra properties
 */
void dtree_procfs_set_bindings(struct dtree_binding_t *dt_b);

void dtree_procfs_unset_bindings();

int dev_parse_helper_string(struct dtree_dev_t *dev, FILE *f, const char *fname);

int dev_parse_helper_integer(struct dtree_dev_t *dev, FILE *f, const char *fname);

#endif

