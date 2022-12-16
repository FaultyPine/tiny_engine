// Copyright 2011-2022 Molecular Matters GmbH, all rights reserved.

#pragma once

#include "LPP_API_Helpers.h"


// ------------------------------------------------------------------------------------------------
// WINDOWS-SPECIFIC DEFINITIONS
// ------------------------------------------------------------------------------------------------

#define LPP_PLATFORM_LIBRARY_PREFIX_ANSI	""
#define LPP_PLATFORM_LIBRARY_PREFIX			L""
#define LPP_PLATFORM_LIBRARY_NAME_ANSI		"\\Agent\\x64\\LPP_Agent_x64_CPP.dll"
#define LPP_PLATFORM_LIBRARY_NAME			L"\\Agent\\x64\\LPP_Agent_x64_CPP.dll"

LPP_NAMESPACE_BEGIN

// Type of a Live++ agent module.
typedef HMODULE LppAgentModule;

// Invalid Live++ agent module.
#define LPP_INVALID_MODULE				LPP_NULL

LPP_NAMESPACE_END

// Linker pseudo-variable representing the DOS header of the module we're being compiled into.
// See Raymond Chen's blog ("Accessing the current module's HINSTANCE from a static library"):
// https://blogs.msdn.microsoft.com/oldnewthing/20041025-00/?p=37483
EXTERN_C IMAGE_DOS_HEADER __ImageBase;


// ------------------------------------------------------------------------------------------------
// WINDOWS-SPECIFIC API
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

LPP_API HMODULE LppPlatformLoadLibraryANSI(const char* const name)
{
	return LoadLibraryA(name);
}

LPP_API HMODULE LppPlatformLoadLibrary(const wchar_t* const name)
{
	return LoadLibraryW(name);
}

LPP_API void LppPlatformUnloadLibrary(HMODULE module)
{
	FreeLibrary(module);
}

LPP_API FARPROC LppPlatformGetFunctionAddress(HMODULE module, const char* const name)
{
	return GetProcAddress(module, name);
}

LPP_API const char* LppPlatformGetCurrentModulePathANSI(void)
{
	static char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(LPP_REINTERPRET_CAST(HMODULE)(&__ImageBase), path, MAX_PATH);

	return path;
}

LPP_API const wchar_t* LppPlatformGetCurrentModulePath(void)
{
	static wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(LPP_REINTERPRET_CAST(HMODULE)(&__ImageBase), path, MAX_PATH);

	return path;
}

LPP_API bool LppPlatformIsValidLibrary(HMODULE module)
{
	return (module != LPP_NULL);
}

LPP_NAMESPACE_END


#include "LPP_API_Version_x64_CPP.h"
#include "LPP_API.h"
