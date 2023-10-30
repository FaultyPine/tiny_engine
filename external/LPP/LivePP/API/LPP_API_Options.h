// Copyright 2011-2022 Molecular Matters GmbH, all rights reserved.

#pragma once


// ------------------------------------------------------------------------------------------------
// API OPTIONS
// ------------------------------------------------------------------------------------------------

LPP_NAMESPACE_BEGIN

typedef enum LppModulesOption
{
	LPP_MODULES_OPTION_NONE,
	LPP_MODULES_OPTION_ALL_IMPORT_MODULES
} LppModulesOption;

typedef enum LppReloadBehaviour
{
	LPP_RELOAD_BEHAVIOUR_CONTINUE_EXECUTION,
	LPP_RELOAD_BEHAVIOUR_WAIT_UNTIL_CHANGES_ARE_APPLIED
} LppReloadBehaviour;

typedef enum LppRestartBehaviour
{
	LPP_RESTART_BEHAVIOUR_DEFAULT_EXIT,				// ExitProcess()
	LPP_RESTART_BEHAVIOUR_EXIT_WITH_FLUSH,			// exit()
	LPP_RESTART_BEHAVIOUR_EXIT_WITHOUT_FLUSH,		// _Exit()
	LPP_RESTART_BEHAVIOUR_INSTANT_TERMINATION		// TerminateProcess
} LppRestartBehaviour;

LPP_NAMESPACE_END
