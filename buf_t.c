/** @file */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <stddef.h>
#include <linux/errno.h>

#include "buf_t.h"
#include "buf_t_string.h"

#include "buf_t_stats.h"
#include "buf_t_debug.h"
#include "buf_t_memory.h"

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
ret_t buf_set_flag(buf_t *buf, buf_t_flags_t f)
{
	T_RET_ABORT(buf, -EINVAL);
	buf->flags |= f;
	return (OK);
}

/* Clear flag(s) of the buf */
ret_t buf_rm_flag(buf_t *buf, buf_t_flags_t f)
{
	T_RET_ABORT(buf, -EINVAL);
	buf->flags &= ~f;
	return (OK);
}

/***** Set of functions to add a flag to the buffer */

ret_t buf_mark_string(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_STRING));
}

ret_t buf_mark_ro(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_READONLY));
}

ret_t buf_mark_compresed(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_COMPRESSED));
}

ret_t buf_mark_encrypted(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_ENCRYPTED));
}

ret_t buf_mark_canary(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_CANARY));
}

ret_t buf_mark_crc(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_set_flag(buf, BUF_T_CRC));
}

/***** Set of functions to remove a flag from the buffer */

ret_t buf_unmark_string(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_STRING));
}

ret_t buf_unmark_ro(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_READONLY));
}

ret_t buf_unmark_compressed(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_COMPRESSED));
}

ret_t buf_unmark_encrypted(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_ENCRYPTED));
}

ret_t buf_unmark_canary(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_CANARY));
}

ret_t buf_unmark_crc(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	return (buf_rm_flag(buf, BUF_T_CRC));
}

/***** CANARY: Protect the buffer *****/

/* Set canary word in the end of the buf
 * If buf has 'BUF_T_CANARY' flag set, it means
 * that extra space for canary pattern is reserved
 * in the end of the buf->data
 */
ret_t buf_set_canary(buf_t *buf)
{
	buf_t_canary_t canary;
	buf_t_canary_t *canary_p;
	T_RET_ABORT(buf, -EINVAL);
	if (!IS_BUF_CANARY(buf)) {
		DE("The buffer doesn't have CANARY flag\n");
		TRY_ABORT();
		/*@ignore@*/
		return (-ECANCELED);
		/*@end@*/
	}

	canary = BUF_T_CANARY_CHAR_PATTERN;
	canary_p = (buf_t_canary_t *)(buf->data + buf_room(buf));
	memcpy(canary_p, &canary, BUF_T_CANARY_SIZE);

	/* Test that the canary pattern set */
	if (0 != memcmp(canary_p, &canary, BUF_T_CANARY_SIZE)) {
		DE("Can't set CANARY\n");
		TRY_ABORT();
		return (-ECANCELED);
	}
	return (OK);
}

buf_usize_t buf_used(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

	/* In case it is a circ buffer, */
	if (OK == buf_is_circ(buf)) {
		DD("The buffer is CIRC\n");
		if (buf->ht.head <= buf->ht.tail) {
			return (buf->ht.head - buf->ht.tail);
		}
		return (buf_room(buf) - (buf->ht.head - buf->ht.tail));
	}

	return (buf->used);
}

ret_t buf_set_used(buf_t *buf, buf_usize_t used)
{
	T_RET_ABORT(buf, -EINVAL);

	/* In case it is a circ buffer, */
	if (OK == buf_is_circ(buf)) {
		DE("Can not set ->used for CIRC fuffer - use 'buf_set_head_tail()' instead\n");
		abort();
	}

	buf->used = used;
	return OK;
}

ret_t buf_inc_used(buf_t *buf, buf_usize_t inc)
{
	T_RET_ABORT(buf, -EINVAL);

	/* In case it is a circ buffer, */
	if (OK == buf_is_circ(buf)) {
		DE("Can not add to ->used for CIRC fuffer - use 'buf_add_head_tail()' instead\n");
		abort();
	}
	buf->used += inc;
	return OK;
}

ret_t buf_dec_used(buf_t *buf, buf_usize_t dec)
{
	T_RET_ABORT(buf, -EINVAL);

	/* In case it is a circ buffer, */
	if (OK == buf_is_circ(buf)) {
		DE("Can not add to ->used for CIRC fuffer - use 'buf_add_head_tail()' instead\n");
		abort();
	}

	if (dec > buf_used(buf)) {
		DE("Can not decrement buf->used: the dec > buf->used (%ld > %ld)\n",
		   dec, buf_used(buf));
		return BAD;
	}

	if (buf->used >= dec) {
		buf->used -= dec;
	}
	return OK;
}

/* This function will add canary bits after an existing buffer
 * and add the CANARY flag. The buffer room will be decreased by 1 (byte).
 * If buf->used == buf->room, the ->used be decreased as well.
 * If this buffer contains a string, i.e.type is BUF_T_STRING,
 * a '\0' will be added before the canary bits
 */
ret_t buf_force_canary(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

	if (buf_used(buf) < BUF_T_CANARY_SIZE) {
		DE("Buffer is to small for CANARY word\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	if (buf_used(buf) == buf_room(buf)) {
		if (OK != buf_dec_used(buf, BUF_T_CANARY_SIZE)) {
			DE("Can not decrement a buffer used value");
			return (BAD);
		}
	}

	if (OK != buf_dec_room(buf, BUF_T_CANARY_SIZE)) {
		DE("Can not decrement a buffer room value\n");
		return (BAD);
	}
	return (buf_set_canary(buf));
}

/* If CANARY (a pattern after the buf->data) enabled we test its integrity */
ret_t buf_test_canary(buf_t *buf)
{
	buf_t_canary_t canary = BUF_T_CANARY_CHAR_PATTERN;
	T_RET_ABORT(buf, -EINVAL);

	/* */
	if (!IS_BUF_CANARY(buf)) {
		return (-ECANCELED);
	}

	if (0 == memcmp(buf->data + buf_room(buf), &canary, BUF_T_CANARY_SIZE)) {
		return (OK);
	}

	DE("The buf CANARY word is wrong, expected: %X, current: %X\n", BUF_T_CANARY_CHAR_PATTERN, (unsigned int)*(buf->data + buf_room(buf)));

	TRY_ABORT();
	return (BAD);
}

/* Extract canary word from the buf */
buf_t_canary_t buf_get_canary(buf_t *buf)
{
	buf_t_canary_t *canary_p;
	T_RET_ABORT(buf, (buf_t_canary_t)-1);
	if (!IS_BUF_CANARY(buf)) {
		DE("The buffer doesn't have canary flag\n");
		return (OK);
	}

	//memcpy(&canary, buf->data + buf->room, BUF_T_CANARY_SIZE);
	canary_p = (buf_t_canary_t *)(buf->data + buf_room(buf));
	return (*canary_p);
}

void buf_print_flags(buf_t *buf)
{
	if (IS_BUF_STRING(buf)) DDD("Buffer is STRING\n");
	if (IS_BUF_BIT(buf)) DDD("Buffer is BIT\n");
	if (IS_BUF_CIRC(buf)) DDD("Buffer is CIRC\n");
	if (IS_BUF_RO(buf)) DDD("Buffer is READONLY\n");
	if (IS_BUF_COMPRESSED(buf)) DDD("Buffer is COMPRESSED\n");
	if (IS_BUF_ENCRYPTED(buf)) DDD("Buffer is ENCRYPTED\n");
	if (IS_BUF_CANARY(buf)) DDD("Buffer is CANARY\n");
	if (IS_BUF_CRC(buf)) DDD("Buffer is CRC\n");
}

/* Validate sanity of buf_t - common for all buffers */
static ret_t buf_common_is_valid(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

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

	if (buf_room(buf) > 0 && IS_BUF_CANARY(buf) && (OK != buf_test_canary(buf))) {
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
	return (OK);
}

ret_t buf_is_valid(buf_t *buf)
{
	ret_t ret;
	ret = buf_common_is_valid(buf);
	if (OK != ret) {
		DE("Buffer is invalud - returning/aborting");
		TRY_ABORT();
		return ret;
	}

	switch (BUF_TYPE(buf)) {
	case BUF_T_RAW:
		/* For RAW buffer no additional test needed */
		return OK;
		break;
	case BUF_T_STRING:
		return buf_str_is_valid(buf);
		break;
	default:
		DE("Unknown buffer type: %d\n", BUF_TYPE(buf));
		TRY_ABORT();
		return BAD;
	}

	return OK;
}

int buf_is_string(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	if (IS_BUF_STRING(buf)) {
		return (OK);
	}
	return (1);
}

int buf_is_bit(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	if (IS_BUF_BIT(buf)) {
		return (OK);
	}
	return (1);
}

int buf_is_circ(buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);
	if (IS_BUF_CIRC(buf)) {
		return (OK);
	}
	return (1);
}

/*@null@*/ buf_t *buf_new(buf_usize_t size)
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
	if (OK != buf_set_room(buf, size)) {
		DE("Can not set a new 'room' value\n");
		free(buf);
		return (NULL);
	}

	if (OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value\n");
		free(buf);
		return (NULL);
	}

	/* Set CANARY word */
	if (size > 0 && IS_BUF_CANARY(buf) && OK != buf_set_canary(buf)) {
		DE("Can't set CANARY word\n");
		if (OK != buf_free(buf)) {
			DE("Can't free the buffer\n");
		}
		TRY_ABORT();
		return (NULL);
	}

	if (OK != buf_is_valid(buf)) {
		DE("Buffer is invalid right after allocation!\n");
		if (OK != buf_free(buf)) {
			DE("Can not free the buffer\n");
		}
		TRY_ABORT();
		return (NULL);
	}

	return (buf);
}

ret_t buf_set_data(/*@null@*/buf_t *buf, /*@null@*/char *data, const buf_usize_t size, const buf_usize_t len)
{
	T_RET_ABORT(buf, -EINVAL);
	T_RET_ABORT(data, -EINVAL);

	/* Don't replace data in read-only buffer */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to replace data in Read-Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	buf->data = data;
	if (OK != buf_set_room(buf, size)) {
		DE("Can not set a new value to the buffer\n");
		return (BAD);
	}

	if (OK != buf_set_used(buf, len)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (BAD);
	}

	/* If external data set we clean CANRY flag */
	/* TODO: Don't do it. Just realloc the buffer to set CANARY in the end */
	/* TODO: Also flag STATIC should be tested */
	if (OK != buf_unmark_canary(buf)) {
		DE("Can not unset canary flag\n");
		return (BAD);
	}

	return (OK);
}

/* Set data into read-only buffer: no changes allowed after that */
ret_t buf_set_data_ro(buf_t *buf, char *data, buf_usize_t size)
{
	int rc;
	T_RET_ABORT(buf, -EINVAL);

	if (NULL == data && size > 0) {
		DE("Wrong arguments: data == NULL but size > 0 (%lu)\n", size);
		return (-ECANCELED);
	}

	rc = buf_set_data(buf, data, size, size);
	if (OK != rc) {
		DE("Can't set data\n");
		return (rc);
	}

	if (OK != buf_unmark_canary(buf)) {
		DE("Can not unset CANARY flag\n");
		return (BAD);
	}

	if (OK != buf_mark_ro(buf)) {
		DE("Can not set RO flag\n");
		return (BAD);
	}
	return (OK);
}

/*@null@*/void *buf_steal_data(/*@null@*/buf_t *buf)
{
	/*@temp@*/void *data;
	T_RET_ABORT(buf, NULL);
	data = buf->data;
	buf->data = NULL;
	if (OK != buf_set_room(buf, 0)) {
		DE("Can not set a new value to the buffer\n");
		return (data);
	}

	if (OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (data);
	}
	/* TODO: If CANARY used - zero it, dont reallocate the buffer */
	return (data);
}

/*@null@*/void *buf_2_data(/*@null@*/buf_t *buf)
{
	void *data;
	T_RET_ABORT(buf, NULL);
	data = buf_steal_data(buf);
	if (OK != buf_free(buf)) {
		DE("Warning! Memory leak: can't clean buf_t!");
		TRY_ABORT();
	}
	return (data);
}

ret_t buf_add_room(/*@null@*/buf_t *buf, buf_usize_t size)
{
	void   *tmp   = NULL;
	size_t canary = 0;

	T_RET_ABORT(buf, -EINVAL);

	if (0 == size) {
		DE("Bad arguments: buf == NULL (%p) or size == 0 (%lu)\b", buf, size);
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

	//BUF_DUMP(buf);
	tmp = realloc(buf->data, buf_room(buf) + size + canary);

	/* Case 1: realloc can't reallocate */
	if (NULL == tmp) {
		DE("Realloc failed\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	/* Case 2: realloc succidded, new memory returned */
	/* No need to clean the old memory - done by realloc */
	if (NULL != tmp) {
		buf->data = tmp;
	}

	/* Clean newely allocated memory */
	memset(buf->data + buf_room(buf), 0, size + canary);

	/* Case 3: realloc succidded, the same pointer - we do nothing */
	/* <Beeep> */

	if (OK != buf_inc_room(buf, size)) {
		DE("Can not increment 'room' value\n");
		return (BAD);
	}

	/* If the buffer use canary add it to the end */

	if (IS_BUF_CANARY(buf) && OK != buf_set_canary(buf)) {
		DE("Can't set CANARY\b");
		TRY_ABORT();
		/*@ignore@*/
		return (-ENOKEY);
		/*@end@*/
	}

	BUF_TEST(buf);
	return (OK);
}

ret_t buf_test_room(/*@null@*/buf_t *buf, buf_usize_t expect)
{
	T_RET_ABORT(buf, -EINVAL);

	if (expect == 0) {
		DE("'expected' size == 0\n");
		TRY_ABORT();
		return (-EINVAL);
	}

	if (buf_used(buf) + expect <= buf_room(buf)) {
		return (OK);
	}

	return (buf_add_room(buf, expect));
}

ret_t buf_clean(/*@only@*//*@null@*/buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

	if (OK != buf_is_valid(buf)) {
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
	if (OK != buf_set_room(buf, 0)) {
		DE("Can not set a new value to the buffer\n");
		return (BAD);
	}

	if (OK != buf_set_used(buf, 0)) {
		DE("Can not set a new 'used' value to the buffer\n");
		return (BAD);
	}

	buf->flags = 0;

	return (OK);
}

ret_t buf_free(/*@only@*//*@null@*/buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

	if (OK != buf_is_valid(buf)) {
		DE("Warning: buffer is invalid\n");
	}

	/* We can not release the data of read-only buffer */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to free Read Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	if (OK != buf_clean(buf)) {
		DE("Can't clean buffer, stopped operation, returning error\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	TFREE_SIZE(buf, sizeof(buf_t));
	/* The buffer is released. Write down statistics. */
	buf_release_num_inc();
	return (OK);
}

ret_t buf_add(/*@null@*/buf_t *buf, /*@null@*/const char *new_data, const buf_usize_t size)
{
	size_t new_size;
	TESTP_ASSERT(buf, "buf is NULL");
	TESTP_ASSERT(buf, "data is NULL");
	if (size < 1) {
		/*@ignore@*/
		DE("Wrong argument(s): b = %p, buf = %p, size = %lu\n", buf, new_data, size);
		/*@end@*/
		TRY_ABORT();
		return (-EINVAL);
	}

	if (IS_BUF_RO(buf)) {
		DE("Tryed to add data to Read Only buffer\n");
		TRY_ABORT();
		return (-EACCES);
	}

	/* Here we switch to a type-specific function */
	switch (BUF_TYPE(buf)) {
	case BUF_T_STRING:
		return buf_str_add(buf, new_data, size);
	}

	new_size = size;

	/* Add room if needed */
	if (0 != buf_test_room(buf, new_size)) {
		DE("Can't add room into buf_t\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	memcpy(buf->data + buf_used(buf), new_data, size);
	if (OK != buf_inc_used(buf, size)) {
		DE("Can not increment 'used' of a budder\n");
		return (BAD);
	}

	BUF_TEST(buf);
	return (OK);
}

buf_usize_t buf_room(/*@null@*/buf_t *buf)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -EINVAL);
	return (buf->room);
}

ret_t buf_set_room(/*@null@*/buf_t *buf, buf_usize_t room)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -EINVAL);
	buf->room = room;
	return OK;
}

ret_t buf_inc_room(/*@null@*/buf_t *buf, buf_usize_t inc)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -EINVAL);
	buf->room += inc;
	return (OK);
}

ret_t buf_dec_room(/*@null@*/buf_t *buf, buf_usize_t dec)
{
	/* If buf is invalid we return '-1' costed into uint */
	T_RET_ABORT(buf, -EINVAL);
	if (dec > buf_room(buf)) {
		DE("Can'r decrement the room: dec > buf->room (%ld > %ld)\n", dec, buf_room(buf));
		return BAD;
	}

	buf->room -= dec;
	return (OK);
}

ret_t buf_pack(/*@null@*/buf_t *buf)
{
	void   *tmp     = NULL;
	size_t new_size = -1;

	T_RET_ABORT(buf, -EINVAL);

	switch (BUF_TYPE(buf)) {
	case BUF_T_STRING:
		return buf_str_pack(buf);
	}

	/*** If buffer is empty we have nothing to do */

	if (NULL == buf->data) {
		return (OK);
	}

	/*** Sanity check: dont' process invalide buffer */

	if (OK != buf_is_valid(buf)) {
		DE("Buffer is invalid - can't proceed\n");
		return (-ECANCELED);
	}

	/*** Should we really pack it? */
	if (buf_used(buf) == buf_room(buf)) {
		/* No need to pack it */
		return (OK);
	}

	/* Here we shrink the buffer */

	new_size = buf_used(buf);
	if (IS_BUF_CANARY(buf)) {
		new_size += BUF_T_CANARY_SIZE;
	}

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

	if (OK != buf_set_room(buf, buf_used(buf))) {
		DE("Can not set a new room value to the buffer\n");
		return (BAD);
	}

	if (IS_BUF_CANARY(buf)) {
		if (OK != buf_set_canary(buf)) {
			DE("Can not set CANARY to the buffer\n");
			TRY_ABORT();
			return (BAD);
		}
	}

	/* Here we are if buf->used == buf->room */
	BUF_TEST(buf);
	return (OK);
}

/* Experimental: Try to set the buf used size automatically */
/* It can be useful if we copied manualy a string into buf_t and we want to update 'used' of the
   buf_t*/
ret_t buf_detect_used(/*@null@*/buf_t *buf)
{
	T_RET_ABORT(buf, -EINVAL);

	/* If the buf is empty - return with error */
	if (0 == buf_room(buf)) {
		DE("Tryed to detect used in empty buffer?\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	switch (BUF_TYPE(buf)) {
	case BUF_T_STRING:
		return buf_str_detect_used(buf);
		break;
	default:
		DE("Not supported buf type %d\n", BUF_TYPE(buf));
		return BAD;
	}

	/* We should not get here */
	return (BAD);
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
size_t buf_recv(buf_t *buf, const int socket, const buf_usize_t expected, const int flags)
{
	int     rc       = BAD;
	ssize_t received = -1;

	T_RET_ABORT(buf, -EINVAL);

	/* Test that we have enough room in the buffer */
	rc = buf_test_room(buf, expected);

	if (OK != rc) {
		DE("Can't allocate enough room in buf\n");
		TRY_ABORT();
		return (-ENOMEM);
	}

	received = recv(socket, buf->data + buf_used(buf), expected, flags);
	if (received > 0) {
		if (OK != buf_inc_used(buf, received)) {
			DE("Can not increment 'used'");
			TRY_ABORT();
			return (-EINVAL);
		}
	}

	return (received);
}

