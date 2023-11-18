#ifndef _BUF_T_TYPES_H__
#define _BUF_T_TYPES_H__

#include <stdint.h>

typedef enum {
	AGN = -2, /* "Try again" status */
	BAD = -1,  /* Error status */
	OK = 0,    /* Success status */
	YES = 0,   /* YES in case a function returns YES or NO */
	NO = 1,    /* NO in case a function returns YES or NO */
} ret_t;

/* Whidth of the flags field */
typedef uint8_t buf_t_flags_t;

/* Size of 'room' and 'used':
 * 1. If this type is "uint64", the max size of data buffer is:
 * 18446744073709551615 bytes,
 * or 18446744073709552 KB,
 * or 18446744073709.550781 Mb
 * or 18446744073.709552765 Gb
 * or 18446744.073709551245 Tb
 *
 * Large enough to keep whatever you want, at least for the next 10 years )
 *
 * 2. In case of CIRCULAR buffer we use half of the 'used' field to keep head of the buffer,
 * another half to keep the tail. In this case max value of the head tail is:
 *
 * 4294967295 bytes,
 * or 4294967.295 Kb
 * or 4294.967295 Mb
 * or 4.294967295 Gb
 *
 * Not as impressive as uint64. In case we need more, the type should be increased to uint128
 * 
 * 2. If this size redefined to uint16, the max size of data buffer is:
 * 65535 bytes, or 65 Kilobyte
 * 
 * Be careful if you do resefine this type.
 * If you plan to used buf_t for file opening / reading / writing, you may have a problem if this
 * type is too small.
 * 
 */

typedef int64_t buf_s64_t;
typedef int32_t buf_s32_t;
//typedef int64_t  buf_ssize_t;
typedef uint32_t buf_circ_usize_t;


/* buf_t flags */

/* How many bits are reserved for buffer type */
#define BUF_T_TYPE_WIDTH 3

/* We use mask to isolate type from other flags */
#define BUF_T_TYPE_MASK  0x07


/* * Types **
 ************
 ************/

/* This is just a regular buffer, keeping user's raw data.
   User knows what to do with it, we don't care */
#define BUF_T_TYPE_RAW        	0

/* String buffer. In this case, additional tests enabled */
#define BUF_T_TYPE_STRING        1

/* Bit buffer */
#define BUF_T_TYPE_BIT      		2

/* Array of elements */
#define BUF_T_TYPE_ARR      		3

/* Circular buffer */
#define BUF_T_TYPE_CIRC			4
#define BUF_T_CIRC_HEAD_WIDTH (32)
#define BUF_T_CIRC_MASK (0x0000FFFF)

/** Flags **/

/* Buffer is read only; for example you may keep a static char * / const char * in buf_t */
#define BUF_T_FLAG_READONLY     (1 << BUF_T_TYPE_WIDTH)

/* Buffer is compressed */
#define BUF_T_FLAG_COMPRESSED (1 << (BUF_T_TYPE_WIDTH + 1))

/* Buffer is enctypted */
#define BUF_T_FLAG_ENCRYPTED  (1 << (BUF_T_TYPE_WIDTH + 2))

/* Buffer is enctypted */
#define BUF_T_FLAG_CANARY  (1 << (BUF_T_TYPE_WIDTH + 3))

/* Buffer is crc32 protected */
#define BUF_T_FLAG_CRC  (1 << (BUF_T_TYPE_WIDTH + 4))

/* Buffer can not chane its size:
   we might need ot for implmenet a circular buffer ot top of buf ARRAY */
#define BUF_T_FLAG_FIXED_SIZE  (1 << (BUF_T_TYPE_WIDTH + 5))

/* Size of canary */
//typedef uint32_t buf_t_canary_t;
/* We use 2 characters as canary tail = 1 short */
//typedef uint16_t buf_t_canary_t;
typedef uint8_t buf_t_canary_t;
typedef uint8_t buf_t_checksum_t;

#endif /* _BUF_T_TYPES_H__ */