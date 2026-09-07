#include "codexion.h"

int is_stopped(t_shared *shared)
{
	int stopped;

	pthread_mutex_lock(&shared->stop_lock);
	stopped = shared->stop;
	pthread_mutex_unlock(&shared->stop_lock);
	return (stopped);
}

void	request_stop(t_shared *shared)
{
	pthread_mutex_lock(&shared->stop_lock);
	shared->stop = 1;
	pthread_mutex_unlock(&shared->stop_lock);
	pthread_mutex_lock(&shared->queue.lock);
	pthread_cond_broadcast(&shared->queue.cond);
	pthread_mutex_unlock(&shared->queue.lock);
}