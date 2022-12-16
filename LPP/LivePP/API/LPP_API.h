// Copyright 2011-2022 Molecular Matters GmbH, all rights reserved.

#pragma once

#include "LPP_API_Hooks.h"
#include "LPP_API_Options.h"
#include "LPP_API_Preferences.h"


// ------------------------------------------------------------------------------------------------
// API MACROS
// ------------------------------------------------------------------------------------------------

// Macros to temporarily enable/disable optimizations
#if defined(__clang__)
#	define LPP_ENABLE_OPTIMIZATIONS				_Pragma("clang optimize on")
#	define LPP_DISABLE_OPTIMIZATIONS			_Pragma("clang optimize off")
#elif defined(_MSC_VER)
#	define LPP_ENABLE_OPTIMIZATIONS				__pragma(optimize("", on))
#	define LPP_DISABLE_OPTIMIZATIONS			__pragma(optimize("", off))
#else
#	error("Live++: Unknown compiler.");
#endif


// ------------------------------------------------------------------------------------------------
// API AGENT FUNCTION TYPEDEFS
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

typedef void LppLogMessageFunctionANSI(const char* const message);
typedef void LppLogMessageFunction(const wchar_t* const message);

typedef void LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunctionANSI(const char* const excludeFilenamesPattern, const char* const includeFilenamesPattern);
typedef void LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunction(const wchar_t* const excludeFilenamesPattern, const wchar_t* const includeFilenamesPattern);

typedef void LppEnableModuleFunctionANSI(const char* const relativeOrFullPath, LppModulesOption options);
typedef void LppEnableModuleFunction(const wchar_t* const relativeOrFullPath, LppModulesOption options);

typedef void LppEnableModulesFunctionANSI(const char* const* const arrayOfRelativeOrFullPaths, size_t count, LppModulesOption options);
typedef void LppEnableModulesFunction(const wchar_t* const* const arrayOfRelativeOrFullPaths, size_t count, LppModulesOption options);

typedef void LppEnableFASTBuildModuleFunctionANSI(const char* const relativeOrFullPath, const char* const relativeOrFullDatabasePath, const char* const targetName);
typedef void LppEnableFASTBuildModuleFunction(const wchar_t* const relativeOrFullPath, const wchar_t* const relativeOrFullDatabasePath, const char* const targetName);

typedef void LppDisableModuleFunctionANSI(const char* const relativeOrFullPath, LppModulesOption options);
typedef void LppDisableModuleFunction(const wchar_t* const relativeOrFullPath, LppModulesOption options);

typedef void LppDisableModulesFunctionANSI(const char* const* const arrayOfRelativeOrFullPaths, size_t count, LppModulesOption options);
typedef void LppDisableModulesFunction(const wchar_t* const* const arrayOfRelativeOrFullPaths, size_t count, LppModulesOption options);

typedef bool LppWantsReloadFunction(void);
typedef void LppScheduleReloadFunction(void);
typedef void LppCompileAndReloadChangesFunction(LppReloadBehaviour behaviour);

typedef bool LppWantsRestartFunction(void);
typedef void LppRestartFunction(LppRestartBehaviour behaviour, unsigned int exitCode);

typedef void LppSetBoolPreferencesFunction(LppBoolPreferences preferences, bool value);
typedef void LppSetIntPreferencesFunction(LppIntPreferences preferences, int value);
typedef void LppSetStringPreferencesFunction(LppStringPreferences preferences, const char* const value);
typedef void LppSetShortcutPreferencesFunction(LppShortcutPreferences preferences, int virtualKeyCode, int modifiers);

LPP_NAMESPACE_END


// ------------------------------------------------------------------------------------------------
// API AGENTS
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

typedef struct LppDefaultAgent
{
	// Internal platform-specific module. DO NOT USE!
	LppAgentModule __internalModule__;

	// Logs a message to the Live++ UI.
	LppLogMessageFunctionANSI* LogMessageANSI;
	LppLogMessageFunction* LogMessage;

	// Enables automatic handling of dynamically loaded modules (e.g. loaded via LoadLibrary()).
	// Those modules will be automatically enabled during load, and disabled during unload.
	LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunctionANSI* EnableAutomaticHandlingOfDynamicallyLoadedModulesANSI;
	LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunction* EnableAutomaticHandlingOfDynamicallyLoadedModules;

	// Enables a module with the given options.
	LppEnableModuleFunctionANSI* EnableModuleANSI;
	LppEnableModuleFunction* EnableModule;

	// Enables several modules with the given options.
	LppEnableModulesFunctionANSI* EnableModulesANSI;
	LppEnableModulesFunction* EnableModules;

	// Enables a module using a FASTBuild database and target.
	LppEnableFASTBuildModuleFunctionANSI* EnableFASTBuildModuleANSI;
	LppEnableFASTBuildModuleFunction* EnableFASTBuildModule;

	// Disables a module with the given options.
	LppDisableModuleFunctionANSI* DisableModuleANSI;
	LppDisableModuleFunction* DisableModule;

	// Disables several modules with the given options.
	LppDisableModulesFunctionANSI* DisableModulesANSI;
	LppDisableModulesFunction* DisableModules;

	// Schedules a hot-reload operation.
	LppScheduleReloadFunction* ScheduleReload;

	// Sets a boolean preference to the given value.
	LppSetBoolPreferencesFunction* SetBoolPreferences;

	// Sets an integer preference to the given value.
	LppSetIntPreferencesFunction* SetIntPreferences;

	// Sets a string preference to the given value.
	LppSetStringPreferencesFunction* SetStringPreferences;

	// Sets a shortcut preference to the given value.
	// Modifiers can be any combination of MOD_ALT, MOD_CONTROL, MOD_SHIFT and MOD_WIN, e.g. MOD_ALT | MOD_CONTROL.
	LppSetShortcutPreferencesFunction* SetShortcutPreferences;
} LppDefaultAgent;


typedef struct LppSynchronizedAgent
{
	// Internal platform-specific module. DO NOT USE!
	LppAgentModule __internalModule__;

	// Logs a message to the Live++ UI.
	LppLogMessageFunctionANSI* LogMessageANSI;
	LppLogMessageFunction* LogMessage;

	// Enables automatic handling of dynamically loaded modules (e.g. loaded via LoadLibrary()).
	// Those modules will be automatically enabled during load, and disabled during unload.
	LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunctionANSI* EnableAutomaticHandlingOfDynamicallyLoadedModulesANSI;
	LppEnableAutomaticHandlingOfDynamicallyLoadedModulesFunction* EnableAutomaticHandlingOfDynamicallyLoadedModules;

	// Enables a module with the given options.
	LppEnableModuleFunctionANSI* EnableModuleANSI;
	LppEnableModuleFunction* EnableModule;

	// Enables several modules with the given options.
	LppEnableModulesFunctionANSI* EnableModulesANSI;
	LppEnableModulesFunction* EnableModules;

	// Enables a module using a FASTBuild database and target.
	LppEnableFASTBuildModuleFunctionANSI* EnableFASTBuildModuleANSI;
	LppEnableFASTBuildModuleFunction* EnableFASTBuildModule;

	// Disables a module with the given options.
	LppDisableModuleFunctionANSI* DisableModuleANSI;
	LppDisableModuleFunction* DisableModule;

	// Disables several modules with the given options.
	LppDisableModulesFunctionANSI* DisableModulesANSI;
	LppDisableModulesFunction* DisableModules;

	// Returns whether Live++ wants to hot-reload modified files.
	// Returns true once the shortcut has been pressed, or modified files have been detected when continuous compilation is enabled.
	LppWantsReloadFunction* WantsReload;

	// Schedules a hot-reload operation, making WantsReload() return true as soon as possible.
	LppScheduleReloadFunction* ScheduleReload;

	// Instructs Live++ to compile and reload all changes, respecting the given behaviour.
	LppCompileAndReloadChangesFunction* CompileAndReloadChanges;

	// Returns whether Live++ wants to hot-restart the process.
	// Returns true once the process has been selected for hot-restart in the Live++ UI.
	LppWantsRestartFunction* WantsRestart;

	// Restarts the process, respecting the given behaviour.
	// Does not return.
	LppRestartFunction* Restart;

	// Sets a boolean preference to the given value.
	LppSetBoolPreferencesFunction* SetBoolPreferences;

	// Sets an integer preference to the given value.
	LppSetIntPreferencesFunction* SetIntPreferences;

	// Sets a string preference to the given value.
	LppSetStringPreferencesFunction* SetStringPreferences;

	// Sets a shortcut preference to the given value.
	// Modifiers can be any combination of MOD_ALT, MOD_CONTROL, MOD_SHIFT and MOD_WIN, e.g. MOD_ALT | MOD_CONTROL.
	LppSetShortcutPreferencesFunction* SetShortcutPreferences;
} LppSynchronizedAgent;

LPP_NAMESPACE_END


// ------------------------------------------------------------------------------------------------
// INTERNAL APIs
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

// Loads the agent from a shared library.
LPP_API LppAgentModule LppInternalLoadAgentLibraryANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash)
{
	char libraryPath[1024u] = { 0 };
	LppHelperMakeLibraryNameANSI(LPP_PLATFORM_LIBRARY_PREFIX_ANSI, absoluteOrRelativePathWithoutTrailingSlash, LPP_PLATFORM_LIBRARY_NAME_ANSI, libraryPath, 1024u);

	return LppPlatformLoadLibraryANSI(libraryPath);
}

// Loads the agent from a shared library.
LPP_API LppAgentModule LppInternalLoadAgentLibrary(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash)
{
	wchar_t libraryPath[1024u] = { 0 };
	LppHelperMakeLibraryName(LPP_PLATFORM_LIBRARY_PREFIX, absoluteOrRelativePathWithoutTrailingSlash, LPP_PLATFORM_LIBRARY_NAME, libraryPath, 1024u);
	
	return LppPlatformLoadLibrary(libraryPath);
}

// Checks whether the agent version and API version match.
LPP_API void LppInternalCheckVersion(LppAgentModule lppModule)
{
	typedef bool LppCheckVersionFunction(const char* const);
	LppCheckVersionFunction* checkVersion = LPP_REINTERPRET_CAST(LppCheckVersionFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppCheckVersion")));
	if (checkVersion(LPP_VERSION) == false)
	{
		// the version of this API and the agent library don't match
		typedef const char* LppGetVersionFunction(void);
		LppGetVersionFunction* getVersion = LPP_REINTERPRET_CAST(LppGetVersionFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppGetVersion")));

		// store both versions in local variables for inspecting them in a debugger
		const char* const apiVersion = LPP_VERSION;
		const char* const agentLibraryVersion = getVersion();
		(void)apiVersion;
		(void)agentLibraryVersion;

		__debugbreak();
	}
}

// Creates a default agent, either loading the project preferences from a file, or passing them along.
LPP_API LppDefaultAgent LppInternalCreateDefaultAgentANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const char* const absoluteOrRelativePathToProjectPreferences, const LppProjectPreferences* const projectPreferences)
{
	LppDefaultAgent agent = { LPP_INVALID_MODULE };
	LppAgentModule lppModule = LppInternalLoadAgentLibraryANSI(absoluteOrRelativePathWithoutTrailingSlash);
	if (!LppPlatformIsValidLibrary(lppModule))
	{
		return agent;
	}

	LppInternalCheckVersion(lppModule);

	// the module is valid, store it in the agent
	agent.__internalModule__ = lppModule;

	typedef void LppStartupFunction(LppDefaultAgent*, const char* const, const LppProjectPreferences* const);
	LppStartupFunction* startup = LPP_REINTERPRET_CAST(LppStartupFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppStartupDefaultAgentANSI")));
	startup(&agent, absoluteOrRelativePathToProjectPreferences, projectPreferences);

	return agent;
}

// Creates a default agent, either loading the project preferences from a file, or passing them along.
LPP_API LppDefaultAgent LppInternalCreateDefaultAgent(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const wchar_t* const absoluteOrRelativePathToProjectPreferences, const LppProjectPreferences* const projectPreferences)
{
	LppDefaultAgent agent = { LPP_INVALID_MODULE };
	LppAgentModule lppModule = LppInternalLoadAgentLibrary(absoluteOrRelativePathWithoutTrailingSlash);
	if (!LppPlatformIsValidLibrary(lppModule))
	{
		return agent;
	}

	LppInternalCheckVersion(lppModule);

	// the module is valid, store it in the agent
	agent.__internalModule__ = lppModule;

	typedef void LppStartupFunction(LppDefaultAgent*, const wchar_t* const, const LppProjectPreferences* const);
	LppStartupFunction* startup = LPP_REINTERPRET_CAST(LppStartupFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppStartupDefaultAgent")));
	startup(&agent, absoluteOrRelativePathToProjectPreferences, projectPreferences);

	return agent;
}

// Creates a synchronized agent, either loading the project preferences from a file, or passing them along.
LPP_API LppSynchronizedAgent LppInternalCreateSynchronizedAgentANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const char* const absoluteOrRelativePathToProjectPreferences, const LppProjectPreferences* const projectPreferences)
{
	LppSynchronizedAgent agent = { LPP_INVALID_MODULE };
	LppAgentModule lppModule = LppInternalLoadAgentLibraryANSI(absoluteOrRelativePathWithoutTrailingSlash);
	if (!LppPlatformIsValidLibrary(lppModule))
	{
		return agent;
	}

	LppInternalCheckVersion(lppModule);

	// the module is valid, store it in the agent
	agent.__internalModule__ = lppModule;

	typedef void LppStartupFunction(LppSynchronizedAgent*, const char* const, const LppProjectPreferences* const);
	LppStartupFunction* startup = LPP_REINTERPRET_CAST(LppStartupFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppStartupSynchronizedAgentANSI")));
	startup(&agent, absoluteOrRelativePathToProjectPreferences, projectPreferences);

	return agent;
}

// Creates a synchronized agent, either loading the project preferences from a file, or passing them along.
LPP_API LppSynchronizedAgent LppInternalCreateSynchronizedAgent(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const wchar_t* const absoluteOrRelativePathToProjectPreferences, const LppProjectPreferences* const projectPreferences)
{
	LppSynchronizedAgent agent = { LPP_INVALID_MODULE };
	LppAgentModule lppModule = LppInternalLoadAgentLibrary(absoluteOrRelativePathWithoutTrailingSlash);
	if (!LppPlatformIsValidLibrary(lppModule))
	{
		return agent;
	}

	LppInternalCheckVersion(lppModule);

	// the module is valid, store it in the agent
	agent.__internalModule__ = lppModule;

	typedef void LppStartupFunction(LppSynchronizedAgent*, const wchar_t* const, const LppProjectPreferences* const);
	LppStartupFunction* startup = LPP_REINTERPRET_CAST(LppStartupFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(lppModule, "LppStartupSynchronizedAgent")));
	startup(&agent, absoluteOrRelativePathToProjectPreferences, projectPreferences);

	return agent;
}

// Destroys the given agent.
LPP_API void LppInternalDestroyAgent(LppAgentModule agentModule)
{
	typedef void LppShutdownFunction(void);
	LppShutdownFunction* shutdown = LPP_REINTERPRET_CAST(LppShutdownFunction*)(LPP_REINTERPRET_CAST(uintptr_t)(LppPlatformGetFunctionAddress(agentModule, "LppShutdown")));
	shutdown();

	LppPlatformUnloadLibrary(agentModule);
}

LPP_NAMESPACE_END


// ------------------------------------------------------------------------------------------------
// GENERAL APIs
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

// Creates default-initialized project preferences.
LPP_API LppProjectPreferences LppCreateDefaultProjectPreferences(void)
{
	LppProjectPreferences prefs;

	prefs.hotReload.objectFileExtensions = ".obj;.o";
	prefs.hotReload.libraryFileExtensions = ".lib;.a";
	prefs.hotReload.captureToolchainEnvironmentTimeout = 10000;
	prefs.hotReload.preBuild.isEnabled = false;
	prefs.hotReload.preBuild.executable = L"";
	prefs.hotReload.preBuild.workingDirectory = L"./";
	prefs.hotReload.preBuild.commandLineOptions = "";

	prefs.compiler.overrideLocation = L"";
	prefs.compiler.commandLineOptions = "";
	prefs.compiler.captureEnvironment = true;
	prefs.compiler.isOverridden = false;
	prefs.compiler.useOverrideAsFallback = false;
	prefs.compiler.forcePrecompiledHeaderPDBs = false;

	prefs.linker.overrideLocation = L"";
	prefs.linker.commandLineOptions = "";
	prefs.linker.captureEnvironment = true;
	prefs.linker.isOverridden = false;
	prefs.linker.useOverrideAsFallback = false;

	prefs.exceptionHandler.isEnabled = true;
	prefs.exceptionHandler.order = 1;

	prefs.continuousCompilation.directory = L"";
	prefs.continuousCompilation.timeout = 100;
	prefs.continuousCompilation.isEnabled = false;

	prefs.virtualDrive.letterPlusColon = "";
	prefs.virtualDrive.directory = L"";
	prefs.virtualDrive.isEnabled = false;

	prefs.unitySplitting.fileExtensions = ".cpp;.c;.cc;.c++;.cp;.cxx";
	prefs.unitySplitting.threshold = 3;
	prefs.unitySplitting.isEnabled = true;

	prefs.fastBuild.dllPath = L"";
	prefs.fastBuild.removeShowIncludes = false;

	return prefs;
}

// Returns the fully qualified path of the current module, e.g. "C:\MyDirectory\MyApplication.exe".
LPP_API const char* LppGetCurrentModulePathANSI(void)
{
	return LppPlatformGetCurrentModulePathANSI();
}

// Returns the fully qualified path of the current module, e.g. "C:\MyDirectory\MyApplication.exe".
LPP_API const wchar_t* LppGetCurrentModulePath(void)
{
	return LppPlatformGetCurrentModulePath();
}

LPP_NAMESPACE_END


// ------------------------------------------------------------------------------------------------
// DEFAULT AGENT APIs
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

// Returns whether the given default agent is valid.
LPP_API bool LppIsValidDefaultAgent(const LppDefaultAgent* const agent)
{
	return LppPlatformIsValidLibrary(agent->__internalModule__);
}


// Creates a default agent, loading the Live++ agent from the given path, e.g. "ThirdParty\LivePP".
LPP_API LppDefaultAgent LppCreateDefaultAgentANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash)
{
	return LppInternalCreateDefaultAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, LPP_NULL);
}


// Creates a default agent, loading the Live++ agent from the given path, e.g. "ThirdParty\LivePP".
LPP_API LppDefaultAgent LppCreateDefaultAgent(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash)
{
	return LppInternalCreateDefaultAgent(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, LPP_NULL);
}


// Creates a default agent with the given project preferences.
LPP_API LppDefaultAgent LppCreateDefaultAgentWithPreferencesANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const LppProjectPreferences* const projectPreferences)
{
	return LppInternalCreateDefaultAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, projectPreferences);
}


// Creates a default agent with the given project preferences.
LPP_API LppDefaultAgent LppCreateDefaultAgentWithPreferences(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const LppProjectPreferences* const projectPreferences)
{
	return LppInternalCreateDefaultAgent(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, projectPreferences);
}


// Creates a default agent, loading project preferences from the given path.
LPP_API LppDefaultAgent LppCreateDefaultAgentWithPreferencesFromFileANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const char* const absoluteOrRelativePathToProjectPreferences)
{
	return LppInternalCreateDefaultAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, absoluteOrRelativePathToProjectPreferences, LPP_NULL);
}


// Creates a default agent, loading project preferences from the given path.
LPP_API LppDefaultAgent LppCreateDefaultAgentWithPreferencesFromFile(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const wchar_t* const absoluteOrRelativePathToProjectPreferences)
{
	return LppInternalCreateDefaultAgent(absoluteOrRelativePathWithoutTrailingSlash, absoluteOrRelativePathToProjectPreferences, LPP_NULL);
}


// Destroys the given default agent.
LPP_API void LppDestroyDefaultAgent(LppDefaultAgent* agent)
{
	LppInternalDestroyAgent(agent->__internalModule__);
	agent->__internalModule__ = LPP_INVALID_MODULE;
}

LPP_NAMESPACE_END


// ------------------------------------------------------------------------------------------------
// SYNCHRONIZED AGENT APIs
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

// Returns whether the given synchronized agent is valid.
LPP_API bool LppIsValidSynchronizedAgent(const LppSynchronizedAgent* const agent)
{
	return LppPlatformIsValidLibrary(agent->__internalModule__);
}


// Creates a synchronized agent, loading the Live++ agent from the given path, e.g. "ThirdParty\LivePP".
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgentANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash)
{
	return LppInternalCreateSynchronizedAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, LPP_NULL);
}


// Creates a synchronized agent, loading the Live++ agent from the given path, e.g. "ThirdParty\LivePP".
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgent(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash)
{
	return LppInternalCreateSynchronizedAgent(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, LPP_NULL);
}


// Creates a synchronized agent with the given project preferences.
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgentWithPreferencesANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const LppProjectPreferences* const projectPreferences)
{
	return LppInternalCreateSynchronizedAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, projectPreferences);
}


// Creates a synchronized agent with the given project preferences.
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgentWithPreferences(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const LppProjectPreferences* const projectPreferences)
{
	return LppInternalCreateSynchronizedAgent(absoluteOrRelativePathWithoutTrailingSlash, LPP_NULL, projectPreferences);
}


// Creates a synchronized agent, loading project preferences from the given path.
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgentWithPreferencesFromFileANSI(const char* const absoluteOrRelativePathWithoutTrailingSlash, const char* const absoluteOrRelativePathToProjectPreferences)
{
	return LppInternalCreateSynchronizedAgentANSI(absoluteOrRelativePathWithoutTrailingSlash, absoluteOrRelativePathToProjectPreferences, LPP_NULL);
}


// Creates a synchronized agent, loading project preferences from the given path.
LPP_API LppSynchronizedAgent LppCreateSynchronizedAgentWithPreferencesFromFile(const wchar_t* const absoluteOrRelativePathWithoutTrailingSlash, const wchar_t* const absoluteOrRelativePathToProjectPreferences)
{
	return LppInternalCreateSynchronizedAgent(absoluteOrRelativePathWithoutTrailingSlash, absoluteOrRelativePathToProjectPreferences, LPP_NULL);
}


// Destroys the given synchronized agent.
LPP_API void LppDestroySynchronizedAgent(LppSynchronizedAgent* agent)
{
	LppInternalDestroyAgent(agent->__internalModule__);
	agent->__internalModule__ = LPP_INVALID_MODULE;
}

LPP_NAMESPACE_END
