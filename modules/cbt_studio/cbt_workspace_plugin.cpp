/**************************************************************************/
/*  cbt_workspace_plugin.cpp - CBT Domain Workspace EditorPlugin          */
/**************************************************************************/

#include "cbt_workspace_plugin.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"

// --- CbtWorkspaceControl ---

CbtWorkspaceControl::CbtWorkspaceControl() {
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_h_size_flags(Control::SIZE_EXPAND_FILL);
	set_v_size_flags(Control::SIZE_EXPAND_FILL);

	Ref<StyleBoxFlat> bg = CbtTheme::create_flat_box(CbtTheme::COLOR_BG_DARKEST, Color(0, 0, 0, 0), 0, 0, 0, 0);
	add_theme_style_override("panel", bg);

	_build_ui();
	_refresh_view();
}

void CbtWorkspaceControl::_build_ui() {
	main_hsplit = memnew(HSplitContainer);
	main_hsplit->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	main_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	main_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(main_hsplit);

	// 1. Left Panel (Domain Collection Navigation)
	left_panel = memnew(PanelContainer);
	left_panel->set_custom_minimum_size(Vector2(Math::round(260 * EDSCALE), 0));
	left_panel->add_theme_style_override("panel", CbtTheme::create_panel_box());
	main_hsplit->add_child(left_panel);

	VBoxContainer *left_vb = memnew(VBoxContainer);
	left_vb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_vb->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	left_vb->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	left_panel->add_child(left_vb);

	MarginContainer *left_margin = memnew(MarginContainer);
	left_margin->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_margin->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	float s = EDSCALE;
	left_margin->add_theme_constant_override("margin_left", Math::round(10 * s));
	left_margin->add_theme_constant_override("margin_right", Math::round(10 * s));
	left_margin->add_theme_constant_override("margin_top", Math::round(10 * s));
	left_margin->add_theme_constant_override("margin_bottom", Math::round(10 * s));
	left_vb->add_child(left_margin);

	VBoxContainer *left_content = memnew(VBoxContainer);
	left_content->add_theme_constant_override("separation", Math::round(8 * s));
	left_margin->add_child(left_content);

	left_title = memnew(Label);
	left_title->set_text(TTR("Items"));
	left_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	left_title->add_theme_font_size_override("font_size", Math::round(13 * s));
	left_content->add_child(left_title);

	left_search = memnew(LineEdit);
	left_search->set_placeholder(TTR("Filter collection…"));
	left_content->add_child(left_search);

	ScrollContainer *left_scroll = memnew(ScrollContainer);
	left_scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	left_content->add_child(left_scroll);

	left_items_vbox = memnew(VBoxContainer);
	left_items_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	left_items_vbox->add_theme_constant_override("separation", Math::round(4 * s));
	left_scroll->add_child(left_items_vbox);

	// 2. Right Split (Center Workspace vs Right Inspector)
	right_hsplit = memnew(HSplitContainer);
	right_hsplit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	right_hsplit->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_hsplit->add_child(right_hsplit);

	// Center Workspace
	center_panel = memnew(PanelContainer);
	center_panel->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	center_panel->set_custom_minimum_size(Vector2(Math::round(400 * EDSCALE), 0));
	center_panel->add_theme_style_override("panel", CbtTheme::create_flat_box(CbtTheme::COLOR_BG_DARKEST, Color(0, 0, 0, 0), 0, 0, 0, 0));
	right_hsplit->add_child(center_panel);

	VBoxContainer *center_vb = memnew(VBoxContainer);
	center_vb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_vb->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	center_vb->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	center_vb->add_theme_constant_override("separation", Math::round(12 * s));
	center_panel->add_child(center_vb);

	center_title = memnew(Label);
	center_title->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	center_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	center_title->add_theme_font_size_override("font_size", Math::round(18 * s));
	center_vb->add_child(center_title);

	center_subtitle = memnew(Label);
	center_subtitle->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	center_subtitle->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	center_subtitle->add_theme_font_size_override("font_size", Math::round(12 * s));
	center_vb->add_child(center_subtitle);

	center_content = memnew(VBoxContainer);
	center_content->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	center_content->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	center_vb->add_child(center_content);

	// 3. Right Inspector
	right_panel = memnew(PanelContainer);
	right_panel->set_custom_minimum_size(Vector2(Math::round(280 * EDSCALE), 0));
	right_panel->add_theme_style_override("panel", CbtTheme::create_panel_box());
	right_hsplit->add_child(right_panel);

	MarginContainer *right_margin = memnew(MarginContainer);
	right_margin->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	right_margin->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	right_margin->add_theme_constant_override("margin_left", Math::round(12 * s));
	right_margin->add_theme_constant_override("margin_right", Math::round(12 * s));
	right_margin->add_theme_constant_override("margin_top", Math::round(12 * s));
	right_margin->add_theme_constant_override("margin_bottom", Math::round(12 * s));
	right_panel->add_child(right_margin);

	VBoxContainer *right_vb = memnew(VBoxContainer);
	right_vb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	right_vb->add_theme_constant_override("separation", Math::round(8 * s));
	right_margin->add_child(right_vb);

	right_title = memnew(Label);
	right_title->set_text(TTR("Item Properties"));
	right_title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	right_title->add_theme_font_size_override("font_size", Math::round(12 * s));
	right_vb->add_child(right_title);

	right_vb->add_child(memnew(HSeparator));

	right_properties_vbox = memnew(VBoxContainer);
	right_properties_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	right_properties_vbox->add_theme_constant_override("separation", Math::round(6 * s));
	right_vb->add_child(right_properties_vbox);

	// Default split offsets
	main_hsplit->set_split_offset(Math::round(260 * s));
	right_hsplit->set_split_offset(Math::round(620 * s));
}

void CbtWorkspaceControl::set_view(int p_view_id, const String &p_name, const String &p_category) {
	current_view_id = p_view_id;
	current_view_name = p_name;
	_refresh_view();
}

void CbtWorkspaceControl::_refresh_view() {
	left_title->set_text(vformat(TTR("%s Collection"), current_view_name));
	center_title->set_text(current_view_name);
	center_subtitle->set_text(vformat(TTR("Authoring workspace for '%s' (Canonical CBT domain view)"), current_view_name));

	// Clear and populate sample items
	while (left_items_vbox->get_child_count() > 0) {
		Node *c = left_items_vbox->get_child(0);
		left_items_vbox->remove_child(c);
		memdelete(c);
	}

	for (int i = 1; i <= 5; i++) {
		Button *item_btn = memnew(Button);
		item_btn->set_text(vformat("%s Item #%02d", current_view_name, i));
		item_btn->set_flat(true);
		item_btn->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
		item_btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
		left_items_vbox->add_child(item_btn);
	}

	// Right properties
	while (right_properties_vbox->get_child_count() > 0) {
		Node *c = right_properties_vbox->get_child(0);
		right_properties_vbox->remove_child(c);
		memdelete(c);
	}

	Label *prop1 = memnew(Label);
	prop1->set_text(TTR("Select an item to view properties."));
	prop1->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	prop1->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	right_properties_vbox->add_child(prop1);
}

void CbtWorkspaceControl::toggle_left_panel() {
	left_panel->set_visible(!left_panel->is_visible());
}

void CbtWorkspaceControl::toggle_right_panel() {
	right_panel->set_visible(!right_panel->is_visible());
}

// --- CbtWorkspacePlugin ---

CbtWorkspacePlugin *CbtWorkspacePlugin::singleton = nullptr;

CbtWorkspacePlugin::CbtWorkspacePlugin() {
	singleton = this;
	workspace_control = memnew(CbtWorkspaceControl);
	EditorNode::get_editor_main_screen()->get_control()->add_child(workspace_control);
	workspace_control->set_visible(false);
}

CbtWorkspacePlugin::~CbtWorkspacePlugin() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

void CbtWorkspacePlugin::make_visible(bool p_visible) {
	if (workspace_control) {
		workspace_control->set_visible(p_visible);
	}
}

void CbtWorkspacePlugin::set_view(int p_view_id, const String &p_name, const String &p_category) {
	if (workspace_control) {
		workspace_control->set_view(p_view_id, p_name, p_category);
	}
}

void CbtWorkspacePlugin::toggle_left_panel() {
	if (workspace_control) {
		workspace_control->toggle_left_panel();
	}
}

void CbtWorkspacePlugin::toggle_right_panel() {
	if (workspace_control) {
		workspace_control->toggle_right_panel();
	}
}

bool CbtWorkspacePlugin::is_left_panel_visible() const {
	return workspace_control && workspace_control->is_left_panel_visible();
}

bool CbtWorkspacePlugin::is_right_panel_visible() const {
	return workspace_control && workspace_control->is_right_panel_visible();
}
