#include "codexion.h"

// Current wall-clock time in milliseconds (absolute, via gettimeofday).
long	get_time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

// Allocates the n dongles: all free, never used, and not in any
// phantom cooldown (released_at starts far in the past).
static t_dongle	*init_dongles(int n)
{
    t_dongle	*dongles;
    int			i;

    dongles = malloc(sizeof(t_dongle) * n);
    if (dongles == NULL)
		return (NULL);
    i = 0;
    while (i < n)
    {
    	dongles[i].id = i;
    	dongles[i].in_use = 0;
    	dongles[i].released_at = -1000000;
    	pthread_mutex_init(&dongles[i].lock, NULL);
    	i++;
    }
    return (dongles);
}

// Allocates the n coders seated in a circle: coder i owns dongles[i]
// as its right dongle and dongles[i-1] as its left dongle.
static t_coder	*init_coders(t_dongle *dongles, t_config *config, t_shared *shared)
{
    t_coder	*coders;
    int		i;
    int		n;
    
    n = config->number_of_coders;
    coders = malloc(sizeof(t_coder) * n);
    if (coders == NULL)
    	return (NULL);
    i = 0;
    while (i < n)
    {
    	coders[i].id = i + 1;
    	coders[i].status = REFACTORING;
    	coders[i].last_compile_start = 0;
    	coders[i].compiles_done = 0;
    	coders[i].right = &dongles[i];
    	coders[i].left = &dongles[(i - 1 + n) % n];
    	coders[i].config = config;
    	coders[i].shared = shared;
    	pthread_mutex_init(&coders[i].lock, NULL);
    	i++;
    }
    return (coders);
}

// Allocates the shared priority queue and initializes every shared
// mutex/condvar/stop flag used across coder and monitor threads.
static int init_shared(t_shared *shared, int n)
{
	shared->queue.requests = malloc(sizeof(t_request) * n);
	if (shared->queue.requests == NULL)
		return (0);
	shared->queue.size = 0;
	shared->queue.capacity = n;
	pthread_mutex_init(&shared->queue.lock, NULL);
	pthread_cond_init(&shared->queue.cond, NULL);
	pthread_mutex_init(&shared->log_lock, NULL);
	shared->stop = 0;
	pthread_mutex_init(&shared->stop_lock, NULL);
	return (1);
}

// Runs all the init_* helpers in order and records the simulation's
// absolute start time, used to convert timestamps to relative ms.
int setup(t_config *config, t_shared *shared, t_dongle **dongles, t_coder **coders)
{
    *dongles = init_dongles(config->number_of_coders);
    if (*dongles == NULL)
        return (0);
    if (init_shared(shared, config->number_of_coders) == 0)
        return (0);
    *coders = init_coders(*dongles, config, shared);
    if (*coders == NULL)
        return (0);
    shared->start_time = get_time_ms();
    return (1);
}
