#include "codexion.h"

int	main(int argc, char **argv)
{
	t_args	args;

	if (handle_args(argc, argv, &args))
		return (1);
	return (0);
}
