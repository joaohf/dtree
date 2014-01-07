#include "dtree.h"
#include "dtree_error.h"

#include <errno.h>
#include <string.h>
#include <assert.h>

#define ERRSTR_COUNT ((int) (sizeof(errstr)/sizeof(char *)))
static const char *errstr[] = {
	[0]                       = "Successful",
	[DTREE_ECANT_READ_ROOT]   = "Can not read the root dir"
};

void dtree_error_clear(struct dtree_t *dt)
{
	dt->error  = 0;
	dt->xerrno = 0;
}

void dtree_error_set(struct dtree_t *dt, int e)
{
	assert(e != 0);

	dt->error  = e;
	dt->xerrno = errno;
}

void dtree_errno_set(struct dtree_t *dt, int e)
{
	dt->error  = -1;
	dt->xerrno = e;
}

void dtree_error_from_errno(struct dtree_t *dt)
{
	if(errno != 0)
		dtree_errno_set(dt, errno);
}

int dtree_iserror(struct dtree_t *dt)
{
	if (!dt)
		return 0;

	return dt->error != 0;
}

const char *dtree_errstr(struct dtree_t *dt)
{
	if(dt->error >= 0 && dt->error < ERRSTR_COUNT)
		return errstr[dt->error];

	if(dt->error < 0)
		return strerror(dt->xerrno);

	return "Unknown error occured";
}

