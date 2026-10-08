/**************************************************************************/
/*  cbt_shell.cpp - CBT Content Studio Application Shell & Navigation     */
/**************************************************************************/

#include "cbt_shell.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "editor/docks/inspector_dock.h"
#include "editor/docks/scene_tree_dock.h"
#include "editor/editor_interface.h"
#include "editor/editor_node.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/themes/editor_scale.h"
#include "main/main.h"
#include "scene/gui/separator.h"

CbtApplicationShell *CbtApplicationShell::singleton = nullptr;

void CbtApplicationShell::_bind_methods() {
	ADD_SIGNAL(MethodInfo("view_changed", PropertyInfo(Variant::INT, "view_id")));
	ADD_SIGNAL(MethodInfo("domain_changed", PropertyInfo(Variant::INT, "domain_id")));
}

CbtApplicationShell::CbtApplicationShell() {
	singleton = this;
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_h_size_flags(Control::SIZE_EXPAND_FILL);
	set_v_size_flags(Control::SIZE_EXPAND_FILL);
	add_theme_constant_override("separation", 0);

	_apply_cbt_styling();

	_build_row1_domains();
	_build_row2_destinations();
	_build_workspace();

	nav_history.push_back(VIEW_PRODUCTS);
	nav_history_idx = 0;
}

CbtApplicationShell::~CbtApplicationShell() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void CbtApplicationShell::_notification(int p_what) {
	if (p_what == NOTIFICATION_THEME_CHANGED) {
		_apply_cbt_styling();
	}
}

void CbtApplicationShell::_apply_cbt_styling() {
	Ref<StyleBoxFlat> bar_bg = CbtTheme::create_flat_box(CbtTheme::COLOR_BG_PANEL, CbtTheme::COLOR_BORDER_SUBTLE, 1, 0, 8, 4);
	if (row1_app_bar) {
		row1_app_bar->add_theme_style_override("panel", bar_bg);
	}
	if (row2_destination_bar) {
		row2_destination_bar->add_theme_style_override("panel", bar_bg);
	}
}

void CbtApplicationShell::_build_row1_domains() {
	row1_app_bar = memnew(HBoxContainer);
	row1_app_bar->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	row1_app_bar->set_custom_minimum_size(Vector2(0, Math::round(38 * EDSCALE)));
	row1_app_bar->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	add_child(row1_app_bar);

	// CBT Brand Icon & Title
	Label *brand_lbl = memnew(Label);
	brand_lbl->set_text(TTR("⚡ CBT CONTENT STUDIO"));
	brand_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
	brand_lbl->add_theme_font_size_override("font_size", Math::round(12 * EDSCALE));
	row1_app_bar->add_child(brand_lbl);

	VSeparator *sep1 = memnew(VSeparator);
	row1_app_bar->add_child(sep1);

	// Primary Domains: Authoring | Content | Product | System | Development
	domain_bar = memnew(HBoxContainer);
	domain_bar->add_theme_constant_override("separation", Math::round(4 * EDSCALE));
	row1_app_bar->add_child(domain_bar);

	const char *domain_names[DOMAIN_MAX] = {
		"Authoring",
		"Content",
		"Product",
		"System",
		"Development"
	};

	for (int i = 0; i < DOMAIN_MAX; i++) {
		Button *btn = memnew(Button);
		btn->set_text(TTR(domain_names[i]));
		btn->set_flat(true);
		btn->add_theme_font_size_override("font_size", Math::round(12 * EDSCALE));
		btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_domain_tab_pressed).bind(i));
		domain_bar->add_child(btn);
		domain_buttons.push_back(btn);
	}

	// Spacer to push utilities to the right
	Control *spacer = memnew(Control);
	spacer->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	row1_app_bar->add_child(spacer);

	// Right Utilities: Project Name, Status, Launcher, Settings
	project_badge = memnew(Label);
	project_badge->set_text(TTR("Project: Loading"));
	project_badge->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	project_badge->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	row1_app_bar->add_child(project_badge);

	status_badge = memnew(Label);
	status_badge->set_text(TTR("Ready"));
	status_badge->add_theme_style_override("normal", CbtTheme::create_badge_box(Color(0.06f, 0.72f, 0.5f, 0.2f), CbtTheme::COLOR_SUCCESS));
	status_badge->add_theme_color_override("font_color", CbtTheme::COLOR_SUCCESS);
	status_badge->add_theme_font_size_override("font_size", Math::round(10 * EDSCALE));
	row1_app_bar->add_child(status_badge);

	launcher_btn = memnew(Button);
	launcher_btn->set_text(TTR("Projects ⊞"));
	launcher_btn->set_flat(true);
	launcher_btn->set_tooltip_text(TTR("Open CBT Project Launcher"));
	launcher_btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	launcher_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_launcher_btn_pressed));
	row1_app_bar->add_child(launcher_btn);

	settings_btn = memnew(Button);
	settings_btn->set_text("⚙");
	settings_btn->set_flat(true);
	settings_btn->set_tooltip_text(TTR("Content Studio Settings"));
	settings_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::switch_to_view).bind((int)VIEW_DEV_SETTINGS));
	row1_app_bar->add_child(settings_btn);
}

void CbtApplicationShell::_build_row2_destinations() {
	row2_destination_bar = memnew(HBoxContainer);
	row2_destination_bar->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	row2_destination_bar->set_custom_minimum_size(Vector2(0, Math::round(34 * EDSCALE)));
	row2_destination_bar->add_theme_constant_override("separation", Math::round(6 * EDSCALE));
	add_child(row2_destination_bar);

	dest_tabs_container = memnew(HBoxContainer);
	dest_tabs_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	dest_tabs_container->add_theme_constant_override("separation", Math::round(4 * EDSCALE));
	row2_destination_bar->add_child(dest_tabs_container);

	// Helper to register destination group matching Unity ContentStudioNavigationRegistry
	auto add_group = [&](DomainId p_domain, const Vector<Pair<ViewId, String>> &p_items) {
		HBoxContainer *grp = memnew(HBoxContainer);
		grp->add_theme_constant_override("separation", Math::round(4 * EDSCALE));
		dest_tabs_container->add_child(grp);
		dest_groups.push_back(grp);

		for (int i = 0; i < p_items.size(); i++) {
			ViewId vid = p_items[i].first;
			Button *btn = memnew(Button);
			btn->set_text(TTR(p_items[i].second));
			btn->set_flat(true);
			btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
			btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_destination_tab_pressed).bind((int)vid));
			grp->add_child(btn);
			view_buttons[vid] = btn;
		}
	};

	// 1. Authoring Group
	Vector<Pair<ViewId, String>> auth_items;
	auth_items.push_back(Pair<ViewId, String>(VIEW_PRODUCTS, "Products"));
	auth_items.push_back(Pair<ViewId, String>(VIEW_MODULES, "Modules"));
	add_group(DOMAIN_AUTHORING, auth_items);

	// 2. Content Group
	Vector<Pair<ViewId, String>> content_items;
	content_items.push_back(Pair<ViewId, String>(VIEW_SCENES, "Scenes 3D"));
	content_items.push_back(Pair<ViewId, String>(VIEW_PREFABS, "Prefabs"));
	content_items.push_back(Pair<ViewId, String>(VIEW_SCENARIOS, "Scenarios"));
	content_items.push_back(Pair<ViewId, String>(VIEW_SECTIONS, "Sections"));
	content_items.push_back(Pair<ViewId, String>(VIEW_STEPS, "Steps"));
	content_items.push_back(Pair<ViewId, String>(VIEW_EFFECT_PRESETS, "Effect Presets"));
	content_items.push_back(Pair<ViewId, String>(VIEW_INTERACTABLES, "Interactables"));
	content_items.push_back(Pair<ViewId, String>(VIEW_CATEGORIES, "Categories"));
	content_items.push_back(Pair<ViewId, String>(VIEW_ASSETS, "Assets"));
	content_items.push_back(Pair<ViewId, String>(VIEW_MODELS, "Models"));
	content_items.push_back(Pair<ViewId, String>(VIEW_MATERIALS, "Materials"));
	add_group(DOMAIN_CONTENT, content_items);

	// 3. Product Group
	Vector<Pair<ViewId, String>> prod_items;
	prod_items.push_back(Pair<ViewId, String>(VIEW_PROJECT_SETTINGS, "Project Settings"));
	prod_items.push_back(Pair<ViewId, String>(VIEW_BRANDING, "Branding"));
	prod_items.push_back(Pair<ViewId, String>(VIEW_UI_DESIGN, "UI Design"));
	prod_items.push_back(Pair<ViewId, String>(VIEW_LOCALIZATION, "Localization"));
	prod_items.push_back(Pair<ViewId, String>(VIEW_PUBLISH, "Publish"));
	add_group(DOMAIN_PRODUCT, prod_items);

	// 4. System Group
	Vector<Pair<ViewId, String>> sys_items;
	sys_items.push_back(Pair<ViewId, String>(VIEW_INTERACTIONS, "Interaction Types"));
	add_group(DOMAIN_SYSTEM, sys_items);

	// 5. Development Group
	Vector<Pair<ViewId, String>> dev_items;
	dev_items.push_back(Pair<ViewId, String>(VIEW_DEV_UI_SYSTEM, "UI System"));
	dev_items.push_back(Pair<ViewId, String>(VIEW_DEV_LOCALIZATION, "Localization"));
	dev_items.push_back(Pair<ViewId, String>(VIEW_DEV_SETTINGS, "Settings"));
	add_group(DOMAIN_DEVELOPMENT, dev_items);

	// Contextual Actions on Right: Nav History + Panel Toggles + Saves
	HBoxContainer *actions_box = memnew(HBoxContainer);
	actions_box->add_theme_constant_override("separation", Math::round(6 * EDSCALE));
	row2_destination_bar->add_child(actions_box);

	nav_back_btn = memnew(Button);
	nav_back_btn->set_text("←");
	nav_back_btn->set_flat(true);
	nav_back_btn->set_tooltip_text(TTR("Back"));
	nav_back_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_nav_back_pressed));
	actions_box->add_child(nav_back_btn);

	nav_forward_btn = memnew(Button);
	nav_forward_btn->set_text("→");
	nav_forward_btn->set_flat(true);
	nav_forward_btn->set_tooltip_text(TTR("Forward"));
	nav_forward_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_nav_forward_pressed));
	actions_box->add_child(nav_forward_btn);

	toggle_list_btn = memnew(Button);
	toggle_list_btn->set_text(TTR("Hide List"));
	toggle_list_btn->set_flat(true);
	toggle_list_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_toggle_left_panel));
	actions_box->add_child(toggle_list_btn);

	toggle_inspector_btn = memnew(Button);
	toggle_inspector_btn->set_text(TTR("Hide Inspector"));
	toggle_inspector_btn->set_flat(true);
	toggle_inspector_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_toggle_right_inspector));
	actions_box->add_child(toggle_inspector_btn);

	save_current_btn = memnew(Button);
	save_current_btn->set_text(TTR("Save Current"));
	save_current_btn->add_theme_style_override("normal", CbtTheme::create_button_box(false, false, false));
	save_current_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_save_current_pressed));
	actions_box->add_child(save_current_btn);

	save_all_btn = memnew(Button);
	save_all_btn->set_text(TTR("Save All"));
	save_all_btn->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	save_all_btn->add_theme_color_override("font_color", Color(1, 1, 1));
	save_all_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_save_all_pressed));
	actions_box->add_child(save_all_btn);
}

void CbtApplicationShell::_build_workspace() {
	workspace_main_hsplit = memnew(HSplitContainer);
	workspace_main_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_main_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(workspace_main_hsplit);

	// Column 1: Left Catalog / Collection Panel
	left_panel_container = memnew(PanelContainer);
	left_panel_container->set_custom_minimum_size(Vector2(Math::round(250 * EDSCALE), 0));
	left_panel_container->add_theme_style_override("panel", CbtTheme::create_panel_box());
	workspace_main_hsplit->add_child(left_panel_container);

	domain_left_nav = memnew(VBoxContainer);
	domain_left_nav->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_left_nav->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_left_nav->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	left_panel_container->add_child(domain_left_nav);

	domain_left_title = memnew(Label);
	domain_left_title->set_text(TTR("Collection Navigator"));
	domain_left_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	domain_left_title->add_theme_font_size_override("font_size", Math::round(13 * EDSCALE));
	domain_left_nav->add_child(domain_left_title);

	left_search_box = memnew(LineEdit);
	left_search_box->set_placeholder(TTR("Filter collection…"));
	domain_left_nav->add_child(left_search_box);

	left_scroll = memnew(ScrollContainer);
	left_scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_left_nav->add_child(left_scroll);

	domain_left_items = memnew(VBoxContainer);
	domain_left_items->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_left_items->add_theme_constant_override("separation", Math::round(4 * EDSCALE));
	left_scroll->add_child(domain_left_items);

	// Right Split: Column 2 (Center Workspace) vs Column 3 (Right Inspector)
	workspace_right_hsplit = memnew(HSplitContainer);
	workspace_right_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_right_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_main_hsplit->add_child(workspace_right_hsplit);

	// Column 2: Center Workspace Container
	center_panel_container = memnew(PanelContainer);
	center_panel_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel_container->set_custom_minimum_size(Vector2(Math::round(480 * EDSCALE), 0));
	center_panel_container->add_theme_style_override("panel", CbtTheme::create_flat_box(CbtTheme::COLOR_BG_DARKEST, Color(0, 0, 0, 0), 0, 0, 0, 0));
	workspace_right_hsplit->add_child(center_panel_container);

	domain_center_view = memnew(VBoxContainer);
	domain_center_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->add_theme_constant_override("separation", Math::round(10 * EDSCALE));
	center_panel_container->add_child(domain_center_view);

	// Workspace Header
	HBoxContainer *center_hdr = memnew(HBoxContainer);
	center_hdr->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->add_child(center_hdr);

	VBoxContainer *center_titles = memnew(VBoxContainer);
	center_titles->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_titles->add_theme_constant_override("separation", Math::round(2 * EDSCALE));
	center_hdr->add_child(center_titles);

	domain_center_title = memnew(Label);
	domain_center_title->set_text(TTR("CBT Domain Workspace"));
	domain_center_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	domain_center_title->add_theme_font_size_override("font_size", Math::round(18 * EDSCALE));
	center_titles->add_child(domain_center_title);

	domain_center_desc = memnew(Label);
	domain_center_desc->set_text(TTR("Authoring workspace for canonical CBT domain definitions"));
	domain_center_desc->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	domain_center_desc->add_theme_font_size_override("font_size", Math::round(12 * EDSCALE));
	center_titles->add_child(domain_center_desc);

	// Action toolbar in Center
	center_action_toolbar = memnew(HBoxContainer);
	center_action_toolbar->add_theme_constant_override("separation", Math::round(6 * EDSCALE));
	center_hdr->add_child(center_action_toolbar);

	Button *new_item_btn = memnew(Button);
	new_item_btn->set_text(TTR("+ New"));
	new_item_btn->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	new_item_btn->add_theme_color_override("font_color", Color(1, 1, 1));
	center_action_toolbar->add_child(new_item_btn);

	Button *dup_item_btn = memnew(Button);
	dup_item_btn->set_text(TTR("Duplicate"));
	dup_item_btn->set_flat(true);
	center_action_toolbar->add_child(dup_item_btn);

	Button *refresh_btn = memnew(Button);
	refresh_btn->set_text("↻");
	refresh_btn->set_flat(true);
	refresh_btn->set_tooltip_text(TTR("Refresh View"));
	center_action_toolbar->add_child(refresh_btn);

	domain_center_view->add_child(memnew(HSeparator));

	// Center Cards Scroll
	center_cards_scroll = memnew(ScrollContainer);
	center_cards_scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_cards_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->add_child(center_cards_scroll);

	center_cards_vbox = memnew(VBoxContainer);
	center_cards_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_cards_vbox->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	center_cards_scroll->add_child(center_cards_vbox);

	// Column 3: Right Inspector Container
	right_panel_container = memnew(PanelContainer);
	right_panel_container->set_custom_minimum_size(Vector2(Math::round(300 * EDSCALE), 0));
	right_panel_container->add_theme_style_override("panel", CbtTheme::create_panel_box());
	workspace_right_hsplit->add_child(right_panel_container);

	domain_right_inspector = memnew(VBoxContainer);
	domain_right_inspector->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_right_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_right_inspector->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	right_panel_container->add_child(domain_right_inspector);

	domain_right_title = memnew(Label);
	domain_right_title->set_text(TTR("Properties & Metadata"));
	domain_right_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	domain_right_title->add_theme_font_size_override("font_size", Math::round(13 * EDSCALE));
	domain_right_inspector->add_child(domain_right_title);

	domain_right_inspector->add_child(memnew(HSeparator));

	inspector_scroll = memnew(ScrollContainer);
	inspector_scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	inspector_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_right_inspector->add_child(inspector_scroll);

	inspector_fields_vbox = memnew(VBoxContainer);
	inspector_fields_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	inspector_fields_vbox->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	inspector_scroll->add_child(inspector_fields_vbox);

	// Default split offsets (Left: 260px, Center: flex, Right: 300px)
	workspace_main_hsplit->set_split_offset(Math::round(260 * EDSCALE));
	workspace_right_hsplit->set_split_offset(Math::round(640 * EDSCALE));
}

void CbtApplicationShell::initialize_with_editor(EditorNode *p_editor) {
	editor_node = p_editor;

	// Read project name from project.json if exists
	String pj = "res://project.json";
	if (FileAccess::exists(pj)) {
		Error err;
		String content = FileAccess::get_file_as_string(pj, &err);
		if (err == OK) {
			Ref<JSON> json;
			json.instantiate();
			if (json->parse(content) == OK) {
				Dictionary d = json->get_data();
				if (d.has("name")) {
					set_project_name(d["name"]);
				}
			}
		}
	} else {
		String appname = GLOBAL_GET("application/config/name");
		set_project_name(appname.is_empty() ? "CBT Training Workspace" : appname);
	}

	// Create launcher overlay inside editor
	launcher_overlay = memnew(CbtProjectLauncher);
	launcher_overlay->set_standalone_mode(false);
	launcher_overlay->set_visible(false);
	launcher_overlay->connect("close_requested", callable_mp(this, &CbtApplicationShell::_on_launcher_close_requested));
	launcher_overlay->connect("project_selected", callable_mp(this, &CbtApplicationShell::_on_launcher_project_selected));
	if (editor_node && editor_node->get_gui_base()) {
		editor_node->get_gui_base()->add_child(launcher_overlay);
	} else {
		add_child(launcher_overlay);
	}

	// Start in Authoring -> Products
	switch_to_domain(DOMAIN_AUTHORING);
	switch_to_view(VIEW_PRODUCTS);
}

void CbtApplicationShell::set_project_name(const String &p_name) {
	if (project_badge) {
		project_badge->set_text(vformat(TTR("Project: %s"), p_name));
	}
}

void CbtApplicationShell::_update_domain_selection(DomainId p_domain) {
	active_domain = p_domain;

	for (int i = 0; i < domain_buttons.size(); i++) {
		bool sel = (i == (int)active_domain);
		domain_buttons[i]->add_theme_style_override("normal", CbtTheme::create_domain_tab_box(sel, false));
		domain_buttons[i]->add_theme_style_override("hover", CbtTheme::create_domain_tab_box(sel, true));
		domain_buttons[i]->add_theme_color_override("font_color", sel ? Color(1, 1, 1) : CbtTheme::COLOR_TEXT_SECONDARY);
	}

	for (int i = 0; i < dest_groups.size(); i++) {
		dest_groups[i]->set_visible(i == (int)active_domain);
	}

	emit_signal("domain_changed", (int)active_domain);
}

void CbtApplicationShell::_update_view_selection(ViewId p_view, bool p_record_history) {
	active_view = p_view;

	if (p_record_history) {
		if (nav_history_idx < nav_history.size() - 1) {
			nav_history.resize(nav_history_idx + 1);
		}
		nav_history.push_back(p_view);
		nav_history_idx = nav_history.size() - 1;
	}

	nav_back_btn->set_disabled(nav_history_idx <= 0);
	nav_forward_btn->set_disabled(nav_history_idx >= nav_history.size() - 1);

	for (KeyValue<ViewId, Button *> &E : view_buttons) {
		bool sel = (E.key == active_view);
		E.value->add_theme_style_override("normal", CbtTheme::create_tab_box(sel, false));
		E.value->add_theme_style_override("hover", CbtTheme::create_tab_box(sel, true));
		E.value->add_theme_color_override("font_color", sel ? Color(1, 1, 1) : CbtTheme::COLOR_TEXT_SECONDARY);
	}

	// Contextual workspace switching
	bool is_scene_view = (active_view == VIEW_SCENES);

	if (is_scene_view) {
		workspace_main_hsplit->set_visible(false);
		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_distraction_free_mode(false);
			EditorInterface::get_singleton()->set_main_screen_editor("3D");
		}
		domain_center_view->set_visible(false);
		domain_left_nav->set_visible(false);
		domain_right_inspector->set_visible(false);
	} else {
		workspace_main_hsplit->set_visible(true);
		workspace_main_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		domain_center_view->set_visible(true);
		domain_left_nav->set_visible(true);
		domain_right_inspector->set_visible(true);

		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_distraction_free_mode(true);
		}

		_populate_view_data(active_view);
	}

	emit_signal("view_changed", (int)active_view);
}

void CbtApplicationShell::on_main_screen_activated() {
	if (active_view == VIEW_SCENES) {
		switch_to_view(VIEW_SCENARIOS);
	} else {
		_populate_view_data(active_view);
	}
}

void CbtApplicationShell::_populate_view_data(ViewId p_view) {
	Button *active_btn = view_buttons.has(p_view) ? view_buttons[p_view] : nullptr;
	String view_name = active_btn ? active_btn->get_text() : "CBT View";

	domain_center_title->set_text(view_name);
	domain_left_title->set_text(vformat(TTR("%s Collection"), view_name));

	// Clear Left Catalog
	while (domain_left_items->get_child_count() > 0) {
		Node *c = domain_left_items->get_child(0);
		domain_left_items->remove_child(c);
		memdelete(c);
	}

	// Clear Center Cards
	while (center_cards_vbox->get_child_count() > 0) {
		Node *c = center_cards_vbox->get_child(0);
		center_cards_vbox->remove_child(c);
		memdelete(c);
	}

	struct DomainSampleItem {
		String name;
		String id;
		String category;
		String status;
		String desc;
	};

	Vector<DomainSampleItem> sample_items;

	switch (p_view) {
		case VIEW_PRODUCTS: {
			domain_center_desc->set_text(TTR("Product catalog, training simulation packaging, and deployment profiles"));
			sample_items.push_back({ "Industrial Mobile Crane Simulator", "prod.crane.v2", "Heavy Equipment", "Published", "Full 6-DOF crane operations with cabin physics and load sway dynamics." });
			sample_items.push_back({ "High-Voltage Switchgear Lab", "prod.switchgear.v1", "Electrical Safety", "Review", "Procedural substation lockout/tagout training with arc flash hazard checks." });
			sample_items.push_back({ "Aircraft De-Icing & Ramp Operations", "prod.deicing.v3", "Aviation Ground", "Draft", "Apron coordination, spray fluid management, and winter taxi clearance." });
			sample_items.push_back({ "Warehouse Forklift Certification", "prod.forklift.v1", "Logistics", "Published", "OSHA compliant pallet handling, ramp ascent, and narrow aisle staging." });
		} break;

		case VIEW_MODULES: {
			domain_center_desc->set_text(TTR("Instructional course modules, training curricula, and assessment tracks"));
			sample_items.push_back({ "Module A: Walkaround & Pre-Op Checks", "mod.preop.01", "Core Curriculum", "Active", "Daily inspection points, fluid reservoirs, and chassis integrity." });
			sample_items.push_back({ "Module B: Hazard Identification & Safety", "mod.hazard.02", "Safety", "Active", "Proximity sensors, power line clearance, and exclusion zones." });
			sample_items.push_back({ "Module C: Precision Load Maneuvering", "mod.ops.03", "Operations", "Active", "Rigging alignment, crane arm extension, and wind compensation." });
			sample_items.push_back({ "Module D: Emergency Shutdown Sequences", "mod.emergency.04", "Emergency", "Active", "Loss of hydraulic pressure, engine fire containment, and egress." });
		} break;

		case VIEW_SCENARIOS: {
			domain_center_desc->set_text(TTR("Interactive scenario flowgraphs, procedural steps, and evaluation logic"));
			sample_items.push_back({ "Scenario 01: Pre-Operational Walkaround", "scen.walkaround.01", "Inspection", "Verified", "12 checkpoints covering tires, hydraulics, safety pins, and outriggers." });
			sample_items.push_back({ "Scenario 02: Standard Cabin Startup Sequence", "scen.startup.02", "Startup", "Verified", "Master power engage, computer diagnostic pass, and engine ignition." });
			sample_items.push_back({ "Scenario 03: Primary Hydraulic Failure Response", "scen.failure.03", "Emergency", "Verified", "Immediate load freeze, emergency bleed-off, and cabin evacuation." });
			sample_items.push_back({ "Scenario 04: Night Operations with Spotter", "scen.night.04", "Advanced", "Draft", "Radio communication protocols, floodlight setup, and hand signals." });
		} break;

		case VIEW_SECTIONS: {
			domain_center_desc->set_text(TTR("Scenario chapters, narrative milestones, and stage transitions"));
			sample_items.push_back({ "Section 1: Ground Safety Perimeter", "sec.perimeter.01", "Ground Phase", "Active", "Cones placement and perimeter tape installation." });
			sample_items.push_back({ "Section 2: Cabin Control Familiarization", "sec.cabin.02", "Cabin Phase", "Active", "Joystick axes, throttle lock, and gauge validation." });
			sample_items.push_back({ "Section 3: Outrigger Extension & Leveling", "sec.outriggers.03", "Setup Phase", "Active", "Hydraulic pad deployment and digital spirit level verification." });
		} break;

		case VIEW_STEPS: {
			domain_center_desc->set_text(TTR("Atomic instructional steps, substep interactions, and criteria"));
			sample_items.push_back({ "Step 01: Fasten 4-Point Safety Harness", "step.harness.01", "Safety", "Complete", "Requires harness snap audio cue and buckle engagement check." });
			sample_items.push_back({ "Step 02: Switch Master Battery Isolator ON", "step.isolator.02", "Electrical", "Complete", "Rotary 90-degree key switch on battery compartment." });
			sample_items.push_back({ "Step 03: Verify Warning Cluster Self-Test", "step.selftest.03", "Diagnostic", "Complete", "Wait for all 8 LED indicators to complete test cycle." });
			sample_items.push_back({ "Step 04: Engage Hydraulic Auxiliary Pump", "step.auxpump.04", "Hydraulic", "In Progress", "Press illuminated green rocker switch on upper dash." });
		} break;

		case VIEW_EFFECT_PRESETS: {
			domain_center_desc->set_text(TTR("Visual FX presets, particle emitters, audio effects, and animation triggers"));
			sample_items.push_back({ "FX_Smoke_EngineOverheat", "fx.smoke.overheat", "Thermal", "Active", "Dense grey smoke with heat distortion sprite layer." });
			sample_items.push_back({ "FX_Spark_ElectricalShort", "fx.spark.short", "Electrical", "Active", "High-frequency spark burst with snap sound event." });
			sample_items.push_back({ "FX_Steam_PressureRelief", "fx.steam.relief", "Pneumatic", "Active", "Directional high-velocity white steam cone." });
		} break;

		case VIEW_INTERACTABLES: {
			domain_center_desc->set_text(TTR("Interactable physical objects, levers, buttons, valves, and XR handles"));
			sample_items.push_back({ "Interactable_RotaryDial_Throttle", "obj.throttle.dial", "Cockpit", "Configured", "Continuous rotary dial with 10 discrete detent steps." });
			sample_items.push_back({ "Interactable_PushButton_EmergencyStop", "obj.estop.button", "Emergency", "Configured", "Mushroom latching push button with twist-to-release mechanism." });
			sample_items.push_back({ "Interactable_Joystick_BoomControl", "obj.joystick.boom", "Controls", "Configured", "Dual-axis spring-centered joystick with top thumb button." });
		} break;

		case VIEW_CATEGORIES: {
			domain_center_desc->set_text(TTR("Global content taxonomy categories and metadata classification"));
			sample_items.push_back({ "Cabin Controls & Instruments", "cat.cabin", "Taxonomy", "Standard", "All interior switchgear, dials, pedals, and digital displays." });
			sample_items.push_back({ "Structural & Chassis Hardware", "cat.structural", "Taxonomy", "Standard", "Outriggers, counterweights, boom segments, and pulleys." });
			sample_items.push_back({ "Personal Protective Equipment", "cat.ppe", "Taxonomy", "Standard", "Hard hats, high-vis vests, steel-toe boots, and safety glasses." });
		} break;

		case VIEW_ASSETS: {
			domain_center_desc->set_text(TTR("Unified asset repository: 3D models, textures, audio, and documents"));
			sample_items.push_back({ "Asset_CraneCabin_HighPoly.glb", "asset.cabin.glb", "3D Model", "Imported", "125k triangles, 4K PBR material set, rigged joysticks." });
			sample_items.push_back({ "Asset_EngineBay_AudioLoop.wav", "asset.engine.wav", "Audio", "Imported", "48kHz 24-bit ambient diesel idle with variable RPM pitch." });
			sample_items.push_back({ "Asset_CraneOperatorsManual_EN.pdf", "asset.manual.pdf", "Document", "Linked", "Manufacturer standard operation and maintenance handbook." });
		} break;

		case VIEW_PROJECT_SETTINGS: {
			domain_center_desc->set_text(TTR("Project identity, package configuration, and deployment targets"));
			sample_items.push_back({ "Simulation Package Identity", "cfg.identity", "Core", "Configured", "com.cbt.training.heavycrane | Version 2.4.0" });
			sample_items.push_back({ "Target Hardware Profiles", "cfg.targets", "Hardware", "Configured", "Meta Quest 3, Vive Focus 3, PC Desktop Simulation" });
			sample_items.push_back({ "Physics & Collision Subsystem", "cfg.physics", "Engine", "Configured", "Jolt Physics 60Hz tick rate, continuous collision detection" });
		} break;

		case VIEW_UI_DESIGN: {
			domain_center_desc->set_text(TTR("Runtime UI surface configurations, HUD styles, and tablet interfaces"));
			sample_items.push_back({ "Surface: VR Headset Spatial HUD", "ui.spatial.hud", "XR UI", "Active", "Floating world-space curved panel pinned to operator periphery." });
			sample_items.push_back({ "Surface: In-Cabin Interactive Tablet", "ui.tablet.cabin", "Diegetic", "Active", "Physical tablet held in hand or docked in cabin charging mount." });
			sample_items.push_back({ "Surface: Instructor Session Overlay", "ui.instructor.overlay", "Desktop", "Active", "Real-time trainee telemetry, fault injection, and reset controls." });
		} break;

		default: {
			domain_center_desc->set_text(vformat(TTR("Authoring workspace for '%s'"), view_name));
			for (int i = 1; i <= 4; i++) {
				sample_items.push_back({
					vformat("%s Item #%02d", view_name, i),
					vformat("cbt.%s.%02d", view_name.to_lower().replace(" ", "_"), i),
					"Standard",
					"Active",
					vformat("Authoring definition for %s #%02d in CBT Content Studio.", view_name, i)
				});
			}
		} break;
	}

	// Populate Left Catalog Items
	for (int i = 0; i < sample_items.size(); i++) {
		const DomainSampleItem &it = sample_items[i];
		Button *btn = memnew(Button);
		btn->set_text(vformat("  %s", it.name));
		btn->set_flat(true);
		btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
		btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
		btn->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
		btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_select_catalog_item).bind(it.name, it.id, it.category, it.status, it.desc));
		domain_left_items->add_child(btn);
	}

	// Populate Center Cards
	for (int i = 0; i < sample_items.size(); i++) {
		const DomainSampleItem &it = sample_items[i];
		PanelContainer *card = memnew(PanelContainer);
		card->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		card->add_theme_style_override("panel", CbtTheme::create_card_box(false, false));
		center_cards_vbox->add_child(card);

		MarginContainer *cm = memnew(MarginContainer);
		float s = EDSCALE;
		cm->add_theme_constant_override("margin_left", Math::round(12 * s));
		cm->add_theme_constant_override("margin_right", Math::round(12 * s));
		cm->add_theme_constant_override("margin_top", Math::round(10 * s));
		cm->add_theme_constant_override("margin_bottom", Math::round(10 * s));
		card->add_child(cm);

		HBoxContainer *chb = memnew(HBoxContainer);
		chb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		chb->add_theme_constant_override("separation", Math::round(12 * s));
		cm->add_child(chb);

		VBoxContainer *cinfo = memnew(VBoxContainer);
		cinfo->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		cinfo->add_theme_constant_override("separation", Math::round(4 * s));
		chb->add_child(cinfo);

		HBoxContainer *ctitle_row = memnew(HBoxContainer);
		ctitle_row->add_theme_constant_override("separation", Math::round(8 * s));
		cinfo->add_child(ctitle_row);

		Label *clbl = memnew(Label);
		clbl->set_text(it.name);
		clbl->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
		clbl->add_theme_font_size_override("font_size", Math::round(13 * s));
		ctitle_row->add_child(clbl);

		Label *cstatus = memnew(Label);
		cstatus->set_text(it.status);
		cstatus->add_theme_style_override("normal", CbtTheme::create_badge_box(Color(0.23f, 0.51f, 0.96f, 0.2f), CbtTheme::COLOR_PRIMARY));
		cstatus->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
		cstatus->add_theme_font_size_override("font_size", Math::round(9 * s));
		ctitle_row->add_child(cstatus);

		Label *cdesc = memnew(Label);
		cdesc->set_text(it.desc);
		cdesc->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
		cdesc->add_theme_font_size_override("font_size", Math::round(11 * s));
		cdesc->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
		cinfo->add_child(cdesc);

		// Action button on card
		Button *card_action_btn = memnew(Button);
		card_action_btn->set_text(TTR("Open"));
		card_action_btn->set_flat(true);
		card_action_btn->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
		card_action_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_select_catalog_item).bind(it.name, it.id, it.category, it.status, it.desc));
		chb->add_child(card_action_btn);
	}

	// Select first item by default
	if (sample_items.size() > 0) {
		const DomainSampleItem &first = sample_items[0];
		_select_catalog_item(first.name, first.id, first.category, first.status, first.desc);
	}
}

void CbtApplicationShell::_select_catalog_item(const String &p_name, const String &p_id, const String &p_category, const String &p_status, const String &p_desc) {
	// Clear existing inspector fields
	while (inspector_fields_vbox->get_child_count() > 0) {
		Node *c = inspector_fields_vbox->get_child(0);
		inspector_fields_vbox->remove_child(c);
		memdelete(c);
	}

	float s = EDSCALE;

	Label *name_lbl = memnew(Label);
	name_lbl->set_text(p_name);
	name_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	name_lbl->add_theme_font_size_override("font_size", Math::round(14 * s));
	name_lbl->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	inspector_fields_vbox->add_child(name_lbl);

	Label *status_lbl = memnew(Label);
	status_lbl->set_text(vformat(TTR("Status: %s"), p_status));
	status_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_SUCCESS);
	status_lbl->add_theme_font_size_override("font_size", Math::round(11 * s));
	inspector_fields_vbox->add_child(status_lbl);

	inspector_fields_vbox->add_child(memnew(HSeparator));

	auto add_property_row = [&](const String &p_label, const String &p_val) {
		VBoxContainer *row = memnew(VBoxContainer);
		row->add_theme_constant_override("separation", Math::round(2 * s));
		inspector_fields_vbox->add_child(row);

		Label *lbl = memnew(Label);
		lbl->set_text(p_label);
		lbl->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
		lbl->add_theme_font_size_override("font_size", Math::round(10 * s));
		row->add_child(lbl);

		Label *val = memnew(Label);
		val->set_text(p_val);
		val->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
		val->add_theme_font_size_override("font_size", Math::round(11 * s));
		row->add_child(val);
	};

	add_property_row("Identifier", p_id);
	add_property_row("Category", p_category);
	add_property_row("Description", p_desc);
	add_property_row("Schema Version", "CBT DataContract v4.2");
	add_property_row("Target Runtime", "VR Headset / PC Desktop");
	add_property_row("Last Modified", "Today, 12:45 PM");

	inspector_fields_vbox->add_child(memnew(HSeparator));

	Button *edit_btn = memnew(Button);
	edit_btn->set_text(TTR("Edit Definition"));
	edit_btn->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	edit_btn->add_theme_color_override("font_color", Color(1, 1, 1));
	inspector_fields_vbox->add_child(edit_btn);

	Button *scene_btn = memnew(Button);
	scene_btn->set_text(TTR("Inspect in 3D Scene →"));
	scene_btn->add_theme_style_override("normal", CbtTheme::create_button_box(false, false, false));
	scene_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::switch_to_view).bind((int)VIEW_SCENES));
	inspector_fields_vbox->add_child(scene_btn);
}

void CbtApplicationShell::switch_to_domain(int p_domain) {
	DomainId dom = (DomainId)p_domain;
	_update_domain_selection(dom);
	switch (dom) {
		case DOMAIN_AUTHORING:
			_update_view_selection(VIEW_PRODUCTS);
			break;
		case DOMAIN_CONTENT:
			_update_view_selection(VIEW_SCENARIOS);
			break;
		case DOMAIN_PRODUCT:
			_update_view_selection(VIEW_PROJECT_SETTINGS);
			break;
		case DOMAIN_SYSTEM:
			_update_view_selection(VIEW_INTERACTIONS);
			break;
		case DOMAIN_DEVELOPMENT:
			_update_view_selection(VIEW_DEV_UI_SYSTEM);
			break;
		default:
			break;
	}
}

void CbtApplicationShell::switch_to_view(int p_view) {
	ViewId vid = (ViewId)p_view;
	if (vid <= VIEW_MODULES) {
		_update_domain_selection(DOMAIN_AUTHORING);
	} else if (vid <= VIEW_MATERIALS) {
		_update_domain_selection(DOMAIN_CONTENT);
	} else if (vid <= VIEW_PUBLISH) {
		_update_domain_selection(DOMAIN_PRODUCT);
	} else if (vid <= VIEW_INTERACTIONS) {
		_update_domain_selection(DOMAIN_SYSTEM);
	} else {
		_update_domain_selection(DOMAIN_DEVELOPMENT);
	}
	_update_view_selection(vid);
}

void CbtApplicationShell::_on_domain_tab_pressed(int p_domain) {
	switch_to_domain((DomainId)p_domain);
}

void CbtApplicationShell::_on_destination_tab_pressed(int p_view) {
	_update_view_selection((ViewId)p_view);
}

void CbtApplicationShell::_on_nav_back_pressed() {
	if (nav_history_idx > 0) {
		nav_history_idx--;
		_update_view_selection(nav_history[nav_history_idx], false);
	}
}

void CbtApplicationShell::_on_nav_forward_pressed() {
	if (nav_history_idx < nav_history.size() - 1) {
		nav_history_idx++;
		_update_view_selection(nav_history[nav_history_idx], false);
	}
}

void CbtApplicationShell::_on_toggle_left_panel() {
	if (active_view == VIEW_SCENES) {
		if (SceneTreeDock::get_singleton()) {
			bool is_vis = SceneTreeDock::get_singleton()->is_visible();
			SceneTreeDock::get_singleton()->set_visible(!is_vis);
			toggle_list_btn->set_text(is_vis ? TTR("Show Hierarchy") : TTR("Hide Hierarchy"));
		}
	} else {
		bool is_vis = left_panel_container->is_visible();
		left_panel_container->set_visible(!is_vis);
		toggle_list_btn->set_text(is_vis ? TTR("Show List") : TTR("Hide List"));
	}
}

void CbtApplicationShell::_on_toggle_right_inspector() {
	if (active_view == VIEW_SCENES) {
		if (InspectorDock::get_singleton()) {
			bool is_vis = InspectorDock::get_singleton()->is_visible();
			InspectorDock::get_singleton()->set_visible(!is_vis);
			toggle_inspector_btn->set_text(is_vis ? TTR("Show Inspector") : TTR("Hide Inspector"));
		}
	} else {
		bool is_vis = right_panel_container->is_visible();
		right_panel_container->set_visible(!is_vis);
		toggle_inspector_btn->set_text(is_vis ? TTR("Show Inspector") : TTR("Hide Inspector"));
	}
}

void CbtApplicationShell::_on_save_current_pressed() {
	if (editor_node) {
		editor_node->save_scene_to_path("");
	}
}

void CbtApplicationShell::_on_save_all_pressed() {
	if (editor_node) {
		editor_node->save_all_scenes();
	}
}

void CbtApplicationShell::_on_launcher_btn_pressed() {
	show_project_launcher();
}

void CbtApplicationShell::_on_launcher_close_requested() {
	hide_project_launcher();
}

void CbtApplicationShell::_on_launcher_project_selected(const String &p_path) {
	hide_project_launcher();
	List<String> args;
	for (const String &a : Main::get_forwardable_cli_arguments(Main::CLI_SCOPE_TOOL)) {
		args.push_back(a);
	}
	args.push_back("--path");
	args.push_back(p_path);
	args.push_back("--editor");
	OS::get_singleton()->create_instance(args);
	get_tree()->quit();
}

void CbtApplicationShell::show_project_launcher() {
	if (launcher_overlay) {
		launcher_overlay->refresh();
		launcher_overlay->show_home();
		launcher_overlay->set_visible(true);
	}
}

void CbtApplicationShell::hide_project_launcher() {
	if (launcher_overlay) {
		launcher_overlay->set_visible(false);
	}
}
