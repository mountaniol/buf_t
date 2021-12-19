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

//#define BUF_DEBUG 1

/* TODO: Split this funtion into set of function per type */
/* Validate sanity of buf_t */
ret_t buf_str_is_valid(buf_t *buf)
{
	if (NULL == buf) {
		DE("Invalid: got NULL pointer\n");
		TRY_ABORT();
		return (-EINVAL);
	}

	/* If the buf is string the room must be greater than used */
	/* If there is a data, then:
	   the room must be greater than used, because we do not count terminating \0 */
	if ((NULL != buf->data) && (buf_room(buf) <= buf_used(buf))) {
		DE("Invalid STRING buf: buf->used (%ld) >= buf->room (%ld)\n", buf_used(buf), buf_room(buf));
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* For string buffers only: check that the string is null terminated */
	/* If the 'used' area not '\0' terminated - invalid */
	if (IS_BUF_STRING(buf) && (NULL != buf->data) && ('\0' != *(buf->data + buf_used(buf)))) {
		DE("Invalid STRING buf: no '0' terminated\n");
		DE("used = %ld, room = %ld, last character = |%c|, string = %s\n", buf_used(buf), buf_room(buf), *(buf->data + buf_used(buf)), buf->data);
		TRY_ABORT();
		return (-ECANCELED);
	}

	DDD0("Buffer is valid\n");
	//buf_print_flags(buf);
	return (OK);
}

/*@null@*/ buf_t *buf_string(buf_s64_t size)
{
	buf_t *buf = NULL;
	buf = buf_new(size);

	T_RET_ABORT(buf, NULL);

	if (OK != buf_mark_string(buf)) {
		DE("Can't set STRING flag\n");
		abort();
	}
	return (buf);
}

/*@null@*/ buf_t *buf_from_string(/*@null@*/char *str, const buf_s64_t size_without_0)
{
	/*@temp@*/buf_t *buf = NULL;
	/* The string must be not NULL */
	T_RET_ABORT(str, NULL);

	/* Test that the string is '\0' terminated */
	if (*(str + size_without_0) != '\0') {
		DE("String is not null terminated\n");
		TRY_ABORT();
		return (NULL);
	}

	buf = buf_new(0);
	TESTP(buf, NULL);

	if (OK != buf_set_flag(buf, BUF_T_STRING)) {
		DE("Can't set STRING flag\n");
		if (OK != buf_free(buf)) {
			DE("Can't release a buffer\n");
		}
		TRY_ABORT();
		return (NULL);
	}
	/* We set string into buffer. The 'room' len contain null terminatior, the 'used' for string
	   doesn't */
	if (OK != buf_set_data(buf, str, size_without_0 + 1, size_without_0)) {
		DE("Can't set string into buffer\n");
		/* Just in case: Disconnect buffer from the buffer before release it */
		buf->data = NULL;
		if (OK != buf_set_room(buf, 0)) {
			DE("Can not set a new room value to the buffer\n");
			TRY_ABORT();
			return (NULL);
		}
		if (OK != buf_set_used(buf, 0)) {
			DE("Can not set a new 'used' value to the buffer\n");
			TRY_ABORT();
			return (NULL);
		}

		if (OK != buf_free(buf)) {
			DE("Can not free a buffer\n");
			TRY_ABORT();
			return (NULL);
		}

		TRY_ABORT();
		return (NULL);
	}

	return (buf);
}

ret_t buf_str_add(/*@null@*/buf_t *buf, /*@null@*/const char *new_data, const buf_s64_t size)
{
	size_t new_size;

	T_RET_ABORT(buf, -EINVAL);
	T_RET_ABORT(new_data, -EINVAL);

	new_size = size;
	/* If this buffer is a string buffer, we should consider \0 after string. If this buffer is empty,
	   we add +1 for the \0 terminator. If the buffer is not empty, we reuse existing \0 terminator */
	if (buf_used(buf) == 0) {
		new_size += sizeof(char);
	}

	/* Add room if needed: buf_test_room() adds room if needed */
	if (0 != buf_test_room(buf, new_size)) {
		DE("Can't add room into buf_t\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	/* All done, now add new data into the buffer */
	memcpy(buf->data + buf_used(buf), new_data, size);
	if (OK != buf_inc_used(buf, size)) {
		DE("Can not increase 'used'\n");
		return (BAD);
	}
	BUF_TEST(buf);
	return (OK);
}

ret_t buf_str_detect_used(/*@null@*/buf_t *buf)
{
	buf_s64_t calculated_used_size;
	T_RET_ABORT(buf, -EINVAL);

	/* If the buf is empty - return with error */
	if (0 == buf_room(buf)) {
		DE("Tryed to detect used in an empty buffer\n");
		return (-ECANCELED);
	}

	/* We start with the current 'used' size */
	calculated_used_size = buf_used(buf);

	/* Search for the first NOT 0 character - this is the end of 'used' area */
	/* TODO: Replace this with a binary search:
	   We start from the end, and if there is 0 detected, we start test it towards beginning using binary search */
	while (calculated_used_size > 0) {
		/* If found not null in the buffer... */
		if (0 != buf->data[calculated_used_size]) {
			break;
		}
		calculated_used_size--;
	}

	/* We increase it by 1 because it is index of array, i.e. starts from 0, but we need the legth, i.e. starts from 1 */
	calculated_used_size++;

	/* If the calculated used_size is te same as set in the buffer - nothing to do */
	if (calculated_used_size == buf_used(buf)) {
		DDD0("No need new string size: %ld -> %ld\n", buf_used(buf), calculated_used_size);
		return OK;
	}

	DDD0("Setting new string size: %ld -> %ld\n", buf_used(buf), calculated_used_size);
	/* No, the new size if less than the current */
	if (OK != buf_set_used(buf, calculated_used_size)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (BAD);
	}
	return (OK);
}

ret_t buf_str_pack(/*@null@*/buf_t *buf)
{
	/*@temp@*/ char   *tmp;
	size_t new_size = -1;
	ret_t  ret;

	T_RET_ABORT(buf, -EINVAL);
	/* If the buf is empty - return with error */
	if (0 == buf_room(buf)) {
		DE("Tryed to pack an empty buffer\n");
		return (-ECANCELED);
	}

	ret = buf_str_detect_used(buf);
	if (OK != ret) {
		return (ret);
	}

	new_size = buf_used(buf);
	if (IS_BUF_CANARY(buf)) {
		new_size += BUF_T_CANARY_SIZE;
	}

	if (IS_BUF_CRC(buf)) {
		new_size += BUF_T_CRC_SIZE;
	}

	new_size++;

	DDD0("Going to resize the buf room %lu -> %lu\n", buf_room(buf), new_size);

	/* TODO: Consider CRC + CANARY */
	tmp = realloc(buf->data, new_size);

	/* Case 1: realloc can't reallocate */
	if (NULL == tmp) {
		DE("Realloc failed\n");
		return (-ENOMEM);
	}

	/* Case 2: realloc succeeded, new memory returned */
	/* No need to free the old memory - done by realloc */
	if (NULL != tmp) {
		buf->data = tmp;
	}

	if (OK != buf_set_room(buf, new_size)) {
		DE("Can not set a new room value to the buffer\n");
		return (BAD);
	}

	return (OK);
}

buf_t *buf_sprintf(const char *format, ...)
{
	va_list args;
	buf_t   *buf = NULL;
	int     rc   = -1;

	T_RET_ABORT(format, NULL);

	/* Create buf_t with reserved room for the string */
	buf = buf_string(0);
	T_RET_ABORT(buf, NULL);

	va_start(args, format);
	/* Measure string lengh */
	rc = vsnprintf(NULL, 0, format, args);
	va_end(args);

	DDD("Measured string size: it is %d\n", rc);

	/* Allocate buffer: we need +1 for final '\0' */
	rc = buf_add_room(buf, rc + 1);

	if (OK != rc) {
		DE("Can't add room to buf\n");
		if (OK != buf_free(buf)) {
			DE("Warning, can't free buf_t, possible memory leak\n");
		}
		return (NULL);
	}
	va_start(args, format);
	rc = vsnprintf(buf->data, buf_room(buf), format, args);
	va_end(args);

	if (rc < 0) {
		DE("Can't print string\n");
		if (OK != buf_free(buf)) {
			DE("Warning, can't free buf_t, possible memory leak\n");
		}
		return (NULL);
	}

	if (OK != buf_set_used(buf, buf_room(buf) - 1)) {
		DE("Can not set a new 'used' value to the buffer\n");
		if (OK != buf_free(buf)) {
			DE("Warning, can't free buf_t, possible memory leak\n");
		}
		return (NULL);
	}

	if (OK != buf_is_valid(buf)) {
		DE("Buffer is invalid - free and return\n");
		TRY_ABORT();
		if (OK != buf_free(buf)) {
			DE("Can not crelease a buffer\n");
		}
		return (NULL);
	}

	return (buf);
}

ret_t buf_str_concat(buf_t *dst, buf_t *src)
{
	T_RET_ABORT(src, -EINVAL);
	T_RET_ABORT(dst, -EINVAL);

	if (OK != buf_is_string(src)) {
		DE("src buffer is not string\n");
		TRY_ABORT();
		return BAD;
	}

	if (OK != buf_is_string(dst)) {
		DE("dst buffer is not string\n");
		TRY_ABORT();
		return BAD;
	}

	if (OK != buf_add_room(dst, buf_used(src))) {
		DE("Can not add room for string copy");
	}

	memcpy(dst->data + buf_used(dst), src->data, buf_used(src));
	dst->used += buf_used(src);
	dst->data[buf_used(dst)] = '\0';
	return OK;
}

