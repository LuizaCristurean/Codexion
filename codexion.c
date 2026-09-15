#include "codexion.h"

// Entry point: validate args, build the config, set up shared state,
// run the simulation, then free everything.
int	main(int ac, char **av)
{
	t_config    config;
	t_shared    shared;
	t_dongle    *dongles;
	t_coder     *coders;

	if (arg_checker(ac, av) == 0)
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	fill_config(av, &config);
	if (setup(&config, &shared, &dongles, &coders) == 0)
		return (1);
	run_simulation(&config, &shared, coders);
	cleanup(&config, &shared, dongles, coders);
	return (0);
}
