#include "codexion.h"

static int	create_coder_threads(t_coder *coders, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]) != 0)
			return (0);
		i++;
	}
	return (1);
}

static void	join_coder_threads(t_coder *coders, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}
}

int	run_simulation(t_config *config, t_shared *shared, t_coder *coders)
{
	t_monitor_args margs;

	margs.config = config;
	margs.shared = shared;
	margs.coders = coders;
	if (pthread_create(&shared->monitor, NULL, monitor_routine, &margs) != 0)
		return (0);
	if (create_coder_threads(coders, config->number_of_coders) == 0)
		return (0);
	join_coder_threads(coders, config->number_of_coders);
	pthread_join(shared->monitor, NULL);
	return (1);
}