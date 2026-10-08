/**************************************************************************/
/*  register_types.cpp - CBT Content Studio Module                        */
/**************************************************************************/

#include "register_types.h"
#include "cbt_project_launcher.h"
#include "cbt_shell.h"
#include "cbt_studio_plugin.h"
#include "core/object/class_db.h"

#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#endif

void initialize_cbt_studio_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(CbtProjectLauncher);
		GDREGISTER_CLASS(CbtApplicationShell);
		GDREGISTER_CLASS(CbtTopBarSwitcher);
		GDREGISTER_CLASS(CbtStudioPlugin);

		EditorPlugins::add_by_type<CbtStudioPlugin>();
		print_line("[CBT Content Studio] Initialized CBT Content Studio Module & Auto-Loaded EditorPlugin.");
	}
#endif
}

void uninitialize_cbt_studio_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		print_line("[CBT Content Studio] Cleaned up CBT Content Studio.");
	}
#endif
}
