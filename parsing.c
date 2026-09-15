#include "codexion.h"

// Returns 1 if str is a non-empty sequence of digits, 0 otherwise.
static	int  is_valid_number(char *str)
{
	int	i;

	i = 0;
	if (str[i] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] >= '0' && str[i] <= '9')
			i++;
		else
			return (0);
	}
	return (1);
}

// Validates argc/argv: exactly 8 numeric arguments, then a scheduler
// name that must be "fifo" or "edf".
int arg_checker(int ac, char **av)
{
    int i;

    i = 1;
    if (ac != 9)
        return (0);
    while (i < 8)
    {
        if (is_valid_number(av[i]) == 1)
            i++;
        else
            return (0);
    }
    return (strcmp(av[i], "fifo") == 0 || strcmp(av[i], "edf") == 0);
}

// Converts the already-validated argv strings into t_config, used
// read-only by every thread for the rest of the run.
void    fill_config(char **av, t_config *config)
{
    config->number_of_coders = atoi(av[1]);
    config->time_to_burnout = atoi(av[2]);
    config->time_to_compile = atoi(av[3]);
    config->time_to_debug = atoi(av[4]);
    config->time_to_refactor = atoi(av[5]);
    config->number_of_compiles_required = atoi(av[6]);
    config->dongle_cooldown = atoi(av[7]);
    if (strcmp(av[8], "fifo") == 0)
        config->scheduler = FIFO;
    else
        config->scheduler = EDF;
}
