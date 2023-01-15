#include "main.h"

int main(int argc, char* argv[])
{
	std::unique_ptr<python_creator> creator;

	if (argc > 1)
	{
		creator = std::make_unique<python_creator>(argv[1]);
	}
	else
	{
		creator = std::make_unique<python_creator>();
	}
	
	creator->parse_files();
	creator->build_python_client();
	creator->install_python_module();

	return 0;
}