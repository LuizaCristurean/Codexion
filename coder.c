#include "codexion.h"

// Records the start of a new compile (this resets the coder's
// burnout deadline), logs it, sleeps time_to_compile, then counts
// the compile as done.
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

// Logs the debugging phase and sleeps for time_to_debug.
static void	do_debug(t_coder *coder)
{
	coder->status = DEBUGGING;
	log_state(coder, "is debugging");
	usleep(coder->config->time_to_debug * 1000);
}

// Logs the refactoring phase and sleeps for time_to_refactor.
static void	do_refactor(t_coder *coder)
{
	coder->status = REFACTORING;
	log_state(coder, "is refactoring");
	usleep(coder->config->time_to_refactor * 1000);
}

// A coder's whole life cycle: acquire both dongles, compile,
// release them, debug, refactor, repeat. Checks the stop flag
// after every phase so it never starts a new phase once the
// simulation has been told to stop.
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
		if (is_stopped(coder->shared))
			break;
		do_debug(coder);
		if (is_stopped(coder->shared))
			break;
		do_refactor(coder);
	}
	return (NULL);
}
