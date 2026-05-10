/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jhoban <jhoban@student.42berlin.de>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 15:55:20 by jhoban            #+#    #+#             */
/*   Updated: 2026/05/10 07:32:04 by jhoban           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_uint(char *str)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	validate_ints(char **argv)
{
	int	i;

	i = 1;
	while (i <= 7)
	{
		if (!is_valid_uint(argv[i]))
			return (0);
		i++;
	}
	return (1);
}

static void	parse_ints(t_args *args, char **argv)
{
	args->number_of_coders = atoi(argv[1]);
	args->time_to_burnout = atoi(argv[2]);
	args->time_to_compile = atoi(argv[3]);
	args->time_to_debug = atoi(argv[4]);
	args->time_to_refactor = atoi(argv[5]);
	args->number_of_compiles_required = atoi(argv[6]);
	args->dongle_cooldown = atoi(argv[7]);
}

static int	parse_scheduler(char *str, t_args *args)
{
	if (strcmp(str, "fifo") == 0)
		args->scheduler = "fifo";
	else if (strcmp(str, "edf") == 0)
		args->scheduler = "edf";
	else
		return (0);
	return (1);
}

int	handle_args(int argc, char **argv, t_args *args)
{
	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s <coders> <burnout> <compile> "
			"<debug> <refactor> <compiles_req> "
			"<cooldown> <fifo|edf>\n", argv[0]);
		return (0);
	}
	if (!validate_ints(argv))
	{
		fprintf(stderr, "Error: invalid integer argument\n");
		return (0);
	}
	parse_ints(args, argv);
	if (args->number_of_coders < 1)
	{
		fprintf(stderr, "Error: number_of_coders must be >= 1\n");
		return (0);
	}
	if (!parse_scheduler(argv[8], args))
	{
		fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'\n");
		return (0);
	}
	return (1);
}
