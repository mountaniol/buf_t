/** @file */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/errno.h>

#include "buf_t.h"
#include "buf_t_string.h"
#include "buf_t_array.h"

#include "buf_t_stats.h"
#include "buf_t_debug.h"
#include "buf_t_memory.h"
#include "se_tests.h"

/* Abort on error */
static buf_t_flags_t g_abort_on_err = 0;

int bug_get_abort_flag(void)
{
	return g_abort_on_err;
}

/* Flags: set on every new buffer */
static buf_t_flags_t g_flags;

buf_t_flags_t buf_save_flags(void)
{
	return (g_flags);
}

void buf_restore_flags(buf_t_flags_t flags)
{
	g_flags = flags;
}

void buf_set_abort_flag(void)
{
	DDD("buf_t: enabled 'abort on error' state\n");
	g_abort_on_err = 1;
}

void buf_unset_abort_flag(void)
{
	DDD("buf_t: disabled 'abort on error' state\n");
	g_abort_on_err = 0;
}

/***** Flags section */

/* Set default flags for all further bufs */
void buf_default_flags(buf_t_flags_t f)
{
	g_flags |= f;
}

/* Set flag(s) of the buf */
ret_t buf_set_flag(/*@temp@*//*@in@*/buf_t *buf, buf_t_flags_t f)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	buf->flags |= f;
	return (BUFT_OK);
}

/* Clear flag(s) of the buf */
ret_t buf_rm_flag(/*@temp@*//*@in@*/buf_t *buf, buf_t_flags_t f)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	buf->flags &= ~f;
	return (BUFT_OK);
}

/***** Set of functions to add a flag to the buffer */

ret_t buf_mark_string(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_TYPE_STRING));
}

ret_t buf_mark_array(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_TYPE_ARR));
}

ret_t buf_mark_ro(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_FLAG_READONLY));
}

ret_t buf_mark_compresed(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_FLAG_COMPRESSED));
}

ret_t buf_mark_encrypted(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_FLAG_ENCRYPTED));
}

ret_t buf_mark_canary(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_FLAG_CANARY));
}

ret_t buf_mark_crc(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_set_flag(buf, BUF_T_FLAG_CRC));
}

/***** Set of functions to remove a flag from the buffer */

ret_t buf_unmark_string(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_TYPE_STRING));
}

ret_t buf_unmark_ro(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_FLAG_READONLY));
}

ret_t buf_unmark_compressed(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_FLAG_COMPRESSED));
}

ret_t buf_unmark_encrypted(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_FLAG_ENCRYPTED));
}

ret_t buf_unmark_canary(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_FLAG_CANARY));
}

ret_t buf_unmark_crc(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf_rm_flag(buf, BUF_T_FLAG_CRC));
}

/***** CANARY: Protect the buffer *****/

ret_t buf_has_canary_flag(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
{
	if (IS_BUF_CANARY(buf)) {
		return (BUFT_YES);
	}

	return (BUFT_NO);
}

/* Set canary word in the end of the buf
 * If buf has 'BUF_T_CANARY' flag set, it means
 * that extra space for canary pattern is reserved
 * in the end of the buf->data
 */
ret_t buf_set_canary(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	buf_t_canary_t canary;
	buf_t_canary_t *canary_p;
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (BUFT_NO == buf_has_canary_flag(buf)) {
		DE("The buffer doesn't have CANARY flag\n");
		TRY_ABORT();
		/*@ignore@*/
		return (-BUFT_WRONG_BUF_FLAG);
		/*@end@*/
	}

	canary = BUF_T_CANARY_CHAR_PATTERN;
	canary_p = (buf_t_canary_t *)(buf->data + buf_room(buf));
	memcpy(canary_p, &canary, BUF_T_CANARY_SIZE);

	/* Test that the canary pattern set */
	if (0 != memcmp(canary_p, &canary, BUF_T_CANARY_SIZE)) {
		DE("Can't set CANARY\n");
		TRY_ABORT();
		return (-BUFT_CANNOT_SET_CANARY);
	}
	return (BUFT_OK);
}

buf_s64_t buf_used(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* In case it is a circ buffer */
	if (BUFT_OK == buf_is_circ(buf)) {
		DD("The buffer is CIRC\n");
		if (buf->ht.head <= buf->ht.tail) {
			return (buf->ht.head - buf->ht.tail);
		}
		return (buf_room(buf) - (buf->ht.head - buf->ht.tail));
	}

	/* In case it is an array buffer */
	if (BUFT_OK == buf_is_array(buf)) {
		return buf_arr_used(buf);
	}

	/* In other cases, just return the '->used' field */
	return (buf->used);
}

ret_t buf_set_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t used)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* In case it is a circ buffer */
	if (BUFT_OK == buf_is_circ(buf)) {
		DE("Can not set ->used for CIRC fuffer - use 'buf_set_head_tail()' instead\n");
		abort();
	}

	/* In case it is an array buffer */
	if (BUFT_OK == buf_is_array(buf)) {
		return buf_array_set_used(buf, used);
	}

	buf->used = used;
	return (BUFT_OK);
}

ret_t buf_inc_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t inc)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* In case it is a circ buffer, */
	if (BUFT_OK == buf_is_circ(buf)) {
		DE("Can not add to ->used for CIRC fuffer - use 'buf_add_head_tail()' instead\n");
		TRY_ABORT();
		return (-BUFT_BAD_BUFT_TYPE);
	}

	/* In case of array buffer it is not applicable */
	if (BUFT_OK == buf_is_array(buf)) {
		DE("Can not add to ->used for ARRAY fuffer - use 'buf_add_head_tail()' instead\n");
		TRY_ABORT();
		return (-BUFT_BAD_BUFT_TYPE);
	}

	/* TODO: Detext out of limit situation */
	buf->used += inc;
	return (BUFT_OK);
}

ret_t buf_dec_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t dec)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* In case it is a circ buffer, */
	if (BUFT_OK == buf_is_circ(buf)) {
		DE("Can not add to ->used for CIRC fuffer - use 'buf_add_head_tail()' instead\n");
		TRY_ABORT();
		return (-BUFT_BAD_BUFT_TYPE);
	}

	/* In case of array buffer it is not applicable */
	if (BUFT_OK == buf_is_array(buf)) {
		DE("Can not add to ->used for ARRAY fuffer - use 'buf_add_head_tail()' instead\n");
		TRY_ABORT();
		return (-BUFT_BAD_BUFT_TYPE);
	}

	if (dec > buf_used(buf)) {
		DE("Can not decrement buf->used: the dec > buf->used (%ld > %ld)\n",
		   dec, buf_used(buf));
		return (-BUFT_OUT_OF_LIMIT_OP);
	}

	if (buf->used >= dec) {
		buf->used -= dec;
	}
	return BUFT_OK;
}

/* This function will add canary bits after an existing buffer
 * and add the CANARY flag. The buffer room will be decreased by size of canary.
 * If buf->used == buf->room, the ->used be decreased as well.
 * If this buffer contains a string, i.e.type is BUF_T_STRING,
 * a '\0' will be added before the canary bits
 */
ret_t buf_force_canary(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* In case there is no space fpr CANARY tail,
	   try to increase buffer space to keep CANARY */

	if (buf_used(buf) < (buf_s64_t)BUF_T_CANARY_SIZE ||
		buf_used(buf) == buf_room(buf)) {

		/* Add room to keep CANARY tail */
		buf_add_room(buf, BUF_T_CANARY_SIZE);
	}


	/* If there no space to set CANARY tail, we abort the function */
	if (buf_used(buf) < (buf_s64_t)BUF_T_CANARY_SIZE) {
		DE("Buffer is to small for CANARY word\n");
		TRY_ABORT();
		return (-BUFT_TOO_SMALL);
	}

	/* The buffer is big enough but all allocated data is occupied */
	if (buf_used(buf) == buf_room(buf)) {
		if (BUFT_OK != buf_dec_used(buf, BUF_T_CANARY_SIZE)) {
			DE("Can not decrement a buffer used value");
			return (-BUFT_BAD);
		}
	}

	if (BUFT_OK != buf_dec_room(buf, BUF_T_CANARY_SIZE)) {
		DE("Can not decrement a buffer room value\n");
		return (-BUFT_BAD);
	}
	return (buf_set_canary(buf));
}

/* If CANARY (a pattern after the buf->data) enabled we test its integrity */
ret_t buf_test_canary(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	buf_t_canary_t canary = BUF_T_CANARY_CHAR_PATTERN;
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* */
	if (!IS_BUF_CANARY(buf)) {
		return (-ECANCELED);
	}

	if (0 == memcmp(buf->data + buf_room(buf), &canary, BUF_T_CANARY_SIZE)) {
		return (BUFT_OK);
	}

	DE("The buf CANARY word is wrong, expected: %X, current: %X\n", BUF_T_CANARY_CHAR_PATTERN, (unsigned int)*(buf->data + buf_room(buf)));

	TRY_ABORT();
	return (-BUFT_BAD);
}

/* Extract canary word from the buf */
buf_t_canary_t buf_get_canary(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	buf_t_canary_t *canary_p;
	T_RET_ABORT(buf, -1);
	if (!IS_BUF_CANARY(buf)) {
		DE("The buffer doesn't have canary flag\n");
		return (BUFT_OK);
	}

	//memcpy(&canary, buf->data + buf->room, BUF_T_CANARY_SIZE);
	canary_p = (buf_t_canary_t *)(buf->data + buf_room(buf));
	return (*canary_p);
}

void buf_print_flags(/*@temp@*//*@in@*/buf_t *buf)
{
	if (IS_BUF_TYPE_STRING(buf)) DDD("Buffer is STRING\n");
	if (IS_BUF_TYPE_BIT(buf)) DDD("Buffer is BIT\n");
	if (IS_BUF_TYPE_CIRC(buf)) DDD("Buffer is CIRC\n");
	if (IS_BUF_RO(buf)) DDD("Buffer is READONLY\n");
	if (IS_BUF_COMPRESSED(buf)) DDD("Buffer is COMPRESSED\n");
	if (IS_BUF_ENCRYPTED(buf)) DDD("Buffer is ENCRYPTED\n");
	if (IS_BUF_CANARY(buf)) DDD("Buffer is CANARY\n");
	if (IS_BUF_CRC(buf)) DDD("Buffer is CRC\n");
}

/* Validate sanity of buf_t - common for all buffers */
static ret_t buf_common_is_valid(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* buf->used always <= buf->room */
	/* TODO: not in case of CIRC buffer */
	if (buf_used(buf) > buf_room(buf)) {
		DE("Invalid buf: buf->used > buf->room\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* The buf->data may be NULL if and only if both buf->used and buf->room == 0; However, we don't
	   check buf->used: we tested that it <= buf->room already */
	if ((NULL == buf->data) && (buf_room(buf) > 0)) {
		DE("Invalid buf: buf->data == NULL but buf->room > 0 (%ld)\n", buf_room(buf));
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* And vice versa: if buf->data != NULL the buf->room must be > 0 */
	if ((NULL != buf->data) && (0 == buf_room(buf))) {
		DE("Invalid buf: buf->data != NULL but buf->room == 0\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	if (buf_room(buf) > 0 && IS_BUF_CANARY(buf) && (BUFT_OK != buf_test_canary(buf))) {
		buf_t_canary_t *canary_p = (buf_t_canary_t *)buf->data + buf_room(buf);
		DE("The buffer was overwritten: canary word is wrong\n");
		DE("Expected canary: %X, current canary: %X\n", BUF_T_CANARY_CHAR_PATTERN, *canary_p);
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* In Read-Only buffer buf->room must be == bub->used */
	if (IS_BUF_RO(buf) && (buf_room(buf) != buf_used(buf))) {
		DE("Warning: in Read-Only buffer buf->used (%ld) != buf->room (%ld)\n", buf_used(buf), buf_room(buf));
		TRY_ABORT();
		return (-ECANCELED);
	}

	DDD0("Buffer is valid\n");
	//buf_print_flags(buf);
	return (BUFT_OK);
}

ret_t buf_is_valid(/*@temp@*//*@in@*/buf_t *buf)
{
	ret_t ret;
	ret = buf_common_is_valid(buf);
	if (BUFT_OK != ret) {
		DE("Buffer is invalud - returning/aborting");
		TRY_ABORT();
		return ret;
	}

	switch (BUF_TYPE(buf)) {
	case BUF_T_TYPE_RAW:
		/* For RAW buffer no additional test needed */
		return BUFT_OK;
		break;
	case BUF_T_TYPE_STRING:
		return buf_str_is_valid(buf);
		break;
	case BUF_T_TYPE_ARR:
		return buf_array_is_valid(buf);
		break;
	default:
		DE("Unknown buffer type: %d\n", BUF_TYPE(buf));
		TRY_ABORT();
		return -BUFT_BAD;
	}

	return BUFT_OK;
}

/***** TEST BUFFER TYPE ******/

int buf_is_string(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (IS_BUF_TYPE_STRING(buf)) {
		return (BUFT_YES);
	}
	return (BUFT_NO);
}

int buf_is_array(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (IS_BUF_TYPE_ARR(buf)) {
		return (BUFT_YES);
	}
	return (BUFT_NO);
}

int buf_is_bit(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (IS_BUF_TYPE_BIT(buf)) {
		return (BUFT_YES);
	}
	return (BUFT_NO);
}

int buf_is_circ(/*@temp@*//*@in@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (IS_BUF_TYPE_CIRC(buf)) {
		return (BUFT_YES);
	}
	return (BUFT_NO);
}

/*@only@*//*@in@*/ buf_t *buf_new(buf_s64_t size)
{
	/*@temp@*/buf_t *buf;

	/* The real size of allocated  buffer can be more than used asked,
	   the canary and the checksum space could be added */
	size_t real_size = size;

	buf = zmalloc(sizeof(buf_t));
	T_RET_ABORT(buf, NULL);

	/* Increase statistics of allocated buffers */
	buf_allocs_num_inc();

	/* Set global flags for the start */
	buf->flags = g_flags;

	/* If a size is given than allocate a data */
	if (size > 0) {

		/* If CANARY is set in global flags add space for CANARY word */
		if (IS_BUF_CANARY(buf)) {
			real_size += BUF_T_CANARY_SIZE;
		}

		/* TODO: If CRC flag set, allocale also space for CRC */

		buf->data = zmalloc(real_size);
		TESTP_ASSERT(buf->data, "Can't allocate buf->data");
	}

	/* TODO: in case of CIRC buffer it is wrong */
	if (BUFT_OK != buf_set_room(buf, size)) {
		DE("Can not set a new 'room' value\n");
		free(buf);
		return (NULL);
	}

	if (BUFT_OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value\n");
		free(buf);
		return (NULL);
	}

	/* Set CANARY word */
	if (size > 0 && IS_BUF_CANARY(buf) && BUFT_OK != buf_set_canary(buf)) {
		DE("Can't set CANARY word\n");
		if (BUFT_OK != buf_free(buf)) {
			DE("Can't free the buffer\n");
		}
		TRY_ABORT();
		return (NULL);
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		DE("Buffer is invalid right after allocation!\n");
		if (BUFT_OK != buf_free(buf)) {
			DE("Can not free the buffer\n");
		}
		TRY_ABORT();
		return (NULL);
	}

	return (buf);
}

ret_t buf_set_data(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@null@*/ /*@only@*/ /*@in@*/char *data, const buf_s64_t size, const buf_s64_t len)
/*@sets buf->data@*/
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	T_RET_ABORT(data, -BUFT_NULL_POINTER);

	/* Don't replace data in read-only buffer */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to replace data in Read-Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	buf->data = data;
	if (BUFT_OK != buf_set_room(buf, size)) {
		DE("Can not set a new value to the buffer\n");
		return (-BUFT_BAD);
	}

	if (BUFT_OK != buf_set_used(buf, len)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (-BUFT_BAD);
	}

	/* If external data set we clean CANRY flag */
	/* TODO: Don't do it. Just realloc the buffer to set CANARY in the end */
	/* TODO: Also flag STATIC should be tested */
	if (BUFT_OK != buf_unmark_canary(buf)) {
		DE("Can not unset canary flag\n");
		return (-BUFT_BAD);
	}

	return (BUFT_OK);
}

/* Set data into read-only buffer: no changes allowed after that */
ret_t buf_set_data_ro(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@null@*//*@only@*//*@in@*/char *data, buf_s64_t size)
/*@sets buf->data@*/
{
	int rc;
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	if (NULL == data && size > 0) {
		DE("Wrong arguments: data == NULL but size > 0 (%lu)\n", size);
		return (-ECANCELED);
	}

	rc = buf_set_data(buf, data, size, size);
	if (BUFT_OK != rc) {
		DE("Can't set data\n");
		return (rc);
	}

	if (BUFT_OK != buf_unmark_canary(buf)) {
		DE("Can not unset CANARY flag\n");
		return (-BUFT_BAD);
	}

	if (BUFT_OK != buf_mark_ro(buf)) {
		DE("Can not set RO flag\n");
		return (-BUFT_BAD);
	}
	return (BUFT_OK);
}

/*@null@*//*@only@*/void *buf_steal_data(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	/*@temp@*/void *data;
	T_RET_ABORT(buf, NULL);
	data = buf->data;
	buf->data = NULL;
	if (BUFT_OK != buf_set_room(buf, 0)) {
		DE("Can not set a new value to the buffer\n");
		return (data);
	}

	if (BUFT_OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (data);
	}
	/* TODO: If CANARY used - zero it, dont reallocate the buffer */
	return (data);
}

/*@null@*/void *buf_2_data(/*@only@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
/*@releases buf@*/
{
	void *data;
	T_RET_ABORT(buf, NULL);
	data = buf_steal_data(buf);
	if (BUFT_OK != buf_free(buf)) {
		DE("Warning! Memory leak: can't clean buf_t!");
		TRY_ABORT();
	}
	return (data);
}

/* TODO: add an 'err' aviable, this function can return NULL if there no data, but also returns NULL on an error */
void /*@temp@*//*@null@*/ *buf_data(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, NULL);
	return (buf->data);
}

ret_t buf_data_is_null(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (NULL == buf->data) {
		return BUFT_YES;
	}
	return BUFT_NO;
}

static ret_t buf_realloc(/*@temp@*//*@in@*//*@special@*/buf_t *buf, size_t new_size)
/*@allocates buf->data@*/
{
	void   *tmp;
#ifdef S_SPLINT_S
	tmp = zmalloc(buf_room(buf) + new_size);
	memcpy(tmp, buf_data(buf), buf_room(buf));
#else
	tmp = realloc(buf->data, new_size);
#endif

	/* Case 1: realloc can't reallocate */
	if (NULL == tmp) {
		DE("Realloc failed\n");
		return (-ENOMEM);
	}

	/* Case 2: realloc succeeded, new memory returned */
	/* No need to free the old memory - done by realloc */
	if (NULL != tmp) {
#ifdef S_SPLINT_S
		free(buf->data);
#endif
		buf->data = tmp;
	}

	return BUFT_OK;
}

ret_t buf_add_room(/*@temp@*//*@in@*//*@special@*/buf_t *buf, buf_s64_t size)
/*@allocates buf->data@*/
/*@uses buf->data@*/
{
	size_t canary = 0;

	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	if (0 == size) {
		DE("Bad arguments: buf == NULL (%p) or size == 0 (%lu)\b", (void *)buf, size);
		TRY_ABORT();
		return (-EINVAL);
	}

	/* We don't free read-only buffer's data */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried add room to Read-Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	if (IS_BUF_CANARY(buf)) {
		canary = BUF_T_CANARY_SIZE;
	}

	if (BUFT_OK != buf_realloc(buf, buf_room(buf) + size + canary)) {
		DE("Can not reallocate buf->data\n");
		return (-ENOMEM);
	}

	/* Clean newely allocated memory */
	memset(buf->data + buf_room(buf), 0, size + canary);

	/* Case 3: realloc succeeded, the same pointer - we do nothing */
	/* <Beeep> */

	if (BUFT_OK != buf_inc_room(buf, size)) {
		DE("Can not increment 'room' value\n");
		return (-BUFT_BAD);
	}

	/* If the buffer use canary add it to the end */

	if (IS_BUF_CANARY(buf) && BUFT_OK != buf_set_canary(buf)) {
		DE("Can't set CANARY\b");
		TRY_ABORT();
		/*@ignore@*/
		return (-ENOKEY);
		/*@end@*/
	}

	BUF_TEST(buf);
	return (BUFT_OK);
}

ret_t buf_test_room(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t expect)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	if (expect == 0) {
		DE("'expected' size == 0\n");
		TRY_ABORT();
		return (-EINVAL);
	}

	if (buf_used(buf) + expect <= buf_room(buf)) {
		return (BUFT_OK);
	}

	return (buf_add_room(buf, expect));
}

ret_t buf_clean(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@releases buf->data@*/
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	if (BUFT_OK != buf_is_valid(buf)) {
		DE("Warning: buffer is invalid\n");
	}

	/* We don't free read-only buffer's data */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to free Read Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	if (buf->data) {
		/* Security: zero memory before it freed */
		memset(buf->data, 0, buf_room(buf));
		free(buf->data);
	}
	if (BUFT_OK != buf_set_room(buf, 0)) {
		DE("Can not set a new value to the buffer\n");
		return (-BUFT_BAD);
	}

	if (BUFT_OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (-BUFT_BAD);
	}

	buf->flags = 0;

	return (BUFT_OK);
}

ret_t buf_free(/*@only@*//*@in@*//*@special@*/buf_t *buf)
/*@releases buf->data@*/
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	if (BUFT_OK != buf_is_valid(buf)) {
		DE("Warning: buffer is invalid\n");
	}

	/* We can not release the data of read-only buffer */
	if (0 != IS_BUF_RO(buf)) {
		DE("Warning: tried to free Read Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	if (BUFT_OK != buf_clean(buf)) {
		DE("Can't clean buffer, stopped operation, returning error\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	TFREE_SIZE(buf, sizeof(buf_t));
	/* The buffer is released. Write down statistics. */
	buf_release_num_inc();
	return (BUFT_OK);
}

/*
 * buf can not be NULL
 * new_data can not be NULL 				  *
 *
	*/
ret_t buf_add(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@temp@*//*@in@*/const char *new_data, const buf_s64_t size)
/*@defines buf->data@*/
/*@uses buf->data@*/
{
	size_t new_size;
	TESTP_ASSERT(buf, "buf is NULL");
	TESTP_ASSERT(new_data, "buf is NULL");
	if (size < 1) {
		/*@ignore@*/
		DE("Wrong argument(s): b = %p, buf = %p, size = %lu\n", (void *)buf, (void *)new_data, size);
		/*@end@*/
		TRY_ABORT();
		return (-EINVAL);
	}

	if (0 != IS_BUF_RO(buf)) {
		DE("Tryed to add data to Read Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	/* Here we switch to a type-specific function */
	switch (BUF_TYPE(buf)) {
	case BUF_T_TYPE_STRING:
		return buf_str_add(buf, new_data, size);
	}

	new_size = size;

	/* Add room if needed */
	if (0 != buf_test_room(buf, new_size)) {
		DE("Can't add room into buf_t\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	/*@ignore@*/
	memcpy(buf->data + buf_used(buf), new_data, size);
	/*@end@*/
	if (BUFT_OK != buf_inc_used(buf, size)) {
		DE("Can not increment 'used' of a budder\n");
		return (-BUFT_BAD);
	}

	BUF_TEST(buf);
	return (BUFT_OK);
}

buf_s64_t buf_room(/*@temp@*//*@in@*/buf_t *buf)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	return (buf->room);
}

ret_t buf_set_room(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t room)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	buf->room = room;
	return BUFT_OK;
}

ret_t buf_inc_room(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t inc)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	buf->room += inc;
	return (BUFT_OK);
}

ret_t buf_dec_room(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t dec)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	if (dec > buf_room(buf)) {
		DE("Can'r decrement the room: dec > buf->room (%ld > %ld)\n", dec, buf_room(buf));
		return -BUFT_BAD;
	}

	buf->room -= dec;
	return (BUFT_OK);
}

ret_t buf_pack(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	size_t new_size = -1;

	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	switch (BUF_TYPE(buf)) {
	case BUF_T_TYPE_STRING:
		return buf_str_pack(buf);
	}

	/*** If buffer is empty we have nothing to do */

	if (NULL == buf->data) {
		return (BUFT_OK);
	}

	/*** Sanity check: dont' process invalide buffer */

	if (BUFT_OK != buf_is_valid(buf)) {
		DE("Buffer is invalid - can't proceed\n");
		return (-ECANCELED);
	}

	/*** Should we really pack it? */
	if (buf_used(buf) == buf_room(buf)) {
		/* No need to pack it */
		return (BUFT_OK);
	}

	/* Here we shrink the buffer */
	new_size = buf_used(buf);
	if (0 != IS_BUF_CANARY(buf)) {
		new_size += BUF_T_CANARY_SIZE;
	}

	if (BUFT_OK != buf_realloc(buf, new_size)) {
		DE("Can not realloc buf->data\n");
		return (-BUFT_BAD);
	}

	if (BUFT_OK != buf_set_room(buf, buf_used(buf))) {
		DE("Can not set a new room value to the buffer\n");
		return (-BUFT_BAD);
	}

	if (0 != IS_BUF_CANARY(buf)) {
		if (BUFT_OK != buf_set_canary(buf)) {
			DE("Can not set CANARY to the buffer\n");
			TRY_ABORT();
			return (-BUFT_BAD);
		}
	}

	/* Here we are if buf->used == buf->room */
	BUF_TEST(buf);
	return (BUFT_OK);
}

/* Experimental: Try to set the buf used size automatically */
/* It can be useful if we copied manualy a string into buf_t and we want to update 'used' of the
   buf_t*/
ret_t buf_detect_used(/*@temp@*//*@in@*//*@special@*/buf_t *buf)
/*@uses buf->data@*/
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* If the buf is empty - return with error */
	if (0 == buf_room(buf)) {
		DE("Tryed to detect used in empty buffer?\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	switch (BUF_TYPE(buf)) {
	case BUF_T_TYPE_STRING:
		return buf_str_detect_used(buf);
	default:
		DE("Not supported buf type %d\n", BUF_TYPE(buf));
	}

	/* We should not get here */
	return (-BUFT_BAD);
}

/* 
 * Extract from buffer field 'number'.
 * 'delims' is set of delimiters, each delimeter 1 character.
 * 'skip' is a pattern to skip after delimeter.
 * 
 * Example:
 * 
 * "Value : red  "
 * 
 * If we want to extract "red": delims=":"
 * skip=" "
 * In this case this funciton finds ":", and skip all " " after it.
 * 
 */

#if 0
buf_t *buf_extract_field(buf_t *buf, const char *delims, const char *skip, size_t number){
	int pos = 0;
	int end = -1;
	int num = 0;
	TESTP(buf, NULL);
	TESTP(delims, NULL);

	/* Find start position */
	while (pos < buf->used) {
		if (buf->data[pos] ) {}
	}
}
#endif

/* Receive from socket; add to the end of the buf; return number of received bytes */
size_t buf_recv(/*@temp@*//*@in@*//*@special@*/buf_t *buf, const int socket, const buf_s64_t expected, const int flags)
/*@uses buf->data@*/
{
	int     rc       = -BUFT_BAD;
	ssize_t received = -1;

	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that we have enough room in the buffer */
	rc = buf_test_room(buf, expected);

	if (BUFT_OK != rc) {
		DE("Can't allocate enough room in buf\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	received = recv(socket, buf->data + buf_used(buf), expected, flags);
	if (received > 0) {
		if (BUFT_OK != buf_inc_used(buf, received)) {
			DE("Can not increment 'used'");
			TRY_ABORT();
			return (-EINVAL);
		}
	}

	return (received);
}

buf_t *buf_from_file(const char *filename)
{
	int         fd;
	buf_t       *buf;
	struct stat st;
	int         rv;


	T_RET_ABORT(filename, NULL);

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		DE("can't open file fir reading: %s\n", filename);
		return (NULL);
	}

	if (0 != fstat(fd, &st)) {
		DE("Can't stat file %s\n", filename);
		close(fd);
		return (NULL);
	}

	buf = buf_new(st.st_size);
	TESTP(buf, NULL);

	rv = read(fd, buf->data, buf->room);
	if (rv < 0 || (buf_s64_t)rv != buf->room) {
		DE("Error on file read: asked %lu, read %d\n", buf->room, rv);
		close(fd);
		buf_free(buf);
		return (NULL);
	}

	buf->used = rv;
	close(fd);
	return (buf);
}

int buf_to_file(buf_t *buf, buf_t *file, mode_t mode)
{
	int fd;
	int rv = -BUFT_BAD;

	T_RET_ABORT(buf, -BUFT_NULL_POINTER);
	T_RET_ABORT(file, -BUFT_NULL_POINTER);
	T_RET_ABORT(file->data, -BUFT_NULL_POINTER);

	fd = open(file->data, O_WRONLY);
	if (fd < 0) {
		DE("can't open file fir reading: %s\n", file->data);
		return (-BUFT_BAD);
	}

	rv = write(fd, buf->data, buf->room);

	if (rv < 0 || (buf_s64_t)rv != buf->room) {
		DE("Error on file writinf: asked %lu, read %d\n", buf->room, rv);
		rv = -BUFT_BAD;
	} else rv = BUFT_OK;

	if (mode != 0) {
		if (0 != fchmod(fd, mode)) {
			DE("Can't chmod file %s\n", file->data);
		}
	}

	close(fd);
	return (rv);
}

