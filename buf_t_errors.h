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
	BUFT_BAD,  /* Error status */
	BUFT_NULL_POINTER,  /* Error status */
	BUFT_NULL_DATA,  /* Error status */
	BUFT_BAD_BUFT_TYPE,  /* Error status */
	BUFT_BAD_CANARY,  /* Error status */
	BUFT_NO_CANARY,  /* Error status */
	BUFT_WRONG_BUF_FLAG,  /* Error status */
	BUFT_CANNOT_SET_CANARY,  /* Error status */
	BUFT_OUT_OF_LIMIT_OP,  /* The operation can not be executed because out of limit situation detected */
	BUFT_TOO_SMALL,  /* The operation can not be executed because there is not space to add */
	BUFT_ALLOCATE,  /* For some reason could not allocate memory */
	BUFT_INCREMENT,  /* For some reason could decrement room */
	BUFT_DECREMENT,  /* For some reason could decrement room */
	BUFT_BAD_USED,  /* For some reason could not allocate memory */
	BUFT_BAD_ROOM,  /* Room size is damaged, returned from validation function */
	BUFT_BAD_SIZE,  /* A bad size passed in argument */
	BUFT_SET_ROOM_SIZE,  /* Can not set new ->room size by some reason */
	BUFT_SET_USED_SIZE,  /* Can not set new ->used size by some reason */
	BUFT_IMMUTABLE_DAMAGED,  /* The buffer is constant, but looks like the buffer is invalid */
	BUFT_IS_IMMUTABLE,  /* Caller asked for an operation forbidden for a constant buffer  */
	BUFT_IS_LOCKED,  /* Caller asked for an operation forbidden for a locked buffer  */
	BUFT_UNKNOWN_TYPE,  /* Unknown buffer type; it it a severe problem indicating version mismatcj or memory corruption */
	BUFT_HAS_CANARY,  /* The buffer is asked to set data, but the buffer has canary. Before set new ->data, the CANARY flag should be removed */
} buf_t_error_t;


#endif /* _BUF_T_ERRORS_H__ */
