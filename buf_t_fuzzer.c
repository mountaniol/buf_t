#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "buf_t.h"
// #include "buf_t_string.h"
#include "buf_t_stats.h"
#include "buf_t_debug.h"

#define INPUTSIZE 1024

int test_buf_strpack(char *data, size_t size)
{
	buf_t *buf   = buf_string(size * 4);
	buf_t *buf2;

	char  *data1;
	char  *data2;

	if (NULL == buf) {
		return 1;
	}

	data1 = malloc(size);
	memcpy(data1, data, size);
	if (BUFT_OK != buf_set_data(buf, data1, size, size)) {
		goto err1;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err1;
	}

	buf2 = buf_string(size);
	if (NULL == buf2) {
		goto err1;
	}

	data2 = malloc(size);
	memcpy(data2, data, size);
	if (BUFT_OK != buf_set_data(buf2, data2, size, size)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_str_concat(buf, buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	if (BUFT_OK != buf_str_pack(buf)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	if (BUFT_OK != buf_str_pack(buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_free(buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_free(buf)) {
		return 1;
	}

	return 0;

err2:
	if (BUFT_OK != buf_free(buf2)) {
		return 1;
	}

err1:
	if (BUFT_OK != buf_free(buf)) {
		return 1;
	}

	return 1;
}

int   test_buf_sprintf(char *data, size_t size)
{
	buf_t *buf             = buf_sprintf("This is data of size %d: %s", size, data);

	if (NULL == buf) {
		return 1;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err1;
	}

	if (BUFT_OK != buf_free(buf)) {
		return 1;
	}
	return 0;

err1:
	if (BUFT_OK != buf_free(buf)) {
		return 1;
	}
	return 1;
}

int   test_buf_string(char *data, size_t size)
{
	buf_t *buf   = buf_string(size);
	buf_t *buf2;

	char  *data1;
	char  *data2;

	if (NULL == buf) {
		return 1;
	}

	data1 = malloc(size);
	memcpy(data1, data, size);
	if (BUFT_OK != buf_set_data(buf, data1, size, size)) {
		goto err1;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err1;
	}

	buf2 = buf_string(size);
	if (NULL == buf2) {
		goto err1;
	}

	data2 = malloc(size);
	memcpy(data2, data, size);
	if (BUFT_OK != buf_set_data(buf2, data2, size, size)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_str_concat(buf, buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	if (BUFT_OK != buf_free(buf)) {
		goto err1;
	}

	if (BUFT_OK != buf_free(buf2)) {
		return 1;
	}

	return 0;

	/*@ignore@*/
err2:
	buf_free(buf2);

err1:
	buf_free(buf);
	return 1;
	/*@end@*/
}

int   test_buf_t(char *data, size_t size)
{
	int   rc;
	buf_t *buf   = buf_new(size);
	buf_t *buf2;

	char  *data1;
	char  *data2;

	if (NULL == buf) {
		return 1;
	}

	data1 = malloc(size);
	memcpy(data1, data, size);
	if (BUFT_OK != buf_set_data(buf, data1, size, size)) {
		goto err1;
	}

	switch (data[0] % 17) {
	case 0:
		rc = buf_mark_canary(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		rc = buf_set_canary(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	case 1:
		rc = buf_mark_compresed(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	case 2:
		rc = buf_mark_crc(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	case 3:
		rc = buf_mark_encrypted(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	case 4:
		rc = buf_mark_immutable(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	case 5:
		rc = buf_mark_locked(buf);
		if (BUFT_OK != rc) {
			goto err1;
		}
		break;
	}

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err1;
	}

	buf2 = buf_new(size);
	if (NULL == buf2) {
		goto err1;
	}

	data2 = malloc(size);
	memcpy(data2, data, size);
	if (BUFT_OK != buf_set_data(buf2, data2, size, size)) {
		goto err2;
	}

	if (BUFT_OK != buf_is_valid(buf2)) {
		goto err2;
	}

	if (BUFT_OK != buf_free(buf)) {
		goto err1;
	}

	rc = buf_free(buf2);
	if (BUFT_OK != rc) {
		return 1;
	}

	return 0;

	/*@ignore@*/
err2:
	buf_free(buf2);

err1:
	buf_free(buf);
	/*@end@*/
	return 1;
}

int   test_buf_t_realloc(char *data, size_t size)
{
	int   i;
	int   rc;
	uint16_t *sizes = (uint16_t *)data;
	buf_t *buf = buf_new(size);

	if (NULL == buf) {
		return 1;
	}

	for (i = 0; i < 100; i++) {
		rc = buf_clean(buf);

		uint16_t *sizes = ((uint16_t *)data) + i;

		if (BUFT_OK != rc) {
			goto err1;
		}

		rc = buf_add_room(buf, sizes[i]);
		if (BUFT_OK != rc) {
			goto err1;
		}
	}

	rc = buf_free(buf);
	if (BUFT_OK != rc) {
		return 1;
	}

	return 0;

err1:
	buf_free(buf);
	/*@end@*/
	return 1;
}

int  main(__attribute__((unused))int argc, __attribute__((unused))char *argv[])
{

	int  rc;
	int  st               = 0;
	char input[INPUTSIZE] = {0};

	rc = read(STDIN_FILENO, input, INPUTSIZE);

	if (rc < 0) {
		fprintf(stderr, "Couldn't read stdin.\n");
	}

	switch (input[0] % 5) {
	case 0:
		st = test_buf_t(input, rc);
		break;
	case 1:
		st = test_buf_string(input, rc);
		break;
	case 2:
		st = test_buf_sprintf(input, rc);
		break;
	case 3:
		st = test_buf_strpack(input, rc);
		break;
	case 4:
		st = test_buf_t_realloc(input, rc);
		break;
	default:
		st = 1;
	}
	return st;

}
