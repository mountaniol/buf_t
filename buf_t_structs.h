#ifndef _BUF_T_STRUCTS_H__
#define _BUF_T_STRUCTS_H__

#include "buf_t_types.h"
/* This structure used in case the buffer is an array:
 * Instead of just 'used' we need to keep how many memners in the array,
   and what is the size of a member; all members must be the same size */

typedef struct array {
	buf_s32_t size;
	buf_s32_t members;
} arr_used_t;

typedef struct head_tail_struct {
	buf_circ_usize_t head;
	buf_circ_usize_t tail;
} head_tail_t;

/* Simple struct to hold a buffer / string and its size / lenght */

#ifdef BUF_DEBUG
struct buf_t_struct {
	buf_s64_t room;           /* Overall allocated size */
	/* The next union is shows how many of the 'room' is used;
	   THe used size can be less than allocated, i.e., 'used' <= 'used' */
	union {
		buf_s64_t used;           	/* For string and raw buf: used size */
		head_tail_t ht;             /* For cirrcular buffer: head and tail of the circular buffer */
		arr_used_t arr;				/* For array: how many members in arr and a member size */
	};
	buf_t_flags_t flags;        /* Buffer flags. Optional. We may use it as we wish. */
	/*@temp@*/char *data;       /* Pointer to data */

	/* Where this buffer allocated: function */
	const char *func;
	/* Where this buffer allocated: file */
	const char *filename;
	/* Where this buffer allocated: line */
	int line;
};

#else /* Not debug */
/* Simple struct to hold a buffer / string and its size / lenght */
struct buf_t_struct {
	buf_s64_t room;           /* Allocated size */
	union {
		buf_s64_t used;           	/* For string and raw buf: used size */
		head_tail_t ht;             /* For cirrcular buffer: head and tail of the circular buffer */
		arr_used_t arr;				/* For array: how many members in arr and a member size */
	};
	buf_t_type_t type;        /* Buffer type. Optional. We may use it as we wish. */
	buf_t_flags_t flags;        /* Buffer flags. Optional. We may use it as we wish. */
	/*@only@*/ char *data;       /* Pointer to data */
};
#endif

typedef /*@abstract@*/ struct buf_t_struct buf_t;

#endif /* _BUF_T_STRUCTS_H__ */
