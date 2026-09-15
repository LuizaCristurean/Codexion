#include "codexion.h"

// Prints one "timestamp id message" line, serialized with a
// mutex so lines from different threads never interleave.
void	log_state(t_coder *coder, char *message)
{
	long	now;

	now = get_time_ms() - coder->shared->start_time;
	pthread_mutex_lock(&coder->shared->log_lock);
	printf("%ld %d %s\n", now, coder->id, message);
	pthread_mutex_unlock(&coder->shared->log_lock);
}
