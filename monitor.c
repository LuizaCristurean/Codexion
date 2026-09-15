#include "codexion.h"

// Returns 1 once every one of the n coders has completed at
// least `required` compiles.
static int	all_done(t_coder *coder, int n, int required)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_mutex_lock(&coder[i].lock);
		if (coder[i].compiles_done < required)
		{
			pthread_mutex_unlock(&coder[i].lock);
			return (0);
		}
		pthread_mutex_unlock(&coder[i].lock);
		i++;
	}
	return (1);
}

// Scans coders in id order; logs and returns 1 for the first one
// whose time since its last compile start exceeds time_to_burnout.
static int	check_burnout(t_monitor_args *args)
{
	int		i;
	long	now;
	long	elapsed;

	i = 0;
	now = get_time_ms() - args->shared->start_time;
	while (i < args->config->number_of_coders)
	{
		pthread_mutex_lock(&args->coders[i].lock);
		elapsed = now - args->coders[i].last_compile_start;
		pthread_mutex_unlock(&args->coders[i].lock);
		if (elapsed > args->config->time_to_burnout)
		{
			log_state(&args->coders[i], "burned out");
			return (1);
		}
		i++;
	}
	return (0);
}

// Runs on its own thread, polling every 500us: stops the
// simulation as soon as a burnout is detected or every coder has
// compiled enough times.
void	*monitor_routine(void *arg)
{
	t_monitor_args	*args;

	args = (t_monitor_args *)arg;
	while (is_stopped(args->shared) == 0)
	{
		if (check_burnout(args))
			request_stop(args->shared);
		else if (all_done(args->coders, args->config->number_of_coders, args->config->number_of_compiles_required))
			request_stop(args->shared);
		usleep(500);
	}
	return (NULL);
}
