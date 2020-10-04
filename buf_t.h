#ifndef _BUF_T_H_
#define _BUF_T_H_

#include <stddef.h>
#include <sys/types.h>
//#define BUF_DEBUG
#define BUF_NOISY


/*
 * buf_t is an implementation of abstract buffer.
 * This buffer keeps data and its size.
 * buf->data - data.
 * buf->room - allocated memory
 * buf->used - size of used memory
 * 
 * A set of function help to manipulate the buffer: allocate / delete / add data and more.
 * Here is several examples:
 * 
 * ==== Example 1 ====
 * // Allocate buffer
 * buf_t *buf = buf_new(NULL, 0);
 * 
 * // Add data to the buffer
 * buf_add(buf, "Aloha", 5);
 * buf_add(buf, "/", 1);
 * buf_add(buf, "Shalom", 6);
 * 
 * // Now buffer contains string "Aloha/Shalom" without null terminator.
 * // The buf->room == 12
 * // The buf->used == 12
 * 
 * // Destroy buffer
 * buf_free(buf);
 * 
 * // The buffer and its memory securely destroyed: all data filled with 0 before it releaased.
 * // The 'buf' structure as well filled with '0' before destroyed.
 * 
 * ==== Example 2 ====
 * 
 * // Allocate buffer with memory == 32 bytes
 * buf_t = buf_new(NULL, 32);
 * // buf->data is buffer 32 bytes filled with '0'
 * 
 * // Add data
 * buf_add(buf, "Aloha", 5);
 * // buf->data contains "Aloha"
 * // buf->used == 5
 * // buf->room == 32
 * 
 * // Shrink memory to size of used area
 * buf_pack(buf)
 * // buf->data contains "Aloha"
 * // buf->used == 5
 * // buf->room == 5
 * 
 * buf_free(buf);
 * 
 * ==== Example 3 ====
 * 
 * // print string into buf_t
 * buf_t *buf = buf_sprintf("%s %s %s", "Lemon", "is", "yellow"); // buf_t allocated; // string
 * length measured and buf->data allocated, including terminating '\0'
 * 
 * 
 * // Now buf->data contains "Lemon is yellow\0"
 * // buf->used == 16
 * // buf->room == 16
 * // The used length contains terminating \0
 * 
 * // Print this string
 * printf("buf->data == %s; buf->used = %u\n", buf->data, buf->used);
 * 
 * // Release buffer
 * buf_free(buf);
 * 
 * ==== Example 4 ====
 * 
 * // print string into buf_t
 * buf_t *buf = buf_sprintf("%s %s %s", "Lemon", "is", "yellow"); // buf_t allocated; // string
 * length measured and buf->data allocated, including terminating '\0'
 * 
 * 
 * // Now buf->data contains "Lemon is yellow\0"
 * // buf->used == 16
 * // buf->room == 16
 * // The used length contains terminating \0
 * 
 * // Steal string from buffer
 * char *str = buf_steal_data(buf);
 * // Now str == "Lemon is yellow\0"
 * // buf->used == 0
 * // buf->room == 0
 * // buf->data == NULL
 * 
 * // Print this string
 * printf("string == %s\n", str);
 * 
 * // Release buffer
 * buf_free(buf);
 * 
 * // Release string
 * free(str);
 * 
 */


/* For uint32_t / uint8_t */
//#include <linux/types.h>
/*@-skipposixheaders@*/
#include <stdint.h>
/*@=skipposixheaders@*/
/* For err_t */
//#include "mp-common.h"

typedef enum {
	BAD = -1,  /* Error status */
	OK = 0,       /* Success status */
	AGN = 1, /* "Try again" status */
} ret_t;

#define TESTP(x, ret) do {if(NULL == x) { DDE("Pointer %s is NULL\n", #x); return ret; }} while(0)
#define TESTP_ASSERT(x, mes) do {if(NULL == x) { DE("[[ ASSERT! ]] %s == NULL: %s\n", #x, mes); abort(); } } while(0)

#ifdef TFREE_SIZE
	#undef TFREE_SIZE
#endif

#define TFREE_SIZE(x,sz) do { if(NULL != x) {memset(x,0,sz);free(x); x = NULL;} else {DE(">>>>>>>> Tried to free_size() NULL: %s (%s +%d)\n", #x, __func__, __LINE__);} }while(0)

/* Whidth of the flags field */
typedef uint8_t buf_t_flags_t;

/* Size of 'room' and 'used':
 * 1. If this type is "uint32", the max size of data buffer is:
 * 4294967295 bytes, or 4294967 KB, or 4294 Bb
 * 
 * 2. If this size redefined to uint16, the max size of data buffer is:
 * 65535 bytes, or 65 Kilobyte
 * 
 * Be careful if you do resefine this type.
 * If you plan to used buf_t for file opening / reading / writing, you may have a problem if this
 * type is too small.
 * 
 */

typedef uint32_t buf_usize_t;

/* Simple struct to hold a buffer / string and its size / lenght */

#ifdef BUF_DEBUG
struct buf_t_struct
{
	buf_usize_t room;           /* Allocated size */
	buf_usize_t used;           /* Used size */
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
struct buf_t_struct
{
	buf_usize_t room;           /* Allocated size */
	buf_usize_t used;           /* Used size */
	buf_t_flags_t flags;        /* Buffer flags. Optional. We may use it as we wish. */
	/*@temp@*/char *data;       /* Pointer to data */
};
#endif

typedef struct buf_t_struct buf_t;

/* buf_t flags */

/* We may mark the buffer as string buffer. In this case, additional test enabled */
#define BUF_T_STRING      (1)

/* Buffer is read only; for example you may keep a static char * / const char * in buf_t */
#define BUF_T_READONLY     (1<<1)

/* Buffer is compressed */
#define BUF_T_COMPRESSED (1<<2)

/* Buffer is enctypted */
#define BUF_T_ENCRYPTED  (1<<3)

/* Buffer is enctypted */
#define BUF_T_CANARY  (1<<4)

/* Buffer is crc32 protected */
#define BUF_T_CRC  (1<<5)

#define IS_BUF_STRING(buf) (buf->flags & BUF_T_STRING)
#define IS_BUF_RO(buf) (buf->flags & BUF_T_READONLY)
#define IS_BUF_COMPRESSED(buf) (buf->flags & BUF_T_COMPRESSED)
#define IS_BUF_ENCRYPTED(buf) (buf->flags & BUF_T_ENCRYPTED)
#define IS_BUF_CANARY(buf) (buf->flags & BUF_T_CANARY)
#define IS_BUF_CRC(buf) (buf->flags & BUF_T_CRC)

#define SET_BUF_STRING(buf) (buf->flags |= BUF_T_STRING)
#define SET_BUF_RO(buf) (buf->flags |= BUF_T_READONLY)
#define SET_BUF_COMPRESSED(buf) (buf->flags |= BUF_T_COMPRESSED)
#define SET_BUF_ENCRYPTED(buf) (buf->flags |= BUF_T_ENCRYPTED)

/* CANARY: Set a mark after allocated buffer*/
/* PRO and CONTRA of this method:*/
/* PRO: It can help to catch memory problems */
/* Contras: The buffer increased, and buffer validation should be run on every buffer operation */
/* The mark we set at the end of the buf if PROTECTED flag is enabled */

/* Size of canary */
//typedef uint32_t buf_t_canary_t;


//typedef uint32_t buf_t_canary_t;
/* We use 2 characters as canary tail = 1 short */
//typedef uint16_t buf_t_canary_t;
typedef uint8_t buf_t_canary_t;

#define BUF_T_CANARY_SIZE (sizeof(buf_t_canary_t))

//#define BUF_T_CANARY_WORD ((buf_t_canary_t) 0xFEE1F4EE)
#define BUF_T_CANARY_WORD ((buf_t_canary_t) 0x31415926)

// The CANARY char pattern is : 10101010 = 0XAA
#define BUF_T_CANARY_CHAR_PATTERN 0XAA
#define BUF_T_CANARY_SHORT_PATTERN 0XAAAA

//0x12345678)

/* Size of a regular buf_t structure */
#define BUF_T_STRUCT_SIZE (sizeof(buf_t))

/* Size of buf_t structure for network transmittion: without the last 'char *data' pointer */
#define BUF_T_STRUCT_NET_SIZE (sizeof(buf_t) - sizeof(char*))

/* How much bytes will be transmitted to send buf_t + its actual data */
#define BUF_T_NET_SEND_SIZE(b) (BUF_T_STRUCT_NET_SIZE + b->used)

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func void buf_set_abort(void)
 * @brief set "abort on errors" state
 * @param void
 */
extern void buf_set_abort(void);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func void buf_unset_abort(void)
 * @brief Unset "abort on errors" state
 * @param void
 */
extern void buf_unset_abort(void);

/**
 * @author Sebastian Mountaniol (14/06/2020)
 * @func void buf_default_flags(buf_t_flags_t f)
 * @brief Set default buf_t flags. Will be applied for every allocated new buf_t struct.
 * @param buf_t_flags_t f
 */
extern void buf_default_flags(buf_t_flags_t f);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func err_t buf_set_data(buf_t *buf, char *data, size_t size, size_t len)
 * @brief Set new data into buffer. The buf_t *buf must be clean, i.e. buf->data == NULL and
 *  buf->room == 0; After the new buffer 'data' set, the buf->len also set to 'len'
 * @param buf_t * buf 'buf_t' buffer to set new data 'data'
 * @param char * data Data to set into the buf_t
 * @param size_t size Size of the new 'data'
 * @param size_t len Length of data in the buffer, user must provide it
 * @return err_t Returns EOK on success
 * 	Return EACCESS if the buffer is read-only
 */
extern ret_t buf_set_data(/*@null@*/buf_t *buf, /*@null@*/char *data, size_t size, size_t len);

/**
 * @author Sebastian Mountaniol (15/06/2020)
 * @func err_t buf_is_valid(buf_t *buf)
 * @brief Test bufer validity
 * @param buf_t * buf Buffer to test
 * @return err_t Returns EOK if the buffer is valid.
 * 	Returns EINVAL if the 'buf' == NULL.
 * 	Returns EBAD if this buffer is invalid.
 */
extern ret_t buf_is_valid(buf_t *buf);

/**
 * @func buf_t* buf_new(size_t size)
 * @brief Allocate buf_t. A new buffer of 'size' will be
 *    allocated.
 * @author se (03/04/2020)
 * @param size_t size Data buffer size, may be 0
 * @return buf_t* New buf_t structure.
 */
extern /*@null@*/ buf_t *buf_new(size_t size);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func buf_t* buf_string(size_t size)
 * @brief Allocate buffer and mark it as STRING
 * @param size_t size
 * @return buf_t*
 */
extern /*@null@*/ buf_t *buf_string(size_t size);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func err_t buf_set_data_ro(buf_t *buf, char *data, size_t size)
 * @brief Set a data into buffer and lock the buffer, i.e. turn the buf into "read-only".
 * @param buf_t * buf Buffer to set read-only
 * @param char * data Data the buf_t will contain. It is legal if this parameter == NULL
 * @param size_t size Size of the buffer. If data == NULL this argument must be 0
 * @return err_t EOK on success
 * 	Returns ECANCELED if data == NULL but size > 0
 * 	Returns EACCESS if this buffer already marked as read-only.
 */
extern ret_t buf_set_data_ro(buf_t *buf, char *data, size_t size);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func void* buf_steal_data(buf_t *buf)
 * @brief 'Steal' data from buffer. After this operation the internal buffer 'data' returned to
 *    caller. After this function buf->data set to NULL, buf->len = 0, buf->size = 0
 * @param buf_t * buf Buffer to extract data buffer
 * @return void* Data buffer pointer on success, NULL on error. Warning: if the but_t did not have a
 * 	buffer (i.e. buf->data was NULL) the NULL will be returned.
 */
extern /*@null@*/void *buf_steal_data(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func void* buf_2_data(buf_t *buf)
 * @brief Return data buffer from the buffer and release the buffer. After this operation the buf_t
 *    structure will be completly destroyed. WARNING: disregarding to the return value the buf_t
 *    will be destoyed!
 * @param buf_t * buf Buffer to extract data
 * @return void* Pointer to buffer on success (buf if the buffer is empty NULL will be returned),
 */
extern /*@null@*/void *buf_2_data(/*@null@*/buf_t *buf);

/**
 * @brief Remove data from buffer (and free the data), set buf->room = buf->len = 0
 * @func err_t buf_clean(buf_t *buf)
 * @author se (16/05/2020)
 * @param buf Buffer to remove data from
 * @return err_t EOK if all right
 * 	EINVAL if buf is NULL pointer
 * 	EACCESS if the buffer is read-only, buffer kept untouched
 * @details If the buffer is invalid (see buf_is_valid()),
 * @details the opreration won't be interrupted and buffer will be cleaned.
 */
extern ret_t buf_clean(/*@only@*//*@null@*/buf_t *buf);

/**
 * @func int buf_room(buf_t *buf, size_t size)
 * @brief Allocate additional 'size' in the tail of buf_t data
 *    buffer; existing content kept unchanged. The new
 *    memory will be cleaned. The 'size' argument must be >
 *    0. For removing buf->data use 'buf_free_force()'
 * @author se (06/04/2020)
 * @param buf_t * buf Buffer to grow
 * @param size_t size How many byte to add
 * @return int EOK on success
 * 	EINVAL if buf == NULL
 * 	EACCESS if buf is read-only
 * 	ENOMEM if allocation of additional space failed. In this case the buffer kept untouched.
 * 	ENOKEY if the buffer marked as CAANRY but CANARY work can't be added.
 */
extern ret_t buf_add_room(/*@null@*/buf_t *buf, size_t size);

/**
 * @func int buf_test_room(buf_t *buf, size_t expect)
 * @brief The function accept size in bytes that caller wants to
 *    add into buf. It check if additional room needed. If
 *    yes, calls buf_room() to increase room. The 'expect'
 *    ca be == 0
 * @author se (06/04/2020)
 * @param buf_t * buf Buffer to test
 * @param size_t expect How many bytes will be added
 * @return int EOK if the buffer has sufficient room or if room added succesfully
 * 	EINVAL if buf is NULL or 'expected' == 0
 * 	Also can return all error statuses of buf_add_room()
 */
extern ret_t buf_test_room(/*@null@*/buf_t *buf, size_t expect);

/**
 * @func int buf_t_free_force(buf_t *buf)
 * @brief Free buf; if buf->data is not empty, free buf->data
 * @author se (03/04/2020)
 * @param buf_t * buf Buffer to remove
 * @return int EOK on success
 * 	EINVAL is the buf is NULL pointer
 * 	EACCESS if the buf is read-only
 * 	ECANCELED if the buffer is invalid
 */
extern ret_t buf_free(/*@only@*//*@null@*/buf_t *buf);

/**
 * @func err_t buf_add(buf_t *buf, const char *new_data, const size_t size)
 * @brief Append (copy) buffer "new_data" of size "size" to the tail of buf_t->data
 *    New memory allocated if needed.
 * @author se (06/04/2020)
 * @param buf_t * buf Buffer to append to buf->data
 * @param const char* new_data This buffer will be appended (copied) to the tail of buf->data
 * @param const size_t size Size of 'new_data' in bytes
 * @return int EOK on success
 * 	EINVAL if: 'buf' == NULL, or 'new_data' == NULL, or 'size' == 0
 * 	EACCESS if the 'buf' is read-only
 * 	ENOMEM if new memory can't be allocated
 */
extern ret_t buf_add(/*@null@*/buf_t *buf, /*@null@*/const char *new_data, const size_t size);

/**
 * @author Sebastian Mountaniol (14/06/2020)
 * @func ssize_t buf_used(buf_t *buf)
 * @brief Return size in bytes of used memory (which is buf->used)
 * @param buf_t * buf Buffer to check
 * @return ssize_t Number of bytes used on success
 * 	EINVAL if the 'buf' == NULL
 */
extern ssize_t buf_used(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (14/06/2020)
 * @func ssize_t buf_room(buf_t *buf)
 * @brief Return size of memory currently allocated for this 'buf' (which is buf->room)
 * @param buf_t * buf Buffer to test
 * @return ssize_t How many bytes allocated for this 'buf'
 * 	EINVAL if the 'buf' == NULL
 */
extern ssize_t buf_room(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func err_t buf_pack(buf_t *buf)
 * @brief Shrink buf->data to buf->len.
 * @brief This function may be used when you finished with the buf_t and
 *    its size won't change anymore.
 *    The unused memory will be released.
 * @param buf_t * buf Buffer to pack
 * @return err_t EOK on success;
 * 	EOK if this buffer is empty (buf->data == NULL) EOK returned
 * 	EOK if this buffer should not be packed (buf->used == buf->room)
 * 	EINVAL id 'buf' == NULL
 * 	ECANCELED if this buffer is invalid (see buf_is_valid)
 * 	ENOMEM if internal realloc can't reallocate / shring memory
 * 	Also can return one of buf_set_canary() errors
 */
extern ret_t buf_pack(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (02/06/2020)
 * @func buf_t* buf_sprintf(char *format, ...)
 * @brief sprintf into buf_t
 * @param char * format Format (like in printf first argument )
 * @return buf_t*
 */
extern buf_t *buf_sprintf(const char *format, ...);


/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func buf_t_flags_t buf_save_flags(void)
 * @brief Return current value of gloabs buf_t flags (the flags added to every new allocate buf_t)
 * @param void
 * @return buf_t_flags_t - value of global buf_t flags
 * @details
 */
extern buf_t_flags_t buf_save_flags(void);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func void buf_restore_flags(buf_t_flags_t flags)
 * @brief Restore global buf_t flags
 * @param buf_t_flags_t flags - flags to set
 *  @details See buf_save_flags()
 */
extern void buf_restore_flags(buf_t_flags_t flags);

/**** Mark / Unmark flags */

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_string(buf_t *buf)
 * @brief Mark (set flag) the buf as a
 * buffer containing string
 * @param buf_t * buf Buf to mark
 * @return err_t OK on success, EINVAL if
 * 	buf in NULL
 */
/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_string(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing string buffer
 * data @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_string(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_ro(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing read-only data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_ro(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_compresed(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing compressed data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_compresed(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_encrypted(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing encrypted data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_encrypted(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_canary(buf_t *buf)
 * @brief Mark (set flag) that the buf has canary word in the end of the buffer
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_canary(buf_t *buf)
 * @brief Mark (set flag) that the buf has CRC word in the end of the buffer
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_crc(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_string(buf_t *buf)
 * @brief Remove "string" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_string(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_ro(buf_t *buf)
 * @brief Remove "read-only" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_ro(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_compresed(buf_t *buf)
 * @brief Remove "compressed" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_compresed(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_encrypted(buf_t *buf)
 * @brief Remove "encryptes" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_encrypted(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_canary(buf_t *buf)
 * @brief Remove "canary" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_crc(buf_t *buf)
 * @brief Remove "crc" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_crc(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_set_canary(buf_t *buf)
 * @brief Set CANARY maer in the end of the buffer. The buf_t must be marked as CANARY.
 * @param buf_t * buf Buffer to set CANARy pattern
 * @return err_t EOK on success,
 * 	EINVAL if buf is NULL,
 * 	ECANCELED if buf does not have CANARY or if CANARY pattern is bad right after it set
 * @details If the buf doesn't have CANARY flag it will return ECANCELED.
 */
extern ret_t buf_set_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_force_canary(buf_t *buf)
 * @brief Set CAANRY mark in the end of the buffer and apply ANARY flag on the buffer
 * @param buf_t * buf Buffer to set CANARY
 * @return err_t EOK on success,
 * 	EINVAL if buf is NULL,
 * 	ECANCELED if the buffer is too small to set CANARY mark,
 * 	or one of the but_set_canary() function errors
 * @details Pay attention, the buffer will be decreased by 1 byte, and the last byte of the buffer
 *    will be replaced with CANARY mark. You must reserve the last byte for it.
 */
extern ret_t buf_force_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_test_canary(buf_t *buf)
 * @brief Check that CANARY mark is untouched
 * @param buf_t * buf Buffer to check
 * @return err_t EOK if CANARY word is untouched,
 * 	EINVAL if the buf is NULL,
 * 	ECANCELED if buf is not marked as a CANARY buffer,
 * 	EBAD if canary mark is invalid
 */
extern ret_t buf_test_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func buf_t_canary_t buf_get_canary(buf_t *buf)
 * @brief Return current value of CANARY byte
 * @param buf_t * buf Buffer to read CANARY
 * @return buf_t_canary_t Value of CANARY byte, ((buf_t_canary_t)-1) on error
 * @details You may want to use this function if CANARY is damaged
 */
extern buf_t_canary_t buf_get_canary(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func void buf_print_flags(buf_t *buf)
 * @brief Print flags of the buffer. Useful for debug
 * @param buf_t * buf Buf to read flags
 */
extern void buf_print_flags(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func int buf_is_string(buf_t *buf)
 * @brief Test if the buffer is a string buffer
 * @param buf_t * buf Buffer to check
 * @return int EOK if the buf a string buffer
 * 	Returns EINVAL if buf is NULL
 * 	Returns 1 if not
 */
extern int buf_is_string(buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_detect_used(buf_t *buf)
 * @brief If you played with the buffer's data (for example, copied / replaced tezt in the
 *    buf->data) this function will help to detect right buf->used value.
 * @param buf_t * buf Buffer to analyze
 * @return err_t EOK on succes + buf->used set to a new value
 * 	EINVAL is 'buf' is NULL
 * 	ECANCELED if the buf is invalid or if the buf is empty
 */
extern ret_t buf_detect_used(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func ssize_t buf_recv(buf_t *buf, const int socket, const size_t expected, const int flags)
 * @brief Receive from socket into buffer
 * @param buf_t * buf Buffer to save received data
 * @param const int socket Opened socket
 * @param const size_t expected How many bytes expected
 * @param const int flags Flags to pass to recv() function
 * @return ssize_t Number of received bytes
 * 	EINVAL if buf is NULL, else returns status of recv() function
 */
extern ssize_t buf_recv(buf_t *buf, const int socket, const size_t expected, const int flags);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func buf_t* buf_from_string(char *str, size_t size_without_0)
 * @brief Convert given string "str" into buf_t. The resulting buf_t is a nirmal STRING type buffer.
 * @param char * str String to convert
 * @param size_t size_without_0 Length of the string without terminating '\0'
 * @return buf_t* New buf_t containing the "str"
 */
extern /*@null@*/ buf_t *buf_from_string(/*@null@*/char *str, size_t size_without_0);

/* Additional defines */
#ifdef BUF_DEBUG

	#define BUF_TEST(buf) do {if (0 != buf_is_valid(buf)){fprintf(stderr, "######>>> Buffer invalid here: func: %s file: %s + %d [allocated here: %s +%d %s()]\n", __func__, __FILE__, __LINE__, buf->filename, buf->line, buf->func);}} while (0)
	#define BUF_DUMP(buf) do {DD("[BUFDUMP]: [%s +%d] buf = %p, data = %p, room = %u, used = %u [allocated here: %s +%d %s()]\n", __func__, __LINE__, buf, buf->data, buf->room, buf->used, buf->filename, buf->line, buf->func);} while(0)
	#define BUF_DUMP_ERR(buf) do {DD("[BUFDUMP]: [%s +%d] buf = %p, data = %p, room = %u, used = %u [allocated here: %s +%d %s()]\n", __func__, __LINE__, buf, buf->data, buf->room, buf->used, buf->filename, buf->line, buf->func);} while(0)
#else
	#define BUF_TEST(buf) do {if (0 != buf_is_valid(buf)){fprintf(stderr, "######>>> Buffer test invalid here: func: %s file: %s + %d\n", __func__, __FILE__, __LINE__);}} while (0)
	#define BUF_DUMP(buf) do {DD("[BUFDUMP]: [%s +%d] buf = %p, data = %p, room = %u, used = %u\n", __func__, __LINE__, buf, buf->data, buf->room, buf->used);} while(0)
	#define BUF_DUMP_ERR(buf) do {DD("[BUFDUMP]: [%s +%d] buf = %p, data = %p, room = %u, used = %u\n", __func__, __LINE__, buf, buf->data, buf->room, buf->used);} while(0)

#endif

#ifndef BUF_NOISY
	#undef BUF_DUMP
	#define BUF_DUMP(buf) do{}while(0)
#endif

#endif /* _BUF_T_H_ */
