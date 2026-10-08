/**************************************************************************/
/*  register_types.cpp - CBT Content Studio Module                        */
/**************************************************************************/

#include "register_types.h"
#include "cbt_project_launcher.h"
#include "cbt_shell.h"
#include "core/object/class_db.h"

void initialize_cbt_studio_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(CbtProjectLauncher);
		GDREGISTER_CLASS(CbtApplicationShell);
		print_line("[CBT Content Studio] Initializing CBT Content Studio Module...");
	}
}

void uninitialize_cbt_studio_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		print_line("[CBT Content Studio] Cleaning up CBT Content Studio.");
	}
}
