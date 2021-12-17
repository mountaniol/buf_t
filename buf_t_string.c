/** @file */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <stddef.h>

#include "buf_t.h"
#include "buf_t_stats.h"
#include "buf_t_debug.h"
#include "buf_t_memory.h"


/* TODO: Split this funtion into set of function per type */
/* Validate sanity of buf_t */
ret_t buf_str_is_valid(buf_t *buf)
{
	if (NULL == buf) {
		DE("Invalid: got NULL pointer\n");
		TRY_ABORT();
		return (EINVAL);
	}

	/* If the buf is string the room must be greater than used */
	/* If there is a data, then:
	   the room must be greater than used, because we do not count terminating \0 */
	if ((NULL != buf->data) && (buf_room(buf) <= buf_used(buf)) ) {
		DE("Invalid STRING buf: buf->used (%ld) >= buf->room (%ld)\n", buf_used(buf), buf_room(buf));
		TRY_ABORT();
		return (ECANCELED);
	}

	/* For string buffers only: check that the string is null terminated */
	/* If the 'used' area not '\0' terminated - invalid */
	if (IS_BUF_STRING(buf) && (NULL != buf->data) && ('\0' != *(buf->data + buf_used(buf)))) {
		DE("Invalid STRING buf: no '0' terminated\n");
		DE("used = %ld, room = %ld, last character = |%c|, string = %s\n", buf_used(buf), buf_room(buf), *(buf->data + buf_used(buf)), buf->data);
		TRY_ABORT();
		return (ECANCELED);
	}

	DDD0("Buffer is valid\n");
	//buf_print_flags(buf);
	return (OK);
}

