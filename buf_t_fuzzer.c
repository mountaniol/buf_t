#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "buf_t.h"
#include "buf_t_string.h"
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

	buf_str_concat(buf, buf2);

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	buf_str_pack(buf);
	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	buf_str_pack(buf2);
	if (BUFT_OK != buf_free(buf2)) {
		goto err2;
	}

	buf_free(buf2);

	return 0;

err2:
	buf_free(buf2);

err1:
	buf_free(buf);
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

	buf_free(buf);
	return 0;

err1:
	buf_free(buf);
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

	buf_str_concat(buf, buf2);

	if (BUFT_OK != buf_is_valid(buf)) {
		goto err2;
	}

	if (BUFT_OK != buf_free(buf)) {
		goto err1;
	}

	buf_free(buf2);

	return 0;

err2:
	buf_free(buf2);

err1:
	buf_free(buf);
	return 1;
}

int   test_buf_t(char *data, size_t size)
{
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
		buf_mark_canary(buf);
		buf_set_canary(buf);
		break;
	case 1:
		buf_mark_compresed(buf);
		break;
	case 2:
		buf_mark_crc(buf);
		break;
	case 3:
		buf_mark_encrypted(buf);
		break;
	case 4:
		buf_mark_ro(buf);
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

	buf_free(buf2);

	return 0;

err2:
	buf_free(buf2);

err1:
	buf_free(buf);
	return 1;
}


int  main(int argc, char *argv[])
{

	int  rc;
	int  st;
	char input[INPUTSIZE] = {0};
	char *data;

	rc = read(STDIN_FILENO, input, INPUTSIZE);

	if (rc < 0) {
		fprintf(stderr, "Couldn't read stdin.\n");
	}

	switch (input[0] % 4) {
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
	}
	return st;

}
