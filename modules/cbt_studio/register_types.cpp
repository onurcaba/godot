/**************************************************************************/
/*  register_types.cpp - CBT Content Studio Module                        */
/**************************************************************************/

#include "register_types.h"
#include "core/config/engine.h"
#include "core/os/os.h"

void initialize_cbt_studio_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		print_line("[CBT Content Studio] Initializing CBT Content Studio Core Authoring Integration (POC)...");
	}
}

void uninitialize_cbt_studio_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		print_line("[CBT Content Studio] Cleaning up CBT Content Studio Core Authoring Integration.");
	}
}
