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

#endif

