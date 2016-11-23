/*
 * ICC_time.h
 *
 *  Created on: Nov 10, 2016
 *      Author: vcvv001
 */

#ifndef ICC_TIME_H
#define ICC_TIME_H

//#define AUTO_POLL_TIME

#define DEFAULT_TIMEOUT_MS	10  // 1 jiffy (msleep_interruptible works on jiffy resolution)
#define MAX_TIMEOUT_NS	(DEFAULT_TIMEOUT_MS * NSEC_PER_MSEC)
#define MIN_TIMEOUT_NS	100
#define AVG_TIMEOUT_NS	400000

#include <linux/time.h>
#include <linux/delay.h>
#include <asm/atomic.h>

#ifdef AUTO_POLL_TIME

#define TIMEOUT_GRAN	10
#define MAX_IDLE_COUNT	20  // idle counts after which timeout gets multiplied
#define MULTIPLY_ON_IDLE(val)	((val) <<= 4) /* mul by 16 */
#define SHOULD_UPDATE_TIMEOUT(exec_count)	((uint32_t)exec_count & 0xFFFFFFC) /* every 4 executions */
#define EPSILON(count)	(count >> 10) /* count / 1024  -> basically 1024 time units (ns) -> on us level */

struct icc_timer_t {
	/* supports ns level */
	uint64_t count_ns;
	uint32_t count_exec;
	atomic_t count_idle;
	atomic_t timeout_ns;
};

static struct icc_timer_t icc_timer = {
	0, 0, 0, MAX_TIMEOUT_NS
};

#endif

/* is it worth making these functions entirely atomic ? */
void timer_sleep(void)
{
#ifdef AUTO_POLL_TIME
	uint32_t crt_timeout = atomic_read(&icc_timer.timeout_ns);
	uint32_t count_idle = atomic_read(&icc_timer.count_idle);

	count_idle++;
	if ((count_idle > MAX_IDLE_COUNT) && (crt_timeout < MAX_TIMEOUT_NS)) {
		count_idle = 0;
		MULTIPLY_ON_IDLE(crt_timeout);
		if (crt_timeout >= MAX_TIMEOUT_NS) {
			crt_timeout = MAX_TIMEOUT_NS;
		}
		atomic_set(&icc_timer.timeout_ns, crt_timeout);
	}

	atomic_set(&icc_timer.count_idle, count_idle);
#else
	static uint32_t crt_timeout = AVG_TIMEOUT_NS;
#endif

	if (crt_timeout >= MAX_TIMEOUT_NS) {
		msleep_interruptible(DEFAULT_TIMEOUT_MS);
	} else if (crt_timeout >> 10) {
		usleep_range(crt_timeout >> 10, crt_timeout >> 8); /* sort-of-transform ns to us */
	} else {
		// timeout too small
		usleep_range(1, 10);  /* these are us, do not use MIN_TIMEOUT_NS */
	}
}


void ICC_Timer_Update_ns(uint64_t new_exec_time_ns)
{
#ifdef AUTO_POLL_TIME
	uint32_t crt_count_exec = icc_timer.count_exec;
	uint64_t new_timeout = icc_timer.count_ns;

	atomic_set(&icc_timer.count_idle, 0);
	icc_timer.count_ns += new_exec_time_ns;

	if (SHOULD_UPDATE_TIMEOUT(crt_count_exec)) { /* either 0 or multiple of the fixed rate */
		uint32_t crt_timeout = 0;
		if (!crt_count_exec) {
			new_timeout = new_exec_time_ns;
			icc_timer.count_ns = new_exec_time_ns;
		}
		else {
			crt_timeout = atomic_read(&icc_timer.timeout_ns);

			new_timeout += new_exec_time_ns;
			new_timeout /= (crt_count_exec + 1);
		}

		new_timeout /= TIMEOUT_GRAN;
		if (new_timeout > MAX_TIMEOUT_NS) {
			new_timeout = MAX_TIMEOUT_NS;
		}
		if (!new_timeout) {
			new_timeout = MIN_TIMEOUT_NS;
		}
		if (EPSILON(abs(new_timeout - crt_timeout))) {
			atomic_set(&icc_timer.timeout_ns, (int)new_timeout);
			//printk(KERN_ALERT "Updated poll time to %d ns\n", (int)new_timeout);
		}

	}

	icc_timer.count_exec++;
#endif
}
EXPORT_SYMBOL(ICC_Timer_Update_ns);


void ICC_Sleep(void)
{
	timer_sleep();
}

#endif /* ICC_TIME_H */
