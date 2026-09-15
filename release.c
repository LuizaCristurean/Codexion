#include "codexion.h"

// Marks both of the coder's dongles free again, timestamps the
// release (start of their cooldown window), and wakes up any
// coder currently waiting on the queue.
void	release_dongles(t_coder *coder)
{
	long now;

	pthread_mutex_lock(&coder->shared->queue.lock);
	lock_dongles(coder);
	now = get_time_ms() - coder->shared->start_time;
	coder->left->in_use = 0;
	coder->left->released_at = now;
	coder->right->in_use = 0;
	coder->right->released_at = now;
	unlock_dongles(coder);
	pthread_cond_broadcast(&coder->shared->queue.cond);
	pthread_mutex_unlock(&coder->shared->queue.lock);
}
