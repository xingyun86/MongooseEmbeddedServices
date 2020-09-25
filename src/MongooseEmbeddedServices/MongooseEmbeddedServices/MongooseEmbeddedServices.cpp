// MongooseEmbeddedServices.cpp : Defines the entry point for the application.
//

#include "MongooseEmbeddedServices.h"

int main(int argc, char ** argv)
{
	std::cout << "Hello CMake." << std::endl;
	MES->start_service();
	getchar();
	MES->close_service();
	return 0;
}
