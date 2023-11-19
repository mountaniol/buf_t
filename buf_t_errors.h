#ifndef _BUF_T_ERRORS_H__
#define _BUF_T_ERRORS_H__

/**
 * @file
 * @brief In this file defined possible buf_t errors
 */

typedef enum {
	BUFT_NO = 1,    /* NO in case a function returns YES or NO */
	BUFT_OK = 0,    /* Success status */
	BUFT_YES = 0,   /* YES in case a function returns YES or NO */
	BUFT_ERROR_BASE = 10000,
	BUFT_BAD = BUFT_ERROR_BASE + 1,  /* Error status */
	BUFT_NULL_POINTER = BUFT_ERROR_BASE + 2,  /* Error status */
	BUFT_BAD_BUFT_TYPE = BUFT_ERROR_BASE + 3,  /* Error status */
	BUFT_BAD_CANARY = BUFT_ERROR_BASE + 4,  /* Error status */
	BUFT_WRONG_BUF_FLAG = BUFT_ERROR_BASE + 5,  /* Error status */
	BUFT_CANNOT_SET_CANARY = BUFT_ERROR_BASE + 6,  /* Error status */
	BUFT_OUT_OF_LIMIT_OP = BUFT_ERROR_BASE + 7,  /* The operation can not be executed because out of limit situation detected */
	BUFT_TOO_SMALL = BUFT_ERROR_BASE + 8  /* The operation can not be executed because there is not space to add */
} buf_t_error_t;


#endif /* _BUF_T_ERRORS_H__ */
