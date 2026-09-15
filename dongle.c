#include "codexion.h"

// Locks the coder's dongle(s) in a fixed order (lowest id first) so
// two coders locking the same pair can never deadlock. With a single
// coder, left and right are the same dongle, so it is locked once.
void	lock_dongles(t_coder *coder)
{
	if (coder->left == coder->right)
	{
		pthread_mutex_lock(&coder->left->lock);
		return ;
	}
	if (coder->left->id < coder->right->id)
	{
		pthread_mutex_lock(&coder->left->lock);
		pthread_mutex_lock(&coder->right->lock);
	}
	else
	{
		pthread_mutex_lock(&coder->right->lock);
		pthread_mutex_lock(&coder->left->lock);
	}
}

// Unlocks the coder's dongle(s) (see lock_dongles for the single-
// dongle case).
void	unlock_dongles(t_coder *coder)
{
	pthread_mutex_unlock(&coder->left->lock);
	if (coder->left != coder->right)
		pthread_mutex_unlock(&coder->right->lock);
}

// Returns 1 if both of the coder's dongles are currently unheld
// and their cooldown has fully elapsed.
static int	dongles_free(t_coder * coder)
{
	long	now;

	now = get_time_ms() - coder->shared->start_time;
	if (coder->left->in_use || now - coder->left->released_at < coder->config->dongle_cooldown)
		return (0);
	if (coder->right->in_use || now - coder->right->released_at < coder->config->dongle_cooldown)
		return (0);
	return (1);
}

// A single attempt to grab both dongles: allowed only if no
// higher-priority pending request wants one of the same dongles
// (is_front) and both dongles are actually free (dongles_free).
int	try_acquire(t_coder *coder)
{
	if (is_front(&coder->shared->queue, coder) == 0)
		return (0);
	lock_dongles(coder);
	if (dongles_free(coder) == 0)
	{
		unlock_dongles(coder);
		return (0);
	}
	coder->left->in_use = 1;
	coder->right->in_use = 1;
	unlock_dongles(coder);
	return (1);
}

// Registers this coder's request in the priority queue, waits
// (retrying with cooldown-aware timeouts via wait_for_dongles)
// until it can grab both dongles or the simulation stops, then
// removes its own request from the queue either way.
int	acquire_dongles(t_coder *coder)
{
	t_request	request;
	int			acquired;

	request.coder = coder;
	request.arrival_time = get_time_ms() - coder->shared->start_time;
	request.deadline = coder->last_compile_start + coder->config->time_to_burnout;
	pthread_mutex_lock(&coder->shared->queue.lock);
	queue_push(&coder->shared->queue, request, coder->config->scheduler);
	acquired = wait_for_dongles(coder);
	if (acquired && is_stopped(coder->shared))
	{
		lock_dongles(coder);
		coder->left->in_use = 0;
		coder->right->in_use = 0;
		unlock_dongles(coder);
		acquired = 0;
	}
	queue_remove(&coder->shared->queue, coder, coder->config->scheduler);
	pthread_mutex_unlock(&coder->shared->queue.lock);
	return (acquired);
}
