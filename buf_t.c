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

/* Abort on error */
static char          g_abort_on_err = 0;

/* Flags: set on every new buffer */
static buf_t_flags_t g_flags;

#define TRY_ABORT() do{ if(g_abort_on_err) {DE("Abort in %s +%d\n", __FILE__, __LINE__);abort();} } while(0)

buf_t_flags_t buf_save_flags()
{
	return (g_flags);
}

void buf_restore_flags(buf_t_flags_t flags)
{
	g_flags = flags;
}


void buf_set_abort(void)
{
	DDD("buf_t: enabled 'abort on error' state\n");
	g_abort_on_err = 1;
}

void buf_unset_abort(void)
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
static error_t buf_set_flag(buf_t *buf, buf_t_flags_t f)
{
	TESTP(buf, EINVAL);
	buf->flags |= f;
	return (EOK);
}

/* Clear flag(s) of the buf */
static error_t buf_rm_flag(buf_t *buf, buf_t_flags_t f)
{
	TESTP(buf, EINVAL);
	buf->flags &= ~f;
	return (EOK);
}

/***** Set of function to add flag to buffer */

error_t buf_mark_string(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_STRING));
}

error_t buf_mark_ro(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_READONLY));
}

error_t buf_mark_compresed(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_COMPRESSED));
}

error_t buf_mark_encrypted(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_ENCRYPTED));
}

error_t buf_mark_canary(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_CANARY));
}

error_t buf_mark_crc(buf_t *buf)
{
	return (buf_set_flag(buf, BUF_T_CRC));
}

/***** Set of function to remove flag from buffer */

error_t buf_unmark_string(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_STRING));
}

error_t buf_unmark_ro(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_READONLY));
}

error_t buf_unmark_compresed(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_COMPRESSED));
}

error_t buf_unmark_encrypted(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_ENCRYPTED));
}

error_t buf_unmark_canary(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_CANARY));
}

error_t buf_unmark_crc(buf_t *buf)
{
	return (buf_rm_flag(buf, BUF_T_CRC));
}

/***** CANARY: Protect the buffer */

/* Set canary word in the end of the buf
 * If buf has 'BUF_T_CANARY' flag set, it means
 * that extra space for canary pattern is reserved
 * in the end of the buf->data
 */
error_t buf_set_canary(buf_t *buf)
{
	buf_t_canary_t canary;
	buf_t_canary_t *canary_p;
	TESTP(buf, EINVAL);
	if (!IS_BUF_CANARY(buf)) {
		DE("The buffer doesn't have CANARY flag\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	canary = BUF_T_CANARY_CHAR_PATTERN;
	canary_p = (buf_t_canary_t *)(buf->data + buf->room);
	memcpy(canary_p, &canary, BUF_T_CANARY_SIZE);

	/* Test that the canary pattern set */
	if (0 != memcmp(canary_p, &canary, BUF_T_CANARY_SIZE)) {
		DE("Can't set CANARY\n");
		TRY_ABORT();
		return (ECANCELED);
	}
	return (EOK);
}

/* This function will add canary after existing buffer
 * and add CANARY flag. The buffer room will be decreased by 1.
 * If buf->used == buf->room, the ->used as well be decreased.
 * If this buffer contains string, i.e.flag BUF_T_STRING is set,
 * a '\0' will be added before canary 
 */
error_t buf_force_canary(buf_t *buf)
{
	TESTP(buf, EINVAL);

	if (buf->used < BUF_T_CANARY_SIZE) {
		DE("Buffer is to small for CANARY word\n");
		TRY_ABORT();
		return (ECANCELED);

	}

	if (buf->used == buf->room) {
		buf->used -= BUF_T_CANARY_SIZE;
	}

	buf->room -= BUF_T_CANARY_SIZE;
	return (buf_set_canary(buf));
}

/* If CANARY (a pattern after the buf->data) enabled we test its integrity */
error_t buf_test_canary(buf_t *buf)
{
	buf_t_canary_t canary = BUF_T_CANARY_CHAR_PATTERN;
	TESTP(buf, EINVAL);

	/* */
	if (!IS_BUF_CANARY(buf)) {
		return (ECANCELED);
	}

	if (0 == memcmp(buf->data + buf->room, &canary, BUF_T_CANARY_SIZE)) {
		return (EOK);
	}

	DE("The buf CANARY word is wrong, expected: %X, current: %X\n", BUF_T_CANARY_CHAR_PATTERN, (unsigned int)*(buf->data + buf->room));

	TRY_ABORT();
	return (EBAD);
}

/* Extract canary word from the buf */
buf_t_canary_t buf_get_canary(buf_t *buf)
{
	buf_t_canary_t *canary_p;
	TESTP(buf, (buf_t_canary_t)-1);
	if (!IS_BUF_CANARY(buf)) {
		DE("The buffer doesn't have canary flag\n");
		return (EOK);
	}

	//memcpy(&canary, buf->data + buf->room, BUF_T_CANARY_SIZE);
	canary_p = (buf_t_canary_t *)(buf->data + buf->room);
	return (*canary_p);
}

void buf_print_flags(buf_t *buf)
{
	if (IS_BUF_STRING(buf)) DDD("Buffer is STRING\n");
	if (IS_BUF_RO(buf)) DDD("Buffer is READONLY\n");
	if (IS_BUF_COMPRESSED(buf)) DDD("Buffer is COMPRESSED\n");
	if (IS_BUF_ENCRYPTED(buf)) DDD("Buffer is ENCRYPTED\n");
	if (IS_BUF_CANARY(buf)) DDD("Buffer is CANARY\n");
	if (IS_BUF_CRC(buf)) DDD("Buffer is CRC\n");
}

/* Validate sanity of buf_t */
error_t buf_is_valid(buf_t *buf)
{
	if (NULL == buf) {
		DE("Invalid: got NULL pointer\n");
		TRY_ABORT();
		return (EINVAL);
	}

	/* buf->used always <= buf->room */
	if (buf->used > buf->room) {
		DE("Invalid buf: buf->used > buf->room\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	/* The buf->data may be NULL if and only if both buf->used and buf->room == 0; However, we don't
	   check buf->used: we tested that it <= buf->room already */
	if (NULL == buf->data && buf->room > 0) {
		DE("Invalid buf: buf->data == NULL but buf->room > 0 (%d)\n", buf->room);
		TRY_ABORT();
		return (ECANCELED);
	}

	/* And vice versa: if buf->data != NULL the buf->room must be > 0 */
	if (NULL != buf->data && 0 == buf->room) {
		DE("Invalid buf: buf->data != NULL but buf->room == 0\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	/* If the buf is string than room must be > used */
	if (IS_BUF_STRING(buf) && (NULL != buf->data) && (buf->room <= buf->used)) {
		DE("Invalid STRING buf: buf->used (%d) >= buf->room (%d)\n", buf->used, buf->room);
		TRY_ABORT();
		return (ECANCELED);
	}

	/* For string buffers only: check that the string is null terminated */
	/* If the 'used' area not '\0' terminated - invalid */
	if (IS_BUF_STRING(buf) && (NULL != buf->data) && ('\0' != *(buf->data + buf->used))) {
		DE("Invalid STRING buf: no '0' terminated\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	if (buf->room > 0 && IS_BUF_CANARY(buf) && (EOK != buf_test_canary(buf))) {
		buf_t_canary_t *canary_p = (buf_t_canary_t *)buf->data + buf->room;
		DE("The buffer was overwritten: canary word is wrong\n");
		DE("Expected canary: %X, current canary: %X\n", BUF_T_CANARY_CHAR_PATTERN, *canary_p);
		TRY_ABORT();
		return (ECANCELED);
	}

	/* In Read-Only buffer buf->room must be == bub->used */
	if (IS_BUF_RO(buf) && (buf->room != buf->used)) {
		DE("Warning: in Read-Only buffer buf->used (%d) != buf->room (%d)\n", buf->used, buf->room);
		TRY_ABORT();
		return (ECANCELED);
	}

	DDD0("Buffer is valid\n");
	//buf_print_flags(buf);
	return (EOK);
}

int buf_is_string(buf_t *buf)
{
	TESTP(buf, EINVAL);
	if (IS_BUF_STRING(buf)) {
		return (EOK);
	}
	return (1);
}

/*@null@*/ buf_t *buf_new(size_t size)
{
	/*@temp@*/buf_t *buf;

	size_t real_size = size;

	buf = zmalloc(sizeof(buf_t));
	if (NULL == buf) {
		TRY_ABORT();
		return (NULL);
	}

	buf_allocs_num_inc();

	buf->flags = g_flags;

	/* If no buffer passed, but size given - allocate new buffer */
	if (size > 0) {

		/* If CANARY is set in global flags add space for CANARY word */
		if (IS_BUF_CANARY(buf)) {
			real_size += BUF_T_CANARY_SIZE;
		}

		buf->data = zmalloc(real_size);
		TESTP_ASSERT(buf->data, "Can't allocate buf->data");
	}

	buf->room = size;
	buf->used = 0;

	/* Set CANARY word */
	if (size > 0 && IS_BUF_CANARY(buf) && EOK != buf_set_canary(buf)) {
		DE("Can't set CANARY word\n");
		buf_free(buf);
		TRY_ABORT();
		return (NULL);
	}

	if (EOK != buf_is_valid(buf)) {
		DE("Buffer is invalid right after allocation!\n");
		buf_free(buf);
		TRY_ABORT();
		return (NULL);
	}

	return (buf);
}

/*@null@*/ buf_t *buf_string(size_t size)
{
	buf_t *buf = NULL;
	buf = buf_new(size);

	if (NULL == buf) {
		DE("buf allocation failed\n");
		TRY_ABORT();
		return (NULL);
	}

	if (EOK != buf_mark_string(buf)) {
		DE("Can't set STRING flag\n");
		abort();
	}
	return (buf);
}

/*@null@*/ buf_t *buf_from_string(/*@null@*/char *str, size_t size_without_0)
{
	buf_t *buf = NULL;
	/* The string must be not NULL */
	TESTP(str, NULL);

	/* Test that the string is '\0' terminated */
	if (*(str + size_without_0) != '\0') {
		DE("String is not null terminated\n");
		TRY_ABORT();
		return (NULL);
	}

	buf = buf_new(0);
	TESTP(buf, NULL);

	if (EOK != buf_set_flag(buf, BUF_T_STRING)) {
		DE("Can't set STRING flag\n");
		buf_free(buf);
		TRY_ABORT();
		return (NULL);
	}
	/* We set string into buffer. The 'room' len contain null terminatior, the 'used' for string
	   doesn't */
	if (EOK != buf_set_data(buf, str, size_without_0 + 1, size_without_0)) {
		DE("Can't set string into buffer\n");
		/* Just in case: Disconnect buffer from the buffer before release it */
		buf->data = NULL;
		buf->used = buf->room = 0;
		buf_free(buf);
		TRY_ABORT();
		return (NULL);
	}

	return (buf);
}

error_t buf_set_data(/*@null@*/buf_t *buf, /*@null@*/char *data, size_t size, size_t len)
{
	TESTP(buf, EINVAL);
	TESTP(data, EINVAL);

	/* Don't replace data in read-only buffer */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to replace data in Read-Only buffer\n");
		TRY_ABORT();
		return (EACCES);
	}

	buf->data = data;
	buf->room = size;
	buf->used = len;

	/* If external data set we clean CANRY flag */
	/* TODO: Don't do it. Just realloc the buffer to set CANARY in the end */
	/* TODO: Also flag STATIC should be tested */
	buf_unmark_canary(buf);

	return (EOK);
}

/* Set data into read-only buffer: no changes allowed after that */
error_t buf_set_data_ro(buf_t *buf, char *data, size_t size)
{
	int rc;
	TESTP(buf, EINVAL);

	if (NULL == data && size > 0) {
		DE("Wrong arguments: data == NULL but size > 0 (%zu)\n", size);
		return (ECANCELED);
	}

	rc = buf_set_data(buf, data, size, size);
	if (EOK != rc) {
		DE("Can't set data\n");
		return (rc);
	}

	buf_unmark_canary(buf);
	buf_mark_ro(buf);
	return (EOK);
}

/*@null@*/void *buf_steal_data(/*@null@*/buf_t *buf)
{
	/*@temp@*/void *data;
	TESTP(buf, NULL);
	data = buf->data;
	buf->data = NULL;
	buf->room = 0;
	buf->used = 0;

	/* TODO: If CANARY used - zero it, dont reallocate the buffer */
	return (data);
}

/*@null@*/void *buf_2_data(/*@null@*/buf_t *buf)
{
	void *data;
	TESTP(buf, NULL);
	data = buf_steal_data(buf);
	if (EOK != buf_free(buf)) {
		DE("Warning! Memory leak: can't clean buf_t!");
		TRY_ABORT();
	}
	return (data);
}

error_t buf_add_room(/*@null@*/buf_t *buf, size_t size)
{
	void   *tmp   = NULL;
	size_t canary = 0;

	if (NULL == buf || 0 == size) {
		DE("Bad arguments: buf == NULL (%p) or size == 0 (%zu)\b", buf, size);
		TRY_ABORT();
		return (EINVAL);
	}

	/* We don't free read-only buffer's data */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried add room to Read-Only buffer\n");
		TRY_ABORT();
		return (EACCES);
	}

	if (IS_BUF_CANARY(buf)) {
		canary = BUF_T_CANARY_SIZE;
	}

	tmp = realloc(buf->data, buf->room + size + canary);

	/* Case 1: realloc can't reallocate */
	if (NULL == tmp) {
		DE("Realloc failed\n");
		TRY_ABORT();
		return (ENOMEM);
	}

	/* Case 2: realloc succidded, new memory returned */
	/* No need to clean the old memory - done by realloc */
	if (NULL != tmp) {
		buf->data = tmp;
	}

	/* Clean newely allocated memory */
	memset(buf->data + buf->room, 0, size + canary);

	/* Case 3: realloc succidded, the same pointer - we do nothing */
	/* <Beeep> */

	buf->room += size;

	/* If the buffer use canary add it to the end */

	if (IS_BUF_CANARY(buf) && EOK != buf_set_canary(buf)) {
		DE("Can't set CANARY\b");
		TRY_ABORT();
		return (ENOKEY);
	}

	BUF_TEST(buf);
	return (EOK);
}

error_t buf_test_room(/*@null@*/buf_t *buf, size_t expect)
{
	if (NULL == buf) {
		DE("Got NULL\n");
		TRY_ABORT();
		return (EINVAL);
	}

	if (expect == 0) {
		DE("'expected' size == 0\n");
		TRY_ABORT();
		return (EINVAL);
	}

	if (buf->used + expect <= buf->room) {
		return (EOK);
	}

	return (buf_add_room(buf, expect));
}

error_t buf_clean(/*@only@*//*@null@*/buf_t *buf)
{
	TESTP(buf, EINVAL);

	if (EOK != buf_is_valid(buf)) {
		DE("Warning: buffer is invalid\n");
	}

	/* We don't free read-only buffer's data */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to free Read Only buffer\n");
		TRY_ABORT();
		return (EACCES);
	}

	if (buf->data) {
		/* Security: zero memory before it freed */
		memset(buf->data, 0, buf->room);
		free(buf->data);
	}
	buf->used = buf->room = 0;
	buf->flags = 0;

	return (EOK);
}

error_t buf_free(/*@only@*//*@null@*/buf_t *buf)
{
	TESTP(buf, EINVAL);

	if (EOK != buf_is_valid(buf)) {
		DE("Warning: buffer is invalid\n");
	}

	/* We don't free read-only buffer's data */
	if (IS_BUF_RO(buf)) {
		DE("Warning: tried to free Read Only buffer\n");
		TRY_ABORT();
		return (EACCES);
	}

	if (EOK != buf_clean(buf)) {
		DE("Can't clean buffer, stopped operation, returning error\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	TFREE_SIZE(buf, sizeof(buf_t));
	/* The buffer is released. Write down statistics. */
	buf_release_num_inc();
	return (EOK);
}

error_t buf_add(/*@null@*/buf_t *buf, /*@null@*/const char *new_data, const size_t size)
{
	size_t new_size;
	if (NULL == buf || NULL == new_data || size < 1) {
		/*@ignore@*/
		DE("Wrong argument(s): b = %p, buf = %p, size = %zu\n", buf, new_data, size);
		/*@end@*/
		TRY_ABORT();
		return (EINVAL);
	}

	if (IS_BUF_RO(buf)) {
		DE("Tryed to add data to Read Only buffer\n");
		TRY_ABORT();
		return (EACCES);
	}

	new_size = size;
	/* If this buffer is a string buffer, we should consider \0 after string. If this buffer is empty,
	   we add +1 for the \0 terminator. If the buffer is not empty, we reuse existing \0 terminator */
	if (IS_BUF_STRING(buf) && buf->used == 0) {
		new_size++;
	}

	/* Add room if needed */
	if (0 != buf_test_room(buf, new_size)) {
		DE("Can't add room into buf_t\n");
		TRY_ABORT();
		return (ENOMEM);
	}

	memcpy(buf->data + buf->used, new_data, size);
	buf->used += size;
	BUF_TEST(buf);
	return (EOK);
}

ssize_t buf_used(/*@null@*/buf_t *buf)
{
	/* If buf is invalid we return '-1' costed into uint */
	TESTP(buf, EINVAL);
	return (buf->used);
}

ssize_t buf_room(/*@null@*/buf_t *buf)
{
	/* If buf is invalid we return '-1' costed into uint */
	TESTP(buf, EINVAL);
	return (buf->room);
}

error_t buf_pack(/*@null@*/buf_t *buf)
{
	void   *tmp     = NULL;
	size_t new_size = -1;

	TESTP(buf, EINVAL);

	/*** If buffer is empty we have nothing to do */

	if (NULL == buf->data) {
		return (EOK);
	}

	/*** Sanity check: dont' process invalide buffer */

	if (EOK != buf_is_valid(buf)) {
		DE("Buffer is invalid - can't proceed\n");
		return (ECANCELED);
	}

	/*** Should we really pack it? */
	if (buf->used == buf->room) {
		/* No, we don't need to pack it */
		return (EOK);
	}

	/*** If the buffer is a string, the used should be == (room - 1): after the string we have '\0' */
	if (IS_BUF_STRING(buf) && buf->used == (buf->room - 1)) {
		/* Looks like the buffer should not be packed */
		return (EOK);
	}

	/* Here we shrink the buffer */

	new_size = buf->used;
	if (IS_BUF_CANARY(buf)) {
		new_size += BUF_T_CANARY_SIZE;
	}

	/* If the buffer a string - keep 1 more for '\0' terminator */
	if (IS_BUF_STRING(buf)) {
		new_size += 1;
	}

	tmp = realloc(buf->data, new_size);

	/* Case 1: realloc can't reallocate */
	if (NULL == tmp) {
		DE("Realloc failed\n");
		return (ENOMEM);
	}

	/* Case 2: realloc succeeded, new memory returned */
	/* No need to free the old memory - done by realloc */
	if (NULL != tmp) {
		buf->data = tmp;
	}

	buf->room = buf->used;

	if (IS_BUF_STRING(buf)) {
		buf->room++;
	}

	if (IS_BUF_CANARY(buf)) {
		buf_set_canary(buf);
	}

	/* Here we are if buf->used == buf->room */
	BUF_TEST(buf);
	return (EOK);
}

/* Experimantal: Try to set the buf used size automatically */
/* It can be useful if we copied manualy a string into buf_t and we want to update 'used' of the
   buf_t*/
error_t buf_detect_used(/*@null@*/buf_t *buf)
{
	int used;
	TESTP(buf, EINVAL);

	if (buf_is_valid(buf)) {
		DE("Buffer is invalid, can't proceed\n");
		return (ECANCELED);
	}

	/* If the buf is empty - return with error */
	if (buf->room == 0) {
		DE("Tryed to detect used in empty buffer?\n");
		TRY_ABORT();
		return (ECANCELED);
	}

	used = buf->room;
	/* Run from tail to the beginning of the buffer */

	/* TODO: Replace this with binary search */
	while (used > 0) {
		/* If found not null in the buffer... */
		if (0 != buf->data[used] ) {
			/* Set buf->used as 'used + 1' to keep finished \0 */
			/* If used > room - we fix it later */
			buf->used = used + 1;
			break;
		}
	}

	/* Ir can happen if the buffer is full; in this case after while() the buf->used should be
	   buf->room + 1 */
	/* TODO: STRING buffer, CANARY */
	if (buf->used > buf->room) {
		buf->used = buf->room;
	}

	return (EOK);
}

#if 0
err_t buf_str_concat(buf_t *buf, ...){
	va_list p_str;
	va_start(p_str, nHowMany);
	for (int i = 0; i < nHowMany; i++) nSum += va_arg(p_str, char *);
	va_end(intArgumentPointer);

}
#endif

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

buf_t *buf_sprintf(const char *format, ...)
{
	va_list args;
	buf_t   *buf = NULL;
	int     rc   = -1;
	TESTP(format, NULL);

	/* Create buf_t with reserved room for the string */
	buf = buf_string(0);
	TESTP(buf, NULL);

	va_start(args, format);
	/* Measure string lengh */
	rc = vsnprintf(NULL, 0, format, args);
	va_end(args);

	DDD("Measured string size: it is %d\n", rc);

	/* Allocate buffer: we need +1 for final '\0' */
	rc = buf_add_room(buf, rc + 1);

	if (EOK != rc) {
		DE("Can't add room to buf\n");
		if (buf_free(buf)) {
			DE("Warning, can't free buf_t, possible memory leak\n");
		}
		return (NULL);
	}
	va_start(args, format);
	rc = vsnprintf(buf->data, buf->room, format, args);
	va_end(args);

	if (rc < 0) {
		DE("Can't print string\n");
		if (EOK != buf_free(buf)) {
			DE("Warning, can't free buf_t, possible memory leak\n");
		}
		return (NULL);
	}

	buf->used = buf->room - 1;
	if (EOK != buf_is_valid(buf)) {
		DE("Buffer is invalid - free and return\n");
		TRY_ABORT();
		buf_free(buf);

		return (NULL);
	}

	return (buf);
}

/* Receive from socket; add to the end of the buf; return number of received bytes */
ssize_t buf_recv(buf_t *buf, const int socket, const size_t expected, const int flags)
{
	int     rc       = EBAD;
	ssize_t received = -1;

	TESTP(buf, EINVAL);

	/* Test that we have enough room in the buffer */
	rc = buf_test_room(buf, expected);

	if (EOK != rc) {
		DE("Can't allocate enough room in buf\n");
		TRY_ABORT();
		return (ENOMEM);
	}

	received = recv(socket, buf->data + buf->used, expected, flags);
	if (received > 0) {
		buf->used += received;
	}

	return (received);
}

#if 0
buf_t *buf_dirname(buf_t *filename){
	TESTP(filename, NULL);
	/* Validate that 'filename' buffer is correct and at least bull terminated */
}
#endif
