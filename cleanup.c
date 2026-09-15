#include "codexion.h"

// Destroys every dongle's mutex and frees the dongles array.
static void	destroy_dongles(t_dongle *dongles, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_mutex_destroy(&dongles[i].lock);
		i++;
	}
	free(dongles);
}

// Destroys every coder's mutex and frees the coders array.
static void	destroy_coders(t_coder *coders, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_mutex_destroy(&coders[i].lock);
		i++;
	}
	free(coders);
}

// Frees the request queue and destroys every shared mutex/condvar.
static void	destroy_shared(t_shared *shared)
{
	free(shared->queue.requests);
	pthread_mutex_destroy(&shared->queue.lock);
	pthread_cond_destroy(&shared->queue.cond);
	pthread_mutex_destroy(&shared->log_lock);
	pthread_mutex_destroy(&shared->stop_lock);
}

// Frees every resource allocated during setup, once all threads
// have finished.
void	cleanup(t_config *config, t_shared *shared, t_dongle *dongles, t_coder *coders)
{
	destroy_dongles(dongles, config->number_of_coders);
	destroy_coders(coders, config->number_of_coders);
	destroy_shared(shared);
}
