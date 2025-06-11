#pragma once

// BEGIN Library export
#define API_NAME(func) func##_func_ptr_t

#define API_DECLARE_FUNC(return_type, func, ...)\
	API_FUNCTION(return_type, func, __VA_ARGS__)\
	inline API_NAME(func) func##_ptr;\
	extern "C" { \
	SHARED_LIBRARY_API return_type func##_api(__VA_ARGS__); }\


#define API_FUNCTION(return_type, func, ...)\
	typedef return_type (*API_NAME(func))(__VA_ARGS__); \

#ifdef _WIN32
#include <windows.h>
#include <iostream>

#ifdef _DEBUG
#define API_LOAD_DLL(dllName)\
	dll = LoadLibrary(#dllName"-d.dll");\
	if (!dll) {\
		std::cerr << #dllName " - Failed to load DLL!" << std::endl;\
		return 1;\
	}\
	loaded = true;
#else
#define API_LOAD_DLL(dllName)\
	dll = LoadLibrary(#dllName".dll");\
	if (!dll) {\
		std::cerr << #dllName " - Failed to load DLL!" << std::endl;\
		return 1;\
	}\
	loaded = true;
#endif

#define API_DEFINE_FUNC(func)\
	func##_ptr = (API_NAME(func))GetProcAddress(dll, #func"_api");\
    if (!(func##_ptr)) {\
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
// END Library export
