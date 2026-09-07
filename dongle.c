#include "codexion.h"

void	lock_dongles(t_coder *coder)
{
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

void	unlock_dongles(t_coder *coder)
{
	pthread_mutex_unlock(&coder->left->lock);
	pthread_mutex_unlock(&coder->right->lock);
}

static int	dongles_free(t_coder * coder)
{
	long	now;

	now = get_time_ms();
	if (coder->left->in_use || now - coder->left->released_at < coder->config->dongle_cooldown)
		return (0);
	if (coder->right->in_use || now - coder->right->released_at < coder->config->dongle_cooldown)
		return (0);
	return (1);
}

static int	try_acquire(t_coder *coder)
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

int	acquire_dongles(t_coder *coder)
{
	t_request	request;
	int			acquired;

	request.coder = coder;
	request.arrival_time = get_time_ms() - coder->shared->start_time;
	request.deadline = coder->last_compile_start + coder->config->time_to_burnout;
	pthread_mutex_lock(&coder->shared->queue.lock);
	queue_push(&coder->shared->queue, request, coder->config->scheduler);
	acquired = try_acquire(coder);
	while (acquired == 0 && is_stopped(coder->shared) == 0)
	{
		pthread_cond_wait(&coder->shared->queue.cond, &coder->shared->queue.lock);
		acquired = try_acquire(coder);
	}
	if (acquired)
		queue_pop(&coder->shared->queue, coder->config->scheduler);
	else
		queue_remove(&coder->shared->queue, coder, coder->config->scheduler);
	pthread_mutex_unlock(&coder->shared->queue.lock);
	return (acquired);
}