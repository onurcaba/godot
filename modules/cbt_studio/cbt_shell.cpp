/**************************************************************************/
/*  cbt_shell.cpp - CBT Content Studio Application Shell & Navigation     */
/**************************************************************************/

#include "cbt_shell.h"
#include "core/config/project_settings.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "editor/docks/editor_dock_manager.h"
#include "editor/docks/inspector_dock.h"
#include "editor/docks/scene_tree_dock.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/themes/editor_scale.h"
#include "main/main.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/scroll_container.h"
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

	// Initialize history
	nav_history.push_back(VIEW_SCENES);
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
	// Top Bars Background
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
	row1_app_bar->set_custom_minimum_size(Vector2(0, Math::round(36 * EDSCALE)));
	row1_app_bar->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	add_child(row1_app_bar);

	// CBT Brand Icon & Title
	Label *brand_lbl = memnew(Label);
	brand_lbl->set_text(TTR("CBT CONTENT STUDIO"));
	brand_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
	brand_lbl->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
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

	// Right Utilities
	project_badge = memnew(Label);
	project_badge->set_text(TTR("Project: Loading"));
	project_badge->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	project_badge->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	row1_app_bar->add_child(project_badge);

	status_badge = memnew(Label);
	status_badge->set_text(TTR("Saved"));
	status_badge->add_theme_style_override("normal", CbtTheme::create_badge_box(Color(0.06f, 0.72f, 0.5f, 0.2f), CbtTheme::COLOR_SUCCESS));
	status_badge->add_theme_color_override("font_color", CbtTheme::COLOR_SUCCESS);
	status_badge->add_theme_font_size_override("font_size", Math::round(10 * EDSCALE));
	row1_app_bar->add_child(status_badge);

	launcher_btn = memnew(Button);
	launcher_btn->set_text(TTR("Projects ⊞"));
	launcher_btn->set_flat(true);
	launcher_btn->set_tooltip_text(TTR("Return to Project Launcher"));
	launcher_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtApplicationShell::_on_launcher_btn_pressed));
	row1_app_bar->add_child(launcher_btn);

	settings_btn = memnew(Button);
	settings_btn->set_text("⚙");
	settings_btn->set_flat(true);
	settings_btn->set_tooltip_text(TTR("Content Studio Settings"));
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

	// Helper lambda to register destination group
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
	content_items.push_back(Pair<ViewId, String>(VIEW_SCENES, "Scenes"));
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
	// Root horizontal split (Left vs [Center + Right])
	workspace_main_hsplit = memnew(HSplitContainer);
	workspace_main_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_main_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(workspace_main_hsplit);

	// 1. Left Panel Container
	left_panel_container = memnew(PanelContainer);
	left_panel_container->set_custom_minimum_size(Vector2(Math::round(240 * EDSCALE), 0));
	left_panel_container->add_theme_style_override("panel", CbtTheme::create_panel_box());
	workspace_main_hsplit->add_child(left_panel_container);

	// Left Panel Domain Navigator (when not in Scene mode)
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

	LineEdit *search_edit = memnew(LineEdit);
	search_edit->set_placeholder(TTR("Filter items…"));
	domain_left_nav->add_child(search_edit);

	ScrollContainer *left_scroll = memnew(ScrollContainer);
	left_scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_left_nav->add_child(left_scroll);

	domain_left_items = memnew(VBoxContainer);
	domain_left_items->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_scroll->add_child(domain_left_items);

	// 2. Right Split (Center Workspace vs Right Inspector)
	workspace_right_hsplit = memnew(HSplitContainer);
	workspace_right_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_right_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	workspace_main_hsplit->add_child(workspace_right_hsplit);

	// Center Workspace Container
	center_panel_container = memnew(PanelContainer);
	center_panel_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel_container->set_custom_minimum_size(Vector2(Math::round(440 * EDSCALE), 0));
	center_panel_container->add_theme_style_override("panel", CbtTheme::create_flat_box(CbtTheme::COLOR_BG_DARKEST, Color(0, 0, 0, 0), 0, 0, 0, 0));
	workspace_right_hsplit->add_child(center_panel_container);

	// Center Domain View (for non-Scene views)
	domain_center_view = memnew(VBoxContainer);
	domain_center_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_center_view->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	domain_center_view->set_visible(false);
	center_panel_container->add_child(domain_center_view);

	domain_center_title = memnew(Label);
	domain_center_title->set_text(TTR("CBT Domain Workspace"));
	domain_center_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	domain_center_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	domain_center_title->add_theme_font_size_override("font_size", Math::round(18 * EDSCALE));
	domain_center_view->add_child(domain_center_title);

	domain_center_desc = memnew(Label);
	domain_center_desc->set_text(TTR("Authoring views will be integrated in subsequent milestones."));
	domain_center_desc->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	domain_center_desc->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	domain_center_desc->add_theme_font_size_override("font_size", Math::round(13 * EDSCALE));
	domain_center_view->add_child(domain_center_desc);

	// 3. Right Inspector Container
	right_panel_container = memnew(PanelContainer);
	right_panel_container->set_custom_minimum_size(Vector2(Math::round(280 * EDSCALE), 0));
	right_panel_container->add_theme_style_override("panel", CbtTheme::create_panel_box());
	workspace_right_hsplit->add_child(right_panel_container);

	domain_right_inspector = memnew(VBoxContainer);
	domain_right_inspector->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	domain_right_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	domain_right_inspector->set_visible(false);
	right_panel_container->add_child(domain_right_inspector);

	domain_right_title = memnew(Label);
	domain_right_title->set_text(TTR("Inspector"));
	domain_right_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	domain_right_title->add_theme_font_size_override("font_size", Math::round(13 * EDSCALE));
	domain_right_inspector->add_child(domain_right_title);

	// Default split offsets (Left: 280px, Center: flex, Right: 320px)
	workspace_main_hsplit->set_split_offset(Math::round(280 * EDSCALE));
	workspace_right_hsplit->set_split_offset(Math::round(600 * EDSCALE));
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
		set_project_name(appname.is_empty() ? "CBT Project" : appname);
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

	// Set initial domain and view
	switch_to_domain(DOMAIN_CONTENT);
	switch_to_view(VIEW_SCENES);
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
		set_v_size_flags(Control::SIZE_SHRINK_BEGIN);
		if (EditorNode::get_editor_main_screen()) {
			EditorNode::get_editor_main_screen()->select(EditorMainScreen::EDITOR_3D);
		}
		domain_center_view->set_visible(false);
		domain_left_nav->set_visible(false);
		domain_right_inspector->set_visible(false);

		// Show Godot Native SceneTree & Inspector
		if (SceneTreeDock::get_singleton()) {
			SceneTreeDock::get_singleton()->show();
		}
		if (InspectorDock::get_singleton()) {
			InspectorDock::get_singleton()->show();
		}
	} else {
		set_v_size_flags(Control::SIZE_EXPAND_FILL);
		workspace_main_hsplit->set_visible(true);
		workspace_main_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		domain_center_view->set_visible(true);
		domain_left_nav->set_visible(true);
		domain_right_inspector->set_visible(true);

		Button *active_btn = view_buttons.has(active_view) ? view_buttons[active_view] : nullptr;
		String view_name = active_btn ? active_btn->get_text() : "CBT View";

		domain_center_title->set_text(view_name);
		domain_center_desc->set_text(vformat(TTR("Authoring workspace for '%s' will be integrated in milestone M2."), view_name));
		domain_left_title->set_text(vformat(TTR("%s Catalog"), view_name));

		// Populate domain sample items
		while (domain_left_items->get_child_count() > 0) {
			Node *c = domain_left_items->get_child(0);
			domain_left_items->remove_child(c);
			memdelete(c);
		}

		for (int i = 1; i <= 4; i++) {
			Button *item = memnew(Button);
			item->set_text(vformat(TTR("%s #%02d"), view_name, i));
			item->set_flat(true);
			item->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
			item->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
			domain_left_items->add_child(item);
		}
	}

	emit_signal("view_changed", (int)active_view);
}

void CbtApplicationShell::switch_to_domain(DomainId p_domain) {
	_update_domain_selection(p_domain);
	// Switch to first view in this domain
	switch (p_domain) {
		case DOMAIN_AUTHORING:
			_update_view_selection(VIEW_PRODUCTS);
			break;
		case DOMAIN_CONTENT:
			_update_view_selection(VIEW_SCENES);
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

void CbtApplicationShell::switch_to_view(ViewId p_view) {
	// Find domain for this view
	if (p_view <= VIEW_MODULES) {
		_update_domain_selection(DOMAIN_AUTHORING);
	} else if (p_view <= VIEW_MATERIALS) {
		_update_domain_selection(DOMAIN_CONTENT);
	} else if (p_view <= VIEW_PUBLISH) {
		_update_domain_selection(DOMAIN_PRODUCT);
	} else if (p_view <= VIEW_INTERACTIONS) {
		_update_domain_selection(DOMAIN_SYSTEM);
	} else {
		_update_domain_selection(DOMAIN_DEVELOPMENT);
	}
	_update_view_selection(p_view);
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
