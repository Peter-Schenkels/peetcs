#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#define API_NAME(func) func##_func_ptr_t
#define API_DECLARE_FUNC(return_type, func, ...)\
	API_FUNCTION(return_type, func, __VA_ARGS__)\
	API_NAME(func) func;\
	extern "C" { \
	SHARED_LIBRARY_API return_type func##_api(__VA_ARGS__); }\

#define API_FUNCTION(return_type, func, ...)\
	typedef return_type (*API_NAME(func))(__VA_ARGS__); \

#ifdef _WIN32
#include <windows.h>
#include <iostream>

#ifdef _DEBUG
#define API_LOAD_DLL()\
	auto dll = LoadLibrary("SHARED-d.dll");\
	if (!dll) {\
		std::cerr << "Failed to load DLL!" << std::endl;\
		return 1;\
	}
#else
#define API_LOAD_DLL()\
	auto dll = LoadLibrary("SHARED.dll");\
	if (!dll) {\
		std::cerr << "Failed to load DLL!" << std::endl;\
		return 1;\
	}
#endif

#define API_DEFINE_FUNC(func)\
	func = (API_NAME(func))GetProcAddress(dll, #func"_api");\
    if (!(func)) {\
		std::cerr << "Failed to get function!" << std::endl;\
		FreeLibrary(dll);\
		return 1;\
	}\

#ifdef SHARED_LIBRARY_EXPORTS
#define SHARED_LIBRARY_API __declspec(dllexport)
#else
#define SHARED_LIBRARY_API __declspec(dllimport)
#endif
#else
// TODO Linux / macos SO alternative 
#define SHARED_LIBRARY_API
#endif

API_DECLARE_FUNC(void, hello)

int load_shared_lib()
{
	API_LOAD_DLL()
	API_DEFINE_FUNC(hello)
	return 0;
}


#endif