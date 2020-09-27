// ResourceServices.cpp : Defines the entry point for the application.
//

#include "ResourceServices.h"

int main(int argc, char ** argv)
{
	std::cout << "Hello CMake." << std::endl;
	RS->start_service();
	getchar();
	RS->close_service();
	return 0;
}
