#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "buf_t.h"
#include "buf_t_string.h"
#include "buf_t_stats.h"
#include "buf_t_debug.h"

int verbose = 0;

#define PRINT(fmt, ...) do{if(verbose > 0){printf("%s +%d : ", __func__, __LINE__); printf(fmt, ##__VA_ARGS__);} }while(0 == 1)
#define PSPLITTER()  do{if(verbose > 0)printf("+++++++++++++++++++++++++++++++++++++++++++++++\n\n");} while(0)
#define PSPLITTER2()  do{if(verbose > 0) printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");} while(0)
#define PSTART(x)     do{if(verbose > 0) printf("Beginning:   [%s]\n", x);} while(0)
#define PSTEP(x)      do{if(verbose > 0) printf("Passed step: [%s] +%d\n", x, __LINE__);} while(0)
#define PSUCCESS(x) do{PSPLITTER2();printf("PASS:        [%s]\n", x);} while(0)
#define PFAIL(x)   do{PSPLITTER2(); printf("FAIL:        [%s] [line +%d]\n", x, __LINE__);} while(0)

/* Create buffer with 0 size data */
void test_buf_new_zero_size(void)
{
	buf_t *buf = NULL;
	PSPLITTER();

	PSTART("allocate 0 size buffer");
	buf = buf_new(0);
	if (NULL == buf) {
		PFAIL("Cant allocate 0 size buf");
		abort();
	}

	PSTEP("Buffer allocated with 0 room size");

	if (buf_used(buf) != 0 || buf_room(buf) != 0) {
		printf("0 size buffer: used (%ld) or room (%ld) != 0\n", buf_used(buf), buf_room(buf));
		PFAIL("0 size buffer");
		abort();
	}

	PSTEP("Tested: buf->used == 0 and buf->room == 0");

	if (buf->data != NULL) {
		printf("0 size buffer: data != NULL (%p)\n", buf->data);
		PFAIL("0 size buffer");
		abort();
	}

	PSTEP("Tested: buf->data == NULL");

	if (OK != buf_free(buf)) {
		PFAIL("Can not free the buffer");
		abort();
	}
	PSTEP("Released buf");
	PSUCCESS("allocate 0 size buffer");
}

/* Create buffers with increasing size */
void test_buf_new_increasing_size(void)
{
	buf_t     *buf = NULL;
	uint64_t size = 64;
	int       i;
	PSPLITTER();

	PSTART("increasing size buffer");
	for (i = 1; i < 16; i++) {
		size = size << 1;

		buf = buf_new(size);
		if (NULL == buf) {
			PFAIL("increasing size buffer");
			/*@ignore@*/
			printf("Tried to allocate: %zu size\n", size);
			/*@end@*/
			abort();
		}

		if ((uint64_t)buf_used(buf) != 0 || (uint64_t)buf_room(buf) != size) {
			/*@ignore@*/
			printf("increasing size buffer: used (%ld) !=0 or room (%ld) != %zu\n", buf_used(buf), buf_room(buf), size);
			/*@end@*/
			PFAIL("increasing size buffer");
			abort();
		}

		if (NULL == buf->data) {
			/*@ignore@*/
			printf("increasing size buffer: data == NULL (%p), asked size: %zu, iteration: %d\n", buf->data, size, i);
			/*@end@*/
			PFAIL("increasing size buffer");
			abort();
		}

		if (OK != buf_free(buf)) {
			PFAIL("Can not free the buffer");
			abort();
		}
	}

	/*@ignore@*/
	PRINT("[Allocated up to %zu bytes buffer]\n", size);
	/*@end@*/

	PSUCCESS("increasing size buffer");
}

void test_buf_string(size_t buffer_init_size)
{
	buf_t      *buf  = NULL;
	const char *str  = "Jabala Labala Hoom";
	const char *str2 = " Lalala";

	PSPLITTER();

	PSTART("buf_string");

	/*@ignore@*/
	PRINT("[Asked string size: %zu]\n", buffer_init_size);
	/*@end@*/

	buf = buf_string(buffer_init_size);
	if (NULL == buf) {
		PFAIL("buf_string: Can't allocate buf");
		abort();
	}

	PSTEP("Allocated buffer");

	if (OK != buf_add(buf, str, strlen(str))) {
		PFAIL("buf_string: can't add");
		abort();
	}

	PSTEP("Adding str");

#if 0
	if (buf->used != (buf->room - 1)) {
		printf("[After buf_add: wrong buf->used or buf->room]\n");
		printf("[buf->used = %d, buf->room = %d]\n", buf->used, buf->room);
		printf("[bif->used should be = (buf->room - 1)]\n");
		PFAIL("buf_string failed");
		abort();
	}
#endif

	if (strlen(buf->data) != strlen(str)) {
		/*@ignore@*/
		printf("[After buf_add: wrong string len of buf->data]\n");
		printf("[Added string len = %zu]\n", strlen(str));
		printf("[buf->data len = %zu]\n", strlen(buf->data));
		/*@end@*/
		PFAIL("buf_string");
		abort();
	}

#if 0
	printf("After string: |%s|, str: |%s|\n", buf->data, str);
	printf("After first string: buf->room = %d, buf->used = %d\n", buf->room, buf->used);
#endif

	PSTEP("Tested str added");

	if (OK != buf_add(buf, str2, strlen(str2))) {
		printf("[Can't add string into buf]\n");
		PFAIL("buf_string");
		abort();
	}

	if (buf_used(buf) != (buf_s64_t)strlen(str) + (buf_s64_t)strlen(str2)) {
		/*@ignore@*/
		printf("After buf_add: wrong buf->used\n");
		printf("Expected: buf->used = %zu\n", strlen(str) + strlen(str2));
		printf("Current : buf->used = %ld\n", buf_used(buf));
		printf("str = |%s| len = %zu\n", str, strlen(str));
		printf("str2 = |%s| len = %zu\n", str2, strlen(str2));
		/*@end@*/

		PFAIL("buf_string");
		abort();
	}

	if (strlen(buf->data) != (strlen(str) + strlen(str2))) {
		/*@ignore@*/
		printf("[buf->used != added strings]\n");
		printf("[buf->used = %zu, added strings len = %zu]\n", strlen(buf->data), strlen(str) + strlen(str2));
		printf("[String is: |%s|, added strings: |%s%s|]\n", buf->data, str, str2);
		printf("str = |%s| len = %zu\n", str, strlen(str));
		printf("str2 = |%s| len = %zu\n", str2, strlen(str2));
		/*@end@*/
		PFAIL("buf_string");
		abort();
	}

	//printf("%s\n", buf->data);
	if (OK != buf_free(buf)) {
		PFAIL("buf_string: Can not free the buffer");
		abort();
	}

	PSUCCESS("buf_string");
}

/* Allocate string buffer. Add several strings. Pack it. Test that after the packing the buf is
   correct. Test that the string in the buffer is correct. */
void test_buf_pack_string(void)
{
	buf_t      *buf     = NULL;
	const char *str     = "Jabala Labala Hoom";
	const char *str2    = " Lalala";
	char       *con_str = NULL;
	buf_s64_t  len;
	buf_s64_t  len2;

	PSPLITTER();

	PSTART("buf_pack_string");

	buf = buf_string(1024);
	if (NULL == buf) {
		PFAIL("buf_string: Can't allocate buf");
		abort();
	}

	PSTEP("Allocated buffer");

	len = strlen(str);

	if (OK != buf_add(buf, str, len)) {
		PFAIL("buf_pack_string: can't add");
		abort();
	}

	PSTEP("Adding str");

	if (strlen(buf->data) != strlen(str)) {
		PFAIL("buf_pack_string");
		abort();
	}

	PSTEP("Tested str added");


	len2 = strlen(str2);
	if (OK != buf_add(buf, str2, len2)) {
		printf("[Can't add string into buf]\n");
		PFAIL("buf_pack_string");
		abort();
	}

	if (buf_used(buf) != (len + len2)) {
		PFAIL("buf_pack_string");
		abort();
	}

	if ((buf_s64_t)strlen(buf->data) != (len + len2)) {
		PFAIL("buf_pack_string");
		abort();
	}

	/* Now we pack the buf */
	if (OK != buf_pack(buf)) {
		PFAIL("buf_pack_string");
		abort();
	}

	/* Test that the packed buffer has the right size */
	if (buf_used(buf) != (len + len2)) {
		DE("buf_used(buf) [%lu] != len + len2 [%lu]\n", buf_used(buf), (len + len2));
		PFAIL("buf_pack_string");
		abort();
	}

	/* Test that buf->room = buf->used + 1 */
	if (buf_used(buf) != buf_room(buf) - 1) {
		DE("buf used [%lu] != buf_room + 1 [%lu]\n", buf_used(buf), buf_room(buf));
		PFAIL("buf_pack_string");
		abort();
	}

	con_str = malloc(len + len2 + 1);
	if (NULL == con_str) {
		printf("Error: can't allocate memory\n");
		abort();
	}

	memset(con_str, 0, len + len2 + 1);
	snprintf(con_str, len + len2 + 1, "%s%s", str, str2);

	if (0 != strcmp(buf->data, con_str)) {
		PFAIL("buf_pack_string: Strings are differ");
		abort();
	}
	//printf("%s\n", buf->data);
	if (OK != buf_free(buf)) {
		PFAIL("buf_pack_string: Can npt free the buffer");
		abort();
	}
	free(con_str);

	PSUCCESS("buf_pack_string");
}

/* Allocate string buffer. Add several strings. Pack it. Test that after the packing the buf is
   correct. Test that the string in the buffer is correct. */
void test_buf_str_concat(void)
{
	buf_t      *buf1    = NULL;
	buf_t      *buf2    = NULL;
	const char *str1    = "Jabala Labala Hoom";
	const char *str2    = " Lalala";
	char       *con_str = NULL;
	buf_s64_t  len1;
	buf_s64_t  len2;

	len1 = strlen(str1);
	len2 = strlen(str2);

	PSPLITTER();

	PSTART("buf_str_concat");

	buf1 = buf_string(0);
	if (NULL == buf1) {
		PFAIL("buf_str_concat: Can't allocate buf1");
		abort();
	}

	if (OK != buf_add(buf1, str1, len1)) {
		PFAIL("buf_str_concat: Can't add string into buf2");
		abort();
	}

	if (OK != buf_is_string(buf1)) {
		PFAIL("buf_str_concat: buf1 is not a string buffer");
		abort();
	}

	PSTEP("Allocated buf1");

	buf2 = buf_string(0);
	if (NULL == buf2) {
		PFAIL("buf_str_concat: Can't allocate buf2");
		abort();
	}

	if (OK != buf_add(buf2, str2, len2)) {
		PFAIL("buf_str_concat: Can't add string into buf2");
		abort();
	}

	if (OK != buf_is_string(buf2)) {
		PFAIL("buf_str_concat: buf2 is not a string buffer");
		abort();
	}

	PSTEP("Allocated buf2");

	if (OK != buf_str_concat(buf1, buf2)) {
		PFAIL("buf_str_concat: buf_str_concat returned an error");
		abort();
	}

	PSTEP("buf_str_concat OK");

	if ((buf_s64_t)strlen(buf1->data) != (len1 + len2)) {
		PFAIL("buf_str_concat: bad length");
		abort();
	}

	PSTEP("string length match 1");

	/* Test that the packed buffer has the right size */
	if (buf_used(buf1) != (len1 + len2)) {
		DE("buf_used(buf) [%lu] != len + len2 [%lu]\n", buf_used(buf1), (len1 + len2));
		PFAIL("buf_str_concat: wrong buf_used()");
		abort();
	}

	PSTEP("string length match 2");

	/* Test that buf->room = buf->used + 1 */
	if (buf_used(buf1) != buf_room(buf1) - 1) {
		DE("buf used [%lu] != buf_room + 1 [%lu]\n", buf_used(buf1), buf_room(buf1));
		PFAIL("buf_str_concat: buf_used(buf1) != buf_room(buf1) - 1");
		abort();
	}

	PSTEP("buf_used, buf_room OK");

	con_str = malloc(len1 + len2 + 1);
	if (NULL == con_str) {
		PFAIL("buf_str_concat: can't allocate memory\n");
		abort();
	}

	memset(con_str, 0, len1 + len2 + 1);
	snprintf(con_str, len1 + len2 + 1, "%s%s", str1, str2);

	if (0 != strcmp(buf1->data, con_str)) {
		PFAIL("buf_str_concat: string is not the same");
		abort();
	}

	PSTEP("strings compared OK");

	if (OK != buf_free(buf1)) {
		PFAIL("buf_str_concat: can't free buf1");
		abort();
	}

	if (OK != buf_free(buf2)) {
		PFAIL("buf_str_concat: can't free buf2");
		abort();
	}

	free(con_str);

	PSUCCESS("buf_str_concat");
}

void test_buf_pack(void)
{
	/*@only@*/ buf_t     *buf          = NULL;
	/*@only@*/ char      *buf_data     = NULL;
	buf_s64_t buf_data_size = 256;
	buf_s64_t i;
	time_t    current_time  = time(0);
	srandom((unsigned int)current_time);

	PSPLITTER();

	PSTART("buf_pack");

	buf = buf_new(1024);
	if (NULL == buf) {
		PFAIL("buf_string: Can't allocate buf");
		abort();
	}

	PSTEP("Allocated buffer");

	buf_data = malloc(256);
	if (NULL == buf_data) {
		PFAIL("Can't allocate buffer");
		abort();
	}

	PSTEP("Allocated local buffer for random data");

	for (i = 0; i < buf_data_size; i++) {
		char randomNumber = (char)random();
		buf_data[i] = randomNumber;
	}


	PSTEP("Filled local buffer with random data");

	/* Make sure that this buffer ended not with 0 */
	//buf_data[buf_data_size - 1] = 9;

	if (OK != buf_add(buf, buf_data, buf_data_size)) {
		PFAIL("buf_pack: can't add");
		abort();
	}

	PSTEP("Added buffer into buf_t");

	if (buf_used(buf) != buf_data_size) {
		PFAIL("buf_pack");
		abort();
	}

	/* Compare memory */
	if (0 != memcmp(buf->data, buf_data, buf_data_size)) {
		PFAIL("buf_pack");
		abort();
	}

	PSTEP("Compared memory");

	/* Now we pack the buf */
	if (OK != buf_pack(buf)) {
		PFAIL("buf_pack");
		abort();
	}
	PSTEP("Packed buf_t");

	/* Test that the packed buffer has the right size */
	if (buf_used(buf) != buf_data_size) {
		PFAIL("buf_pack");
		abort();
	}
	PSTEP("That buf->used is right");

	/* Test that buf->room = buf->used + 1 */
	if (buf_used(buf) != buf_room(buf)) {
		printf("buf->room (%ld) != buf->used (%ld)\n", buf_room(buf), buf_used(buf));
		PFAIL("buf_pack");
		abort();
	}
	PSTEP("Tested room and used");

	//printf("%s\n", buf->data);
	if (OK != buf_free(buf)) {
		PFAIL("Can not free the buffer");
		abort();
	}
	free(buf_data);

	PSUCCESS("buf_pack");
}

void test_buf_canary(void)
{
	/*@only@*/ buf_t         *buf          = NULL;
	/*@only@*/ char          *buf_data     = NULL;
	buf_s64_t     buf_data_size = 256;
	buf_s64_t     i;
	time_t        current_time  = time(0);
	buf_t_flags_t flags;

	srandom((unsigned int)current_time);

	PSPLITTER();

	PSTART("buf_canary");

	/* We need to save and later restore flags: during this test we must unset the 'abort' flags */
	flags = buf_save_flags();
	buf_unset_abort_flag();

	buf = buf_new(0);
	if (NULL == buf) {
		PFAIL("buf_string: Can't allocate buf");
		abort();
	}

	if (OK != buf_mark_canary(buf)) {
		PFAIL("buf_canary: Can't set CANARY flag");
		abort();
	}

	PSTEP("Allocated buffer");

	buf_data = malloc(256);
	if (NULL == buf_data) {
		PFAIL("Can't allocate buffer");
		abort();
	}

	PSTEP("Allocated local buffer for random data");

	for (i = 0; i < buf_data_size; i++) {
		char randomNumber = (char)random();
		buf_data[i] = randomNumber;
	}

	PSTEP("Filled local buffer with random data");

	if (OK != buf_add(buf, buf_data, buf_data_size - 1)) {
		PFAIL("buf_pack: can't add");
		abort();
	}

	PSTEP("Added buffer into buf_t");

	if (buf_used(buf) != buf_data_size - 1) {
		PFAIL("buf_pack");
		abort();
	}

	/* Compare memory */
	if (0 != memcmp(buf->data, buf_data, buf_data_size - 1)) {
		PFAIL("buf_canary: buffer is wrong");
		abort();
	}

	PSTEP("Compared memory");

	/* Test canary */
	if (OK != buf_test_canary(buf)) {
		PFAIL("buf_canary: bad canary");
		abort();
	}

	PSTEP("Canary word is OK for the buffer");

	/* Now we copy the full buffer into buf->data and such we break the canary pattern */
	memcpy(buf->data, buf_data, buf_data_size);

	PSTEP("Corrupted buf->data: we expect an ERR");

	/* Test canary: we expect it to be wrong */
	if (OK == buf_test_canary(buf)) {
		PFAIL("buf_canary: good canary but must be bad");
		abort();
	}

	printf("Ignore the previous ERR printout; we expected it\n");

	PSTEP("The canary is broken. It is what expected to be");

	if (OK != buf_set_canary(buf)) {
		PFAIL("buf_canary: can't set canary on the buffer");
		abort();
	}

	PSTEP("Fixed canary");

	/* Test canary again */
	if (OK != buf_test_canary(buf)) {
		PFAIL("buf_canary: bad canary but must be good");
		abort();
	}

	PSTEP("Now canary is OK");

	/* Run buf validation */
	if (OK != buf_is_valid(buf)) {
		PFAIL("buf_canary: buffer is not valid");
		abort();
	}

	PSTEP("Buffer is valid");

	if (OK != buf_free(buf)) {
		PFAIL("buf_canary: buf_free returned not OK");
		abort();
	}

	PSTEP("Buffer released");

	free(buf_data);

	buf_restore_flags(flags);

	PSUCCESS("buf_canary");
}

int main(void)
{
	/* Abort on any error */
	//DD("This is regular print\n");
	//DDD("This is extended print\n");
	//DE("This is error print\n");
	//DDE("This is extended error print\n");

	buf_set_abort_flag();
	test_buf_new_zero_size();
	test_buf_new_increasing_size();
	test_buf_string(0);
	test_buf_string(1);
	test_buf_string(32);
	test_buf_string(1024);
	test_buf_pack_string();
	test_buf_str_concat();
	test_buf_pack();
	test_buf_canary();

	PSUCCESS("All tests passed, good work!");
	buf_t_stats_print();
	return (0);
}
