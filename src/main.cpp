
#include "Core.hpp"

int			main(int argc, char **argv)
{
	Core	core;

	if (!core.init())
		return (0);
	core.loop();
	(void)argc;
	(void)argv;
	return (1);
}
