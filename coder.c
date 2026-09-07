#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->lock);
	coder->last_compile_start = get_time_ms() - coder->shared->start_time;
	pthread_mutex_unlock(&coder->lock);
	coder->status = COMPILING;
	log_state(coder, "is compiling");
	usleep(coder->config->time_to_compile * 1000);
	pthread_mutex_lock(&coder->lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->lock);
}

static void	do_debug(t_coder *coder)
{
	coder->status = DEBUGGING;
	log_state(coder, "is debugging");
	usleep(coder->config->time_to_debug * 1000);
}

static void	do_refactor(t_coder *coder)
{
	coder->status = REFACTORING;
	log_state(coder, "is refactoring");
	usleep(coder->config->time_to_refactor * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (is_stopped(coder->shared) == 0)
	{
		if (acquire_dongles(coder) == 0)
			break;
		log_state(coder, "has taken a dongle");
		log_state(coder, "has taken a dongle");
		do_compile(coder);
		release_dongles(coder);
		do_debug(coder);
		do_refactor(coder);
	}
	return (NULL);
}