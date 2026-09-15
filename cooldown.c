#include "codexion.h"

// Builds an absolute deadline (now + dongle_cooldown) for
// pthread_cond_timedwait, so a waiting coder wakes up on its own
// once cooldown expires, even with no broadcast from anyone.
static void	compute_deadline(t_coder *coder, struct timespec *ts)
{
	struct timeval	tv;
	long			wait_ms;
	long			nsec;

	wait_ms = coder->config->dongle_cooldown;
	if (wait_ms < 1)
		wait_ms = 1;
	gettimeofday(&tv, NULL);
	nsec = tv.tv_usec * 1000 + (wait_ms % 1000) * 1000000;
	ts->tv_sec = tv.tv_sec + wait_ms / 1000 + nsec / 1000000000;
	ts->tv_nsec = nsec % 1000000000;
}

// Retries try_acquire, sleeping on the queue's condition variable
// with a timeout, until it succeeds or the simulation stops.
int	wait_for_dongles(t_coder *coder)
{
	struct timespec	ts;
	int				acquired;

	acquired = try_acquire(coder);
	while (acquired == 0 && is_stopped(coder->shared) == 0)
	{
		compute_deadline(coder, &ts);
		pthread_cond_timedwait(&coder->shared->queue.cond, &coder->shared->queue.lock, &ts);
		acquired = try_acquire(coder);
	}
	return (acquired);
}
