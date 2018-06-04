#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys/time.h>

static inline uint64_t getTimer(void)
{
	struct timeval tv_date;
	gettimeofday( &tv_date, NULL );
	return( (uint64_t) tv_date.tv_sec * 1000000 + (uint64_t) tv_date.tv_usec );
}

#endif /* __TIMER_H__ */
