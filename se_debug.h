#ifndef _SEC_DEBUG_H_
#define _SEC_DEBUG_H_
/*@-skipposixheaders@*/
#include <stdio.h>
#include <time.h>
/*@=skipposixheaders@*/

#ifdef DEBUG3
#undef DEBUG2
#define DEBUG2
#endif

#ifdef DEBUG2
#undef DEBUG
#define DEBUG
#endif

#ifdef DDD
	#undef DDD
#endif
#ifdef DD
	#undef DD
#endif
#ifdef D
	#undef D
#endif
#ifdef DE
	#undef DE
#endif
#ifdef DDE
	#undef DDE
#endif
#ifdef DDD0
	#undef DDD0
#endif

#define _D_PRINT(fmt, ...) do{printf("%s +%d : ", __func__, __LINE__); printf(fmt, ##__VA_ARGS__); }while(0 == 1)
#define _D_PRINT_ERR(fmt, ...) do{fprintf(stderr, "%s +%d [ERR] : ", __func__, __LINE__); printf(fmt, ##__VA_ARGS__); }while(0 == 1)

/* D and DE print */
#ifdef DEBUG
	#define DE _D_PRINT_ERR
	#define D _D_PRINT
#else
	#define DE(x,...) do{}while(0)
	#define D(x,...) do{}while(0)
#endif

/* DD and DDE print */
#ifdef DEBUG2
	#define DDE _D_PRINT_ERR
	#define DD _D_PRINT
#else
	#define DDE(x,...) do{}while(0)
	#define DD(x,...) do{}while(0)
#endif

/* DD and DDE print */
#ifdef DEBUG3
	#define DDDE _D_PRINT_ERR
	#define DDD _D_PRINT
#else
	#define DDDE(x,...) do{}while(0)
	#define DDD(x,...) do{}while(0)
#endif

/* Now, if we want to enable only error prints */
#ifdef DERROR
	#define DE _D_PRINT_ERR
#endif
#ifdef DERROR2
	#define DDE _D_PRINT_ERR
#endif
#ifdef DERROR3
	#define DDDE _D_PRINT_ERR
#endif

/* D0 and DE0 are always empty : use it to keep printings for just in case you need it some day */
#define D0(x,...) do{}while(0)
#define DD0(x,...) do{}while(0)
#define DDD0(x,...) do{}while(0)
#define DE0(x,...) do{}while(0)

#define ENTRY() DD("Entry to the function %s\n", __func__)

/* This used to test and print time of execution in the same function; if you copy thism don't forget include <time.h> */
#define D_TIME_START(x) clock_t x##_start = clock();
#define D_TIME_END(x) {clock_t x##_end = clock(); D("Line %d: time used %f\n", __LINE__, (((double)(x##_end - x##_start))/ CLOCKS_PER_SEC));}
#define D_TIME_END_MES(x, mes) {clock_t x##_end = clock(); D("%s: Line %d: time used %f\n", mes, __LINE__, (((double)(x##_end - x##_start))/ CLOCKS_PER_SEC));}

/* Start time measurement */
#define DD_TIME_START(x) clock_t x##_start = clock()
/* Stop time measurement */
#define DD_TIME_END(x) clock_t x##_end = clock()
/* Calulate the time */
#define DD_TIME_RESULT(x) ((((double)(x##_end - x##_start))/ CLOCKS_PER_SEC));
/* Calculate percent of 'fraction' time in the 'whole' time of the running */
#define DD_PERCENT_OF(fraction, whole) ((int) ((fraction * 100) / whole))

#define DSVAR(x) do{DD("%s = |%s|\n", #x, x);}while(0);
#define DIVAR(x) do{DD("%s = |%d|\n", #x, x);}while(0);

#endif /* _SEC_DEBUG_H_ */
