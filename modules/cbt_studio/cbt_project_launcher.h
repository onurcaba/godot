/**************************************************************************/
/*  cbt_project_launcher.h - CBT Content Studio Project Launcher UI       */
/**************************************************************************/

#pragma once

#include "cbt_theme.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"

class CbtProjectLauncher : public PanelContainer {
	GDCLASS(CbtProjectLauncher, PanelContainer);

public:
	enum ViewMode {
		VIEW_HOME,
		VIEW_REPOSITORY
	};

private:
	ViewMode current_view = VIEW_HOME;

	// UI Elements
	VBoxContainer *main_layout = nullptr;
	VBoxContainer *home_view = nullptr;
	VBoxContainer *repo_view = nullptr;

	VBoxContainer *recent_section = nullptr;
	VBoxContainer *recent_list = nullptr;
	VBoxContainer *empty_state = nullptr;

	Label *error_label = nullptr;
	Label *status_label = nullptr;

	LineEdit *quick_repo_url_input = nullptr;
	Button *quick_add_button = nullptr;
	Button *advanced_button = nullptr;

	Button *new_project_button = nullptr;
	Button *open_project_button = nullptr;
	Button *close_button = nullptr;

	// Advanced Repo View Elements
	LineEdit *repo_url_field = nullptr;
	LineEdit *dest_parent_field = nullptr;
	LineEdit *project_name_field = nullptr;
	Button *repo_confirm_button = nullptr;
	Button *repo_cancel_button = nullptr;

	// Dialogs
	ConfirmationDialog *new_project_dialog = nullptr;
	LineEdit *new_project_name_edit = nullptr;
	LineEdit *new_project_path_edit = nullptr;
	FileDialog *file_dialog = nullptr;
	bool is_picking_new_project_dir = false;

	bool is_standalone_mode = true; // True when running at initial launch

	void _build_header();
	void _build_home_view();
	void _build_repo_view();
	void _build_dialogs();

	void _refresh_recent_projects();
	void _open_project(const String &p_path);
	void _remove_recent_project(const String &p_path);

	void _on_new_project_pressed();
	void _on_new_project_confirmed();
	void _on_open_project_pressed();
	void _on_dir_selected(const String &p_path);
	void _on_quick_add_pressed();
	void _on_advanced_pressed();
	void _on_repo_confirm_pressed();
	void _on_repo_cancel_pressed();
	void _on_close_pressed();

	String _infer_project_name(const String &p_url);
	bool _validate_cbt_project(const String &p_path, String &r_project_name);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_standalone_mode(bool p_standalone) { is_standalone_mode = p_standalone; }
	void refresh();
	void show_home();
	void show_error(const String &p_message);

	CbtProjectLauncher();
	~CbtProjectLauncher();
};
