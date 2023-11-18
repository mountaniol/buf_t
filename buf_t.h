#ifndef _BUF_T_H_
#define _BUF_T_H_

#include <stddef.h>
#include <sys/types.h>
#include "buf_t_types.h"
#include "buf_t_structs.h"
//#define BUF_DEBUG
#define BUF_NOISY

/* For uint32_t / uint8_t */
//#include <linux/types.h>
/*@-skipposixheaders@*/
#include <stdint.h>
/*@=skipposixheaders@*/
/* For err_t */
//#include "mp-common.h"

#define TESTP(x, ret) do {if(NULL == x) { DDE("Pointer %s is NULL\n", #x); return ret; }} while(0)
#define TESTP_ASSERT(x, mes) do {if(NULL == x) { DE("[[ ASSERT! ]] %s == NULL: %s\n", #x, mes); abort(); } } while(0)

/* This is a switchable version; depending on the global abort flag it will abort or rturn an error */
extern int bug_get_abort_flag(void);
//#define T_RET_ABORT(x, ret) do {if(NULL == x) {if(0 != bug_get_abort_flag()) {DE("[[ ASSERT! ]] %s == NULL\n", #x);abort();} else {DDE("[[ ERROR! ]]: Pointer %s is NULL\n", #x); return ret;}}} while(0)
#define T_RET_ABORT(x, ret) do {if(NULL == x) {DE("[[ ASSERT! ]] %s == NULL\n", #x);abort(); }} while(0)

#ifdef TFREE_SIZE
	#undef TFREE_SIZE
#endif

#define TFREE_SIZE(x,sz) do { if(NULL != x) {memset(x,0,sz);free(x); x = NULL;} else {DE(">>>>>>>> Tried to free_size() NULL: %s (%s +%d)\n", #x, __func__, __LINE__);} }while(0)

typedef /*@abstract@*/ struct buf_t_struct buf_t;

/*@access buf_t@*/

/* Types test */
#define BUF_TYPE(buf) (buf->flags & BUF_T_TYPE_MASK)
#define IS_BUF_TYPE_STRING(buf) ( BUF_TYPE(buf) == BUF_T_TYPE_STRING )
#define IS_BUF_TYPE_BIT(buf) ( BUF_TYPE(buf) == BUF_T_TYPE_BIT )
#define IS_BUF_TYPE_CIRC(buf) ( BUF_TYPE(buf) == BUF_T_TYPE_CIRC )
#define IS_BUF_TYPE_ARR(buf) ( BUF_TYPE(buf) == BUF_T_TYPE_ARR )

/* Flags test */
#define IS_BUF_RO(buf) (buf->flags & BUF_T_FLAG_READONLY)
#define IS_BUF_COMPRESSED(buf) (buf->flags & BUF_T_FLAG_COMPRESSED)
#define IS_BUF_ENCRYPTED(buf) (buf->flags & BUF_T_FLAG_ENCRYPTED)
#define IS_BUF_CANARY(buf) (buf->flags & BUF_T_FLAG_CANARY)
#define IS_BUF_CRC(buf) (buf->flags & BUF_T_FLAG_CRC)

/* Type set */
#define SET_BUF_TYPE_STRING(buf) (buf->flags |= BUF_T_TYPE_STRING)
#define SET_BUF_TYPE_RO(buf) (buf->flags |= BUF_T_FLAG_READONLY)
#define SET_BUF_TYPE_COMPRESSED(buf) (buf->flags |= BUF_T_FLAG_COMPRESSED)
#define SET_BUF_TYPE_ENCRYPTED(buf) (buf->flags |= BUF_T_FLAG_ENCRYPTED)
/*@noaccess buf_t@*/

/* CANARY: Set a mark after allocated buffer*/
/* PRO and CONTRA of this method:*/
/* PRO: It can help to catch memory problems */
/* Contras: The buffer increased, and buffer validation should be run on every buffer operation */
/* The mark we set at the end of the buf if PROTECTED flag is enabled */

/* Canary size: the size of special buffer added after data to detect data tail corruption */
#define BUF_T_CANARY_SIZE (sizeof(buf_t_canary_t))

//#define BUF_T_CANARY_WORD ((buf_t_canary_t) 0xFEE1F4EE)
#define BUF_T_CANARY_WORD ((buf_t_canary_t) 0x31415926)

// The CANARY char pattern is : 10101010 = 0XAA
#define BUF_T_CANARY_CHAR_PATTERN 0XAA
#define BUF_T_CANARY_SHORT_PATTERN 0XAAAA

//0x12345678)

#define BUF_T_CRC_SIZE (sizeof(buf_t_checksum_t))

/* Size of a regular buf_t structure */
#define BUF_T_STRUCT_SIZE (sizeof(buf_t))

/* Size of buf_t structure for network transmittion: without the last 'char *data' pointer */
#define BUF_T_STRUCT_NET_SIZE (sizeof(buf_t) - sizeof(char*))

/* How much bytes will be transmitted to send buf_t + its actual data */
#define BUF_T_NET_SEND_SIZE(b) (BUF_T_STRUCT_NET_SIZE + b->used)


/**
 * @author Sebastian Mountaniol (12/17/21)
 * buf_t_flags_t bug_get_abort_flag(void)
 * 
 * @brief Get the status of global "abort" flag. If this flag is
 *  	  set, the buf_t will abort on errors and generate a
 *  	  core file
 * @param void  
 * 
 * @return buf_t_flags_t Status of 'abort' flag. 0 means it is
 *  	   not ebabled.
 * @details 
 */
extern int bug_get_abort_flag(void);

/** If there is 'abort on error' is set, this macro stops
 *  execution and generates core file */
#define TRY_ABORT() do{ if(0 != bug_get_abort_flag()) {DE("Abort in %s +%d\n", __FILE__, __LINE__);abort();} } while(0)

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func void buf_set_abort(void)
 * @brief set "abort on errors" state
 * @param void
 */
extern void buf_set_abort_flag(void);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func void buf_unset_abort(void)
 * @brief Unset "abort on errors" state
 * @param void
 */
extern void buf_unset_abort_flag(void);

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
 *  Return -EACCESS if the buffer is read-only
 *  Return -EINVAL if buffer or data is NULL
 */
extern ret_t buf_set_data(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@null@*/ /*@only@*/ /*@in@*/char *data, const buf_s64_t size, const buf_s64_t len);

/**
 * @author Sebastian Mountaniol (12/19/21)
 * void* buf_data(buf_t *buf)
 * 
 * @brief Returns buf->data
 * @param buf - Buf to return data from
 * 
 * @return void* Returns buf->data; NULL on an error
 * @details 
 */
extern void /*@temp@*/ *buf_data(/*@temp@*//*@in@*//*@special@*/buf_t *buf);


/**
 * @author Sebastian Mountaniol (12/19/21)
 * ret_t buf_data_is_null(buf_t *buf)
 * 
 * @brief Test that the buf->data is NULL
 * @param buf - Buffer to test
 * 
 * @return ret_t YES if data is NULL, NO if data not NULL,
 *  	   -EINVAL if the buffer is NULL pointer
 * @details 
 */
extern ret_t buf_data_is_null(/*@temp@*//*@in@*//*@special@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (15/06/2020)
 * @func err_t buf_is_valid(buf_t *buf)
 * @brief Test bufer validity
 * @param buf_t * buf Buffer to test
 * @return err_t Returns EOK if the buffer is valid.
 * 	Returns EINVAL if the 'buf' == NULL.
 * 	Returns EBAD if this buffer is invalid.
 */
extern ret_t buf_is_valid(/*@temp@*/ /*@in@*/buf_t *buf);

/**
 * @func buf_t* buf_new(size_t size)
 * @brief Allocate buf_t. A new buffer of 'size' will be
 *    allocated.
 * @author se (03/04/2020)
 * @param size_t size Data buffer size, may be 0
 * @return buf_t* New buf_t structure.
 */
extern /*@null@*/ /*@in@*/buf_t *buf_new(buf_s64_t size);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func err_t buf_set_data_ro(buf_t *buf, char *data, size_t size)
 * @brief Set a data into buffer and lock the buffer, i.e. turn the buf into "read-only".
 * @param buf_t * buf Buffer to set read-only
 * @param char * data Data the buf_t will contain. It is legal if this parameter == NULL
 * @param size_t size Size of the buffer. If data == NULL this argument must be 0
 * @return err_t EOK on success
 * 	Returns -ECANCELED if data == NULL but size > 0
 * 	Returns -EACCESS if this buffer already marked as read-only.
 */
extern ret_t buf_set_data_ro(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@null@*//*@only@*//*@in@*/char *data, buf_s64_t size);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func void* buf_steal_data(buf_t *buf)
 * @brief 'Steal' data from buffer. After this operation the internal buffer 'data' returned to
 *    caller. After this function buf->data set to NULL, buf->len = 0, buf->size = 0
 * @param buf_t * buf Buffer to extract data buffer
 * @return void* Data buffer pointer on success, NULL on error. Warning: if the but_t did not have a
 * 	buffer (i.e. buf->data was NULL) the NULL will be returned.
 */
extern /*@null@*//*@only@*/void *buf_steal_data(/*@temp@*/ /*@in@*//*@special@*/ buf_t *buf);

/**
 * @author Sebastian Mountaniol (01/06/2020)
 * @func void* buf_2_data(buf_t *buf)
 * @brief Return data buffer from the buffer and release the buffer. After this operation the buf_t
 *    structure will be completly destroyed. WARNING: disregarding to the return value the buf_t
 *    will be destoyed!
 * @param buf_t * buf Buffer to extract data
 * @return void* Pointer to buffer on success (buf if the buffer is empty NULL will be returned),
 */
extern /*@null@*/void *buf_2_data(/*@only@*//*@in@*//*@special@*/buf_t *buf);

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
extern ret_t buf_clean(/*@temp@*//*@in@*//*@special@*/buf_t *buf);

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
 * 	-EINVAL if buf == NULL
 * 	-EACCESS if buf is read-only
 *  -ENOMEM if allocation of additional space failed. In this
 *  case the buffer kept untouched. -ENOKEY if the buffer marked
 *  as CAANRY but CANARY work can't be added.
 */
extern ret_t buf_add_room(/*@temp@*//*@in@*//*@special@*/buf_t *buf, buf_s64_t size);

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
extern ret_t buf_test_room(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t expect);

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
extern ret_t buf_free(/*@only@*//*@in@*//*@special@*/buf_t *buf);

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
extern ret_t buf_add(/*@temp@*//*@in@*//*@special@*/buf_t *buf, /*@temp@*//*@in@*/const char *new_data, const buf_s64_t size);

/**
 * @author Sebastian Mountaniol (14/06/2020)
 * @func buf_usize_t buf_used(buf_t *buf)
 * @brief Return size in bytes of used memory (which is buf->used)
 * @param buf_t * buf Buffer to check
 * @return ssize_t Number of bytes used on success
 * 	EINVAL if the 'buf' == NULL
 */
extern buf_s64_t buf_used(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_set_used(buf_t *buf, buf_usize_t used)
 * 
 * @brief Set a new value of the buf->used
 * @param buf - Buffer to set the new value
 * @param used - The new value to set
 * 
 * @return ret_t 
 * @details 
 */
extern ret_t buf_set_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t used);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_inc_used(buf_t *buf, buf_usize_t inc)
 * 
 * @brief Increment the buf->used value by 'inc'
 * @param buf - The buffer to increment the value of the
 *  		  buf->used
 * @param inc - The value to add to the buf->used
 * 
 * @return ret_t OK on success, BAD on an error
 * @details 
 */
extern ret_t buf_inc_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t used);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_dec_used(buf_t *buf, buf_usize_t dec)
 * 
 * @brief Decrement value of the buf->used 
 * @param buf - Buffer to decrement the '->used'
 * @param dec - Decrement ->used by this value; the 'used' must
 *  		   be > 0 (it can not be < 0, however)
 * 
 * @return ret_t OK on success, BAD on an error
 * @details 
 */
extern ret_t buf_dec_used(/*@temp@*//*@in@*/buf_t *buf, buf_s64_t dec);

/**
 * @author Sebastian Mountaniol (14/06/2020)
 * @func buf_usize_t buf_room(buf_t *buf)
 * @brief Return size of memory currently allocated for this 'buf' (which is buf->room)
 * @param buf_t * buf Buffer to test
 * @return ssize_t How many bytes allocated for this 'buf'
 * 	EINVAL if the 'buf' == NULL
 */
extern buf_s64_t buf_room(/*@temp@*/ /*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_set_room(buf_t *buf, buf_usize_t room)
 * 
 * @brief Set new buf->room value
 * @param buf - Buffer to set the a value
 * @param room - The new value of buf->room to set
 * 
 * @return ret_t OK on success, BAD on an error
 * @details 
 */
extern ret_t buf_set_room(/*@temp@*/ /*@in@*/ buf_t *buf, buf_s64_t room);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_inc_room(buf_t *buf, buf_usize_t inc)
 * 
 * @brief Increment value of buf->room by 'inc' value
 * @param buf - Buffer to increment the buf->room value
 * @param inc - The value to add to buf->room
 * 
 * @return ret_t OK on sucess, BAD on an error
 * @details 
 */
extern ret_t buf_inc_room(/*@temp@*/ /*@in@*/ buf_t *buf, buf_s64_t inc);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * ret_t buf_dec_room(buf_t *buf, buf_usize_t dec)
 * 
 * @brief Decrement the value of buf->room by 'dec' value
 * @param buf - The buffer to decrement buf->room
 * @param dec - The value to substract from nuf->room
 * 
 * @return ret_t OK on sucess, BAD on an error
 * @details The 'dec' must be less or equal to the buf->room,
 *  		else BAD error returned and no value decremented
 */
extern ret_t buf_dec_room(/*@temp@*/ /*@in@*/buf_t *buf, buf_s64_t dec);

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
extern ret_t buf_pack(/*@temp@*//*@in@*//*@special@*/buf_t *buf);

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
 * @brief Mark (set flag) the buf as a buffer containing string buffer
 * data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, -EINVAL if buf is NULL
 */
extern ret_t buf_mark_string(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/7/23)
 * @brief Mark (set flag) the buf as a buffer containing an
 * array
 * @param buf_t * buf Buffer to mark as array
 * @return err_t EOK on success, -EINVAL if buf is NULL
 */
extern ret_t buf_mark_array(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_set_flag(buf_t *buf, buf_t_flags_t f)
 * 
 * @brief Set the buffer flags 
 * @param buf - Buffer to set flags
 * @param f - Flag(s) to set
 *
 * @return ret_t OK on success, -EINVAL if NULL pointer received
 * @details 
 */
extern ret_t buf_set_flag(/*@temp@*//*@in@*/buf_t *buf, buf_t_flags_t f);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_rm_flag(buf_t *buf, buf_t_flags_t f)
 * 
 * @brief Unset a flag (or flags) from the buffer flags
 * @param buf - Buffer to unset flag(s)
 * @param f - Flag(s) to unset
 * 
 * @return ret_t 
 * @details 
 */
extern ret_t buf_rm_flag(/*@temp@*//*@in@*/buf_t *buf, buf_t_flags_t f);
/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_ro(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing read-only data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, -EINVAL if buf is NULL
 */
extern ret_t buf_mark_ro(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_compresed(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing compressed data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_compresed(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_encrypted(buf_t *buf)
 * @brief Mark (set flag) the buf as a buffer containing encrypted data
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_encrypted(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_canary(buf_t *buf)
 * @brief Mark (set flag) that the buf has canary word in the end of the buffer
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_canary(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_mark_canary(buf_t *buf)
 * @brief Mark (set flag) that the buf has CRC word in the end of the buffer
 * @param buf_t * buf Buffer to mark
 * @return err_t EOK on success, EINVAL if buf is NULL
 */
extern ret_t buf_mark_crc(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_string(buf_t *buf)
 * @brief Remove "string" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_string(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_ro(buf_t *buf)
 * @brief Remove "read-only" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_ro(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_compresed(buf_t *buf)
 * @brief Remove "compressed" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_compressed(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_encrypted(buf_t *buf)
 * @brief Remove "encryptes" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_encrypted(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_canary(buf_t *buf)
 * @brief Remove "canary" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_canary(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_unmark_crc(buf_t *buf)
 * @brief Remove "crc" mark (unset flag) from the buf
 * @param buf_t * buf Buffer to unmark
 * @return err_t OK on success, EINVAL if buf is NULL
 */
extern ret_t buf_unmark_crc(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_set_canary(buf_t *buf)
 * @brief Set the CANARY mark in the end of the buffer. The
 *  	  buf_t must be marked as CANARY, else an error
 *  	  returned.
 * @param buf_t * buf Buffer to set CANARy pattern
 * @return err_t EOK on success,
 * 	EINVAL if buf is NULL,
 *  ECANCELED if the buf does not have CANARY or if the CANARY
 *  pattern is not valid right after it set
 * @details If the buf doesn't have CANARY flag it will return ECANCELED.
 */
extern ret_t buf_set_canary(/*@temp@*//*@in@*//*@special@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_force_canary(buf_t *buf)
 * @brief Set CAANRY mark in the end of the buffer and apply ANARY flag on the buffer
 * @param buf_t * buf Buffer to set CANARY
 * @return err_t EOK on success,
 * 	-EINVAL if buf is NULL,
 * 	-ECANCELED if the buffer is too small to set CANARY mark,
 * 	or one of the but_set_canary() function errors
 * @details Pay attention, the buffer will be decreased by 1 byte, and the last byte of the buffer
 *    will be replaced with CANARY mark. You must reserve the last byte for it.
 */
extern ret_t buf_force_canary(/*@temp@*//*@in@*/ buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func err_t buf_test_canary(buf_t *buf)
 * @brief Check that CANARY mark is untouched
 * @param buf_t * buf Buffer to check
 * @return err_t EOK if CANARY word is untouched,
 * 	-EINVAL if the buf is NULL,
 * 	-ECANCELED if buf is not marked as a CANARY buffer,
 * 	EBAD if canary mark is invalid
 */
extern ret_t buf_test_canary(/*@temp@*//*@in@*//*@special@*/ buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func buf_t_canary_t buf_get_canary(buf_t *buf)
 * @brief Return current value of CANARY byte
 * @param buf_t * buf Buffer to read CANARY
 * @return buf_t_canary_t Value of CANARY byte, ((buf_t_canary_t)-1) on error
 * @details You may want to use this function if CANARY is damaged
 */
extern buf_t_canary_t buf_get_canary(/*@temp@*//*@in@*//*@special@*/ buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func void buf_print_flags(buf_t *buf)
 * @brief Print flags of the buffer. Useful for debug
 * @param buf_t * buf Buf to read flags
 */
extern void buf_print_flags(/*@temp@*//*@in@*/ buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func int buf_is_string(buf_t *buf)
 * @brief Test if the buffer is a string buffer
 * @param buf_t * buf Buffer to check
 * @return int EOK if the buf a string buffer
 * 	Returns -EINVAL if buf is NULL
 * 	Returns 1 if not a string buffer
 */
extern int buf_is_string(/*@temp@*/ /*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/7/23)
 * @brief Test if the buffer is an array buffer
 * @param buf_t * buf Buffer to check
 * @return int EOK if the buf an array buffer
 * 	Returns -EINVAL if buf is NULL
 * 	Returns 1 if not an array buffer
 */
extern int buf_is_array(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * int buf_is_bit(buf_t *buf)
 * 
 * @brief Test if the buffer is a bit buffer
 * @param buf - Buffer to test
 * 
 * @return int OK if it is a bit buffer, -EINVAL if the buffer
 *  	   is NULL, 1 if the buffer is not a bit buffer
 * @details 
 */
extern int buf_is_bit(/*@temp@*//*@in@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/16/21)
 * int buf_is_circ(buf_t *buf)
 * 
 * @brief Test if the buffer is a circular buffer
 * @param buf - Buffer to test
 * 
 * @return int - OK  if the buffer is a circular buffer, 1 if
 *  	   not, -EINVAL if the buffer id NULL pointer
 * @details 
 */
extern int buf_is_circ(/*@temp@*/ /*@in@*/buf_t *buf);
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
extern ret_t buf_detect_used(/*@temp@*//*@in@*//*@special@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func size_t buf_recv(buf_t *buf, const int socket, const
 *  	 size_t expected, const int flags)
 * @brief Receive from socket into buffer
 * @param buf_t * buf Buffer to save received data
 * @param const int socket Opened socket
 * @param const size_t expected How many bytes expected
 * @param const int flags Flags to pass to recv() function
 * @return ssize_t Number of received bytes
 * 	EINVAL if buf is NULL, else returns status of recv() function
 */
extern size_t buf_recv(/*@temp@*/ /*@in@*/ /*@special@*/buf_t *buf, const int socket, const buf_s64_t expected, const int flags);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Load content of the file into buf_t
 * @param const char* filename Full name of file to load into
 *  			buf_t
 * @return buf_t* Buffer containin the contentof the file
 * @details 
 */
extern buf_t *buf_from_file(const char *filename);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Save content of the buffer to the givel file
 * @param buf_t* buf   Buffer to save
 * @param buf_t* file  Buffer containing name of the file to
 *  		   save tje buffer
 * @param mode_t mode  Mode of the file; after the file created
 *  			 and content of the buf_t is saved,
 *  			 this function will change the file permission
 *  			 accordingly to the given 'mode'
 * @return int OK on success, a negative value on an error
 * @details See 'man fchmod' for mode_t format
 */
extern int buf_to_file(buf_t *buf, buf_t *file, mode_t mode);

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
