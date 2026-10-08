/**************************************************************************/
/*  cbt_shell.h - CBT Content Studio Application Shell & Navigation       */
/**************************************************************************/

#pragma once

#include "cbt_theme.h"
#include "cbt_project_launcher.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"

class EditorNode;

class CbtApplicationShell : public VBoxContainer {
	GDCLASS(CbtApplicationShell, VBoxContainer);

public:
	enum DomainId {
		DOMAIN_AUTHORING = 0,
		DOMAIN_CONTENT,
		DOMAIN_PRODUCT,
		DOMAIN_SYSTEM,
		DOMAIN_DEVELOPMENT,
		DOMAIN_MAX
	};

	enum ViewId {
		// Authoring
		VIEW_PRODUCTS,
		VIEW_MODULES,
		// Content
		VIEW_SCENES,
		VIEW_PREFABS,
		VIEW_SCENARIOS,
		VIEW_SECTIONS,
		VIEW_STEPS,
		VIEW_EFFECT_PRESETS,
		VIEW_INTERACTABLES,
		VIEW_CATEGORIES,
		VIEW_ASSETS,
		VIEW_MODELS,
		VIEW_MATERIALS,
		// Product
		VIEW_PROJECT_SETTINGS,
		VIEW_BRANDING,
		VIEW_UI_DESIGN,
		VIEW_LOCALIZATION,
		VIEW_PUBLISH,
		// System
		VIEW_INTERACTIONS,
		// Development
		VIEW_DEV_UI_SYSTEM,
		VIEW_DEV_LOCALIZATION,
		VIEW_DEV_SETTINGS,
		VIEW_MAX
	};

private:
	static CbtApplicationShell *singleton;

	EditorNode *editor_node = nullptr;

	DomainId active_domain = DOMAIN_AUTHORING;
	ViewId active_view = VIEW_PRODUCTS;

	// Row 1: App Menu + Domains + Utilities
	HBoxContainer *row1_app_bar = nullptr;
	HBoxContainer *domain_bar = nullptr;
	Vector<Button *> domain_buttons;

	Label *project_badge = nullptr;
	Label *status_badge = nullptr;
	Button *launcher_btn = nullptr;
	Button *settings_btn = nullptr;

	// Row 2: Secondary Destination Bar + Contextual Actions
	HBoxContainer *row2_destination_bar = nullptr;
	HBoxContainer *dest_tabs_container = nullptr;
	Vector<HBoxContainer *> dest_groups;
	HashMap<ViewId, Button *> view_buttons;

	Button *nav_back_btn = nullptr;
	Button *nav_forward_btn = nullptr;
	Button *toggle_list_btn = nullptr;
	Button *toggle_inspector_btn = nullptr;
	Button *save_current_btn = nullptr;
	Button *save_all_btn = nullptr;

	Vector<ViewId> nav_history;
	int nav_history_idx = -1;

	// Reusable 3-Column Workspace Architecture
	HSplitContainer *workspace_main_hsplit = nullptr;
	HSplitContainer *workspace_right_hsplit = nullptr;

	PanelContainer *left_panel_container = nullptr;
	PanelContainer *center_panel_container = nullptr;
	PanelContainer *right_panel_container = nullptr;

	// Column 1: Left Catalog / Collection
	VBoxContainer *domain_left_nav = nullptr;
	Label *domain_left_title = nullptr;
	LineEdit *left_search_box = nullptr;
	ScrollContainer *left_scroll = nullptr;
	VBoxContainer *domain_left_items = nullptr;

	// Column 2: Center Workspace
	VBoxContainer *domain_center_view = nullptr;
	Label *domain_center_title = nullptr;
	Label *domain_center_desc = nullptr;
	HBoxContainer *center_action_toolbar = nullptr;
	ScrollContainer *center_cards_scroll = nullptr;
	VBoxContainer *center_cards_vbox = nullptr;

	// Column 3: Right Inspector
	VBoxContainer *domain_right_inspector = nullptr;
	Label *domain_right_title = nullptr;
	ScrollContainer *inspector_scroll = nullptr;
	VBoxContainer *inspector_fields_vbox = nullptr;

	// Project Launcher Overlay
	CbtProjectLauncher *launcher_overlay = nullptr;

	void _build_row1_domains();
	void _build_row2_destinations();
	void _build_workspace();

	void _update_domain_selection(DomainId p_domain);
	void _update_view_selection(ViewId p_view, bool p_record_history = true);
	void _populate_view_data(ViewId p_view);
	void _select_catalog_item(const String &p_name, const String &p_id, const String &p_category, const String &p_status, const String &p_desc);

	void _on_domain_tab_pressed(int p_domain);
	void _on_destination_tab_pressed(int p_view);

	void _on_nav_back_pressed();
	void _on_nav_forward_pressed();
	void _on_toggle_left_panel();
	void _on_toggle_right_inspector();
	void _on_save_current_pressed();
	void _on_save_all_pressed();
	void _on_launcher_btn_pressed();
	void _on_launcher_close_requested();
	void _on_launcher_project_selected(const String &p_path);

	void _apply_cbt_styling();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	static CbtApplicationShell *get_singleton() { return singleton; }

	void initialize_with_editor(EditorNode *p_editor);
	void set_project_name(const String &p_name);
	void switch_to_domain(int p_domain);
	void switch_to_view(int p_view);
	void show_project_launcher();
	void hide_project_launcher();
	void on_main_screen_activated();

	DomainId get_active_domain() const { return active_domain; }
	ViewId get_active_view() const { return active_view; }

	CbtApplicationShell();
	~CbtApplicationShell();
};
