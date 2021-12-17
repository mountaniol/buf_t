#ifndef _BUF_T_STRING_H_
#define _BUF_T_STRING_H_

extern ret_t buf_str_is_valid(buf_t *buf);

/**
 * @author Sebastian Mountaniol (16/06/2020)
 * @func buf_t* buf_string(size_t size)
 * @brief Allocate buffer and mark it as STRING
 * @param size_t size
 * @return buf_t*
 */
extern /*@null@*/ buf_t *buf_string(buf_usize_t size);

/**
 * @author Sebastian Mountaniol (18/06/2020)
 * @func buf_t* buf_from_string(char *str, size_t size_without_0)
 * @brief Convert given string "str" into buf_t. The resulting buf_t is a nirmal STRING type buffer.
 * @param char * str String to convert
 * @param size_t size_without_0 Length of the string without terminating '\0'
 * @return buf_t* New buf_t containing the "str"
 */
extern /*@null@*/ buf_t *buf_from_string(/*@null@*/char *str, const buf_usize_t size_without_0);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_str_add(buf_t *buf, const char *new_data, const buf_usize_t size)
 * 
 * @brief Add data to the string buffer
 * @param buf Pointer to the string buffer
 * @param new_data - New data to add (Unux string)
 * @param size - Size of the data to add
 * 
 * @return ret_t 
 * @details 
 */
extern ret_t buf_str_add(/*@null@*/buf_t *buf, /*@null@*/const char *new_data, const buf_usize_t size);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_str_detect_used(buf_t *buf)
 * 
 * @brief Experimental: detect used length of string buffer
 * @param buf - Buffer to detect string length
 * 
 * @return ret_t OK on success, 
 * @details 
 */
extern ret_t buf_str_detect_used(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_str_pack(buf_t *buf)
 * 
 * @brief String-specific buffer pack function 
 * @param buf - Buffer to pack
 * 
 * @return ret_t OK on success, not OK on a failure 
 * @details 
 */
extern ret_t buf_str_pack(/*@null@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (02/06/2020)
 * @func buf_t* buf_sprintf(char *format, ...)
 * @brief sprintf into buf_t
 * @param char * format Format (like in printf first argument )
 * @return buf_t*
 */
extern buf_t *buf_sprintf(const char *format, ...);

/**
 * @author Sebastian Mountaniol (12/17/21)
 * ret_t buf_str_concat(buf_t *dst, buf_t *src)
 * 
 * @brief Concat two buf_t buffers
 * @param dst - Buffer to add string from src
 * @param src - Buffer to copy from
 * 
 * @return ret_t - OK on success, EINVAL if the buffer is NULL,
 *  	   BAD if a buffer is not the string buffer
 * @details 
 */
extern ret_t buf_str_concat(buf_t *dst, buf_t *src);
#endif /* _BUF_T_STRING_H_ */
