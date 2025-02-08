#include "tests/shared.hpp"

#include <iostream>

extern "C"
{
	SHARED_LIBRARY_API void hello_api()
	{
		std::cout << "Hello World from SHARED.DLL" << std::endl;
	}
}

