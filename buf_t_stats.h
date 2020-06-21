#ifndef BUF_T_STATS_H
#define BUF_T_STATS_H

/* Define statistics entries */
#define BUF_T_STAT_BUF_ADD  (0)
#define BUF_T_STAT_BUF_FREE (1)

/* This structure keeps per function statistics */
struct buf_t_stat_data
{
	size_t calls; /* How many times this function called */
	size_t eok; /* How many times this function finished with EOK status */
	/* Note: The (calls - ok) = number of errors */
	/* Note: for functions like buf_new, buf_string this number also number of allocated bufs of this
	   type */
	size_t einval; /* How many times the function terminated because invalid argument */
	unsigned int max_time; /* Max running time of this function  */
	float average_time; /* Average time of the function run */
};


void average_buf_size_inc(size_t buf_size);
void buf_allocs_num_inc(void);
void buf_release_num_inc(void);
void buf_regular_num_inc(void);
void buf_string_num_ins(void);
void buf_ro_num_inc();
void max_data_size_upd(size_t data_size);
void data_allocated_inc(size_t allocated);
void data_released_inc(size_t released);
void buf_realloc_calls_inc(void);
void buf_realloc_max_upd(size_t realloced);
void buf_realloc_average_upd(size_t realloced);
void buf_t_stats_print(void);
#endif /* BUF_T_STATS_H */
