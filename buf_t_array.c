/** @file */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <stddef.h>

#include "buf_t.h"
#include "buf_t_types.h"
#include "buf_t_structs.h"
#include "buf_t_stats.h"
#include "buf_t_debug.h"
#include "buf_t_memory.h"

//#define BUF_DEBUG 1

/*** Interface to buf_t internals ***/

/* Get number of members in buf_t */
buf_s32_t buf_arr_members(buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that this is an array buffer */
	if (BUFT_NO == buf_is_array(buf)) {
		DE("Buffer is not an array buffer");
		TRY_ABORT();
		return (-ECANCELED);
	}

	return buf->arr.members;
}

/* Set number of members in buf_t */
ret_t buf_set_arr_members(buf_t *buf, buf_s32_t new_members)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that this is an array buffer */
	if (BUFT_NO == buf_is_array(buf)) {
		DE("Buffer is not an array buffer");
		TRY_ABORT();
		return (-ECANCELED);
	}

	if (new_members < 0) {
		DE("A negative value of new_members: %d; aborted\n", new_members);
		TRY_ABORT();
		return (-ECANCELED);
	}

	buf->arr.members = new_members;
	return BUFT_OK;
}

/* Get size of a single member in buf_t */
buf_s32_t buf_arr_member_size(buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that this is an array buffer */
	if (BUFT_NO == buf_is_array(buf)) {
		DE("Buffer is not an array buffer");
		TRY_ABORT();
		return (-ECANCELED);
	}

	return buf->arr.size;
}

/* Set a size of a single member in buf_t */
ret_t buf_set_arr_member_size(buf_t *buf, buf_s32_t new_size)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that this is an array buffer */
	if (BUFT_NO == buf_is_array(buf)) {
		DE("Buffer is not an array buffer");
		TRY_ABORT();
		return (-ECANCELED);
	}

	buf->arr.size = new_size;
	return BUFT_OK;
}

/* Calculate 'used' space int the buf */
buf_s64_t buf_arr_used(buf_t *buf)
{
	buf_s64_t ret_members     = 0;
	buf_s64_t ret_member_size = 0;

	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* Test that this is an array buffer;
	   if it is not, cancel the operation */
	if (BUFT_NO == buf_is_array(buf)) {
		DE("Buffer is not an array buffer");
		TRY_ABORT();
		return (-ECANCELED);
	}

	ret_members = buf_arr_members(buf);
	if (ret_members < 0) {
		DE("Could not take number of membrs in array\n");
		TRY_ABORT();
		return ret_members;
	}

	ret_member_size = buf_arr_member_size(buf);
	if (ret_member_size < 0) {
		DE("Could not take size of one of member in array\n");
		TRY_ABORT();
		return ret_member_size;
	}

	return (ret_members * ret_member_size);
}

/* Set 'used' for the array in case of manipulation;
   in this case we recalculate number of members in the array */
ret_t buf_arr_set_used(buf_t *buf, buf_s64_t new_used)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* We can execute this operation only when we know the size of a single member */
	if (buf->arr.size < 1) {
		DE("Can not set 'used' for buf ARRARY if a member size is not set\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* Test that the new size is aligned with the size of a single memner */
	if (0 != (new_used % buf->arr.size)) {
		DE("The new 'used' for the buffer (%ld) is not aligned with member size (%d): modulo (%ld)\n",
		   new_used, buf->arr.size, (new_used % buf->arr.size));
		TRY_ABORT();
		return (-ECANCELED);
	}

	buf->arr.members = (new_used / buf->arr.size);
	return BUFT_OK;
}

/* Validate sanity of the buffer */
ret_t buf_array_is_valid(/*@in@*//*@temp@*/buf_t *buf)
{
	T_RET_ABORT(buf, -BUFT_NULL_POINTER);

	/* TEST: The type must be ARRAY */
	if (BUFT_OK != buf_is_array(buf)) {
		DE("Buffer is not an array type\n");
		TRY_ABORT();
		return (-ECANCELED);
	}


	/* TEST: If buf->data is NULL, the 'room' and 'usage' can not be > 0 */
	if (BUFT_YES == buf_data_is_null(buf)) {
		/* Test buf->room is 0 */
		if (buf_room(buf) > 0) {
			DE("buf->data is NULL buf buf->room > 0\n");
			TRY_ABORT();
			return (-ECANCELED);
		}

		/* Test buf->used is 0 */
		if (buf_used(buf) > 0) {
			DE("buf->data is NULL buf buf->used > 0\n");
			TRY_ABORT();
			return (-ECANCELED);
		}
	}

	/* TEST: If the buffer is ARR, and number of memners > 0, the size of a memner must be > 0 */
	if (buf->arr.members > 0 && buf->arr.size == 0) {
		DE("buf->arr.members > 0 && buf->arr.size == 0\n");
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* TEST: ->data is not NULL, than neither arr.amount not arr.size could be < 0 */
	if (BUFT_NO == buf_data_is_null(buf)) {
		if (buf_arr_members(buf) < 0) {
			DE("Value of 'members' < 0");
			TRY_ABORT();
			return (-ECANCELED);
		}

		if (buf_arr_member_size(buf) < 0) {
			DE("Value of 'member size' < 0");
			TRY_ABORT();
			return (-ECANCELED);
		}
	}

	DDD0("Buffer is valid\n");
	//buf_print_flags(buf);
	return (BUFT_OK);
}

/* We can set used if it 0 or it multiples of size of one memnber */
ret_t buf_array_set_used(/*@in@*//*@temp@*/buf_t *buf, buf_s64_t used)
{
	if (0 == used) {
		buf->arr.members = buf->arr.size = 0;
		return BUFT_OK;
	}

	if (0 == (used % buf->arr.size)) {
		buf->arr.members = used / buf->arr.size;
		return BUFT_OK;
	}

	/* Else we have an error */
	DE("Can not set 'used' of the array: it is not multiples of size of one memnber: asked to sed %ld, member size %d, modulo is %ld\n",
	   used, buf->arr.size, (used % buf->arr.size));
	TRY_ABORT();
	return (-BUFT_BAD);

}

/* Allocate new buf_t of type 'array' */
/*@null@*/ buf_t *buf_array(buf_s32_t member_size, buf_s32_t num_of_members)
{
	buf_t *buf = buf_new(member_size * num_of_members);

	T_RET_ABORT(buf, NULL);

	if (BUFT_OK != buf_mark_array(buf)) {
		DE("Can't set ARRAY flag\n");
		abort();
	}

	buf->arr.size = member_size;
	buf->arr.members = 0;
	return (buf);
}

/* Add several members into the array */
ret_t buf_arr_add_members(buf_t *buf, const void *new_data_ptr, const buf_s32_t num_of_new_members)
{
	buf_s64_t offset;

	T_RET_ABORT(buf, -ECANCELED);
	T_RET_ABORT(new_data_ptr, -ECANCELED);

	/* Number of element to copy must be > 0 value */
	if (num_of_new_members < 1) {
		DE("Illegal number of new elements, < 1 (%d)", num_of_new_members);
		TRY_ABORT();
		return -ECANCELED;
	}

	/* Let's calulate a new room: Get the current room... */
	buf_s64_t new_room = buf_arr_used(buf);
	if (new_room < 0) {
		DE("Could not calculate used memory in a buffer\n");
		return -ECANCELED;
	}

	/* ... and add space for new elements, every element has a known size */
	new_room += buf->arr.size * num_of_new_members;

	/* Add room if needed: buf_test_room() adds room if needed */
	if (BUFT_OK != buf_test_room(buf, new_room)) {
		DE("Can't add room to buf_t: old room is %ld, asked room is %ld\n",
		   buf_arr_used(buf), new_room);
		TRY_ABORT();
		return (-ENOMEM);
	}

	/* Copy new data into the buffer */
	offset = buf->arr.members * buf->arr.size;
	memcpy(buf->data + offset, new_data_ptr, buf->arr.size * num_of_new_members);

	/* Increase number of elements in the array */
	buf->arr.members += num_of_new_members;
	return BUFT_OK;
}

/* Add one memner */
ret_t buf_arr_add(buf_t *buf, const void *new_data_ptr)
{
	return buf_arr_add_members(buf, new_data_ptr, 1);
}

/* Remove members from buffer: from 'from_memeber' (inclusive), number of members */
ret_t buf_arr_rm_members(buf_t *buf, const buf_s32_t from_member, const buf_s32_t num_of_new_members)
{
	TESTP_BUF_DATA(buf);

	/* The start member must be > 0 */
	if (from_member < 0) {
		DE("The from_member < 0: %d\n", num_of_new_members);
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* The num_of_new_members of member to remove must be > 0 */
	if (num_of_new_members < 1) {
		DE("The num_of_new_members < 1: %d\n", num_of_new_members);
		TRY_ABORT();
		return (-ECANCELED);
	}


	/* Test we have enough members to delete */
	if (buf->arr.members - from_member < num_of_new_members) {
		DE("Asked to delete more members than possible: members in array %d, askeed to delete from %d amount %d\n",
		   buf->arr.members, from_member, num_of_new_members);
		TRY_ABORT();
		return (-ECANCELED);
	}

	/* Now we move all members to the new position */


	/* Size of memory to me removed */
	size_t memory_to_remove_size = buf->arr.size * num_of_new_members;

	/* Move to: the memory where the first member to remove */
	char   *move_to              = buf->data + (from_member * buf->arr.size);

	/* Move from: The first member after the memory section to be removed,
	   which is the first member to remove + size of memory of all member to remove */
	char   *move_from            = move_to + memory_to_remove_size;

	/* Move memory */
	memmove(move_to, move_from, memory_to_remove_size);

	/* Decrease number of members in the array */
	buf->arr.members -= num_of_new_members;
	return BUFT_OK;
}

ret_t buf_arr_rm(buf_t *buf, const buf_s32_t member_index)
{
	return buf_arr_rm_members(buf, member_index, 1);
}

void *buf_arr_member_ptr(buf_t *buf, const buf_s32_t member_index)
{
	T_RET_ABORT(buf, NULL);
	T_RET_ABORT(buf->data, NULL);

	if (member_index > buf->arr.members) {
		DE("Asked a index (%d) > buf->arr.members (%d)\n", member_index, buf->arr.members);
		TRY_ABORT();
		return NULL;
	}

	if (member_index < 0) {
		DE("Asked a index < 0 : %d\n", member_index);
		TRY_ABORT();
		return NULL;
	}

	return buf->data + (buf->arr.size * member_index);
}

ret_t buf_arr_member_copy(buf_t *buf, const buf_s32_t member_index, void *dest, buf_s32_t dest_memory_size)
{
	TESTP_BUF_DATA(buf);
	T_RET_ABORT(dest, -ECANCELED);

	if (dest_memory_size < buf->arr.size) {
		DE("Destination beffer not large enough: %d, require at least %d\n",
		   dest_memory_size, buf->arr.size);
		TRY_ABORT();
		return -ECANCELED;
	}

	void *ptr = buf_arr_member_ptr(buf, member_index);
	if (NULL == ptr) {
		DE("Can not get pointer of asked memner\n");
		TRY_ABORT();
		return -BUFT_BAD;
	}

	memcpy(dest, ptr, buf->arr.size);
	return BUFT_OK;
}

