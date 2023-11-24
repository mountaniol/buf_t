#ifndef _BUF_T_ARRAY_USED_H_
#define _BUF_T_ARRAY_USED_H_

#include "buf_t_structs.h"

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Get number of members kept in this buf_t
 * @param buf_t* buf   buf_t buffer to get numbet of members
 * @return buf_s32_t Number of members
 * @details 
 */
buf_s32_t buf_arr_get_memberss_count(buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Set number of members in the array
 * @param buf_t* buf        Buffer to set number of members
 * @param buf_s32_t new_members Number of members to set
 * @return ret_t OK on syccess, a negative on an error
 * @details Number of members must by 0 or it should be multiple
 *  		of buf->arr.size; otherwise an error returned
 */
ret_t buf_set_arr_members_count(buf_t *buf, buf_s32_t new_members);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Return size of a member in buf_t->arr
 * @param buf_t* buf   Array buffer to get size of an array
 *  		   member
 * @return buf_s32_t Size of an member of the array buffer;
 *  	   On an error a negative value returned
 * @details 
 */
buf_s32_t buf_arr_get_member_size(buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Set array memeber size
 * @param buf_t* buf     Buffer to set a member size
 * @param buf_s32_t new_size New size of the array member
 * @return ret_t OK on success
 * @details This is a function for a raw manupulation with a
 *  		array buffer.
 */
ret_t buf_set_arr_member_size(buf_t *buf, buf_s32_t new_size);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Calcualate used memory
 * @param buf_t* buf   Array buffer to calculate used memory
 * @return buf_s64_t Calculated used memory
 * @details This function returns number of members * size of
 *  		one mebeber
 */
buf_s64_t buf_arr_get_used(buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Set used space; this function used internally and also
 *  	  for raw manipulation on the buffer
 * @param buf_t* buf     
 * @param buf_s64_t new_used
 * @return ret_t 
 * @details This function can be used only when a size of a
 *  		member in the array is set
 */
ret_t buf_arr_set_used(buf_t *buf, buf_s64_t new_used);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Test different condition to detect wheather the buffer
 *  	  valid or invalid. 
 * @param buf_t* buf   Buffer to test
 * @return ret_t OK on success; a negative value on an error
 * @details 
 */
ret_t buf_array_is_valid(/*@in@*//*@temp@*/buf_t *buf);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Allocate new array buffer. Allocate memory for
 *  	  'num_of_members' members of 'member_size' bytes each
 * @param buf_s32_t member_size   Size of a single member,
 *  				in bytez
 * @param buf_s32_t num_of_members Number of memebrs
 * @return buf_t* Returns allocated buffer; on error,
 *  	   NULL pointer returned
 * @details 
 */
/*@null@*/ buf_t *buf_array(buf_s32_t member_size, buf_s32_t num_of_members);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Add several members at once to the buf aray
 * @param buf_t* buf   buf_t buffer to add members into
 * @param const void* new_data_ptr      Mmeory buffer containing
 *  			members to add into buf_t
 * @param const buf_s32_t num_of_new_members Number of members
 *  			should be added into buf_t
 * @return ret_t OK on success
 * @details 
 */
ret_t buf_arr_add_members(buf_t *buf, const void *new_data_ptr, const buf_s32_t num_of_new_members);

/**
 * @author Sebastian Mountaniol (11/19/23)
 * @brief Add new members as a memory with size, not number of
 *  	  members
 * @param buf_t* buf     Buffer to add new mebers as a memory
 * @param const char* new_data Pointer to memory containing
 *  			members to be added
 * @param const buf_s64_t size    Size of the mmeory in bytes
 * @return ret_t BUFT_OK on success, arror code on faulure
 * @details The size of the memory to add must be exact multiply
 *  		of one memeber size
 */
extern ret_t buf_arr_add_memory(buf_t *buf, /*@temp@*//*@in@*/const char *new_data, const buf_s64_t size);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Add one member to the beffer
 * @param buf_t* buf         Buffer to add a new member
 * @param const void* new_data_ptr Pointer to memory where new
 *  			data is kept
 * @return ret_t OK on success, a negative value on an error
 * @details 
 */
ret_t buf_arr_add(buf_t *buf, const void *new_data_ptr);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Remove several members from the array buffer
 * @param buf_t* buf               Buffer to remove members
 * @param const buf_s32_t from_member       Index of the first
 *  			meber to remove; The first member has index 0
 * @param const buf_s32_t num_of_new_members How mamny members
 *  			to remove. Minumum is 1, maximum is up to the
 *  			last member
 * @return ret_t OK on success
 * @details Pay attention: after members are removed,
 *  		the rest of members are shifted to close the gap.
 *  		For example: if there are 3 members [0],[1],[2],[3]
 *  		and members [1],[2] are removed, than after this
 *  		operation member [3] becomes member [1]
 */
ret_t buf_arr_rm_members(buf_t *buf, const buf_s32_t from_member, const buf_s32_t num_of_new_members);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Remove one member from the array buffer
 * @param buf_t* buf         Buffer to remove one member
 * @param const buf_s32_t member_index Index of member to
 *  			remove; the first member has 0 index
 * @return ret_t OK on success, a negative value on an error
 * @details This function is a wrapper of the
 *  		buf_arr_rm_members(); all restructions an behavoir
 *  		applied.
 */
ret_t buf_arr_rm(buf_t *buf, const buf_s32_t member_index);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Set new 'used' value
 * @param buf_t* buf   Buffer to set 'used' value
 * @param buf_s64_t used  New 'used' value
 * @return ret_t OK on success
 * @details Be aware: this function doesn't reallocate data;
 *  		it only sets ne value of the '->used' field.
 */
ret_t buf_array_set_used(/*@in@*//*@temp@*/buf_t *buf, buf_s64_t used);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Return pointer to an array member
 * @param buf_t* buf         Buffer to get pointer to a member
 *  		   of the internal array
 * @param const buf_s32_t member_index Index of the mmeber to
 *  			get pointer to
 * @return void* Pointer to the start of memory of the memner;
 *  	   NULL on an error
 * @details The index must be >= 0 and <= the last member 
 */
void *buf_arr_get_member_ptr(buf_t *buf, const buf_s32_t member_index);

/**
 * @author Sebastian Mountaniol (11/18/23)
 * @brief Copy the member by given index to the 'dest' buffer
 * @param buf_t* buf         Array buffer to copy a member from
 * @param const buf_s32_t member_index Index of the mmeber
 * @param void* dest        Buffer to copy the mmeber to
 * @param buf_s32_t dest_memory_size - Size of the destination
 *  				buffer, must be >= size of the member
 * @return ret_t OK on success, a negative error code on an
 *  	   error
 * @details 
 */
ret_t buf_arr_member_copy(buf_t *buf, const buf_s32_t member_index, void *dest, buf_s32_t dest_memory_size);

/**
 * @author Sebastian Mountaniol (11/20/23)
 * @brief Clean the array buffer; the allocated memory is
 *  	  released.
 * @param buf_t* buf   Buffer to clean
 * @return ret_t BUFT_OK on success, a gegative error code on an
 *  	   error
 * @details 
 */
extern ret_t buf_arr_clean(/*@temp@*//*@in@*//*@special@*/buf_t *buf);
#endif /* _BUF_T_ARRAY_USED_H_ */
