/**************************************************************************/
/*  cbt_workspace_plugin.h - CBT Domain Workspace EditorPlugin            */
/**************************************************************************/

#pragma once

#include "cbt_theme.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"

class CbtWorkspaceControl : public PanelContainer {
	GDCLASS(CbtWorkspaceControl, PanelContainer);

private:
	int current_view_id = 0;
	String current_view_name = "Products";

	HSplitContainer *main_hsplit = nullptr;
	HSplitContainer *right_hsplit = nullptr;

	PanelContainer *left_panel = nullptr;
	PanelContainer *center_panel = nullptr;
	PanelContainer *right_panel = nullptr;

	Label *left_title = nullptr;
	LineEdit *left_search = nullptr;
	VBoxContainer *left_items_vbox = nullptr;

	Label *center_title = nullptr;
	Label *center_subtitle = nullptr;
	VBoxContainer *center_content = nullptr;

	Label *right_title = nullptr;
	VBoxContainer *right_properties_vbox = nullptr;

	void _build_ui();
	void _refresh_view();

public:
	void set_view(int p_view_id, const String &p_name, const String &p_category);
	void toggle_left_panel();
	void toggle_right_panel();
	bool is_left_panel_visible() const { return left_panel->is_visible(); }
	bool is_right_panel_visible() const { return right_panel->is_visible(); }

	CbtWorkspaceControl();
};

class CbtWorkspacePlugin : public EditorPlugin {
	GDCLASS(CbtWorkspacePlugin, EditorPlugin);

private:
	static CbtWorkspacePlugin *singleton;
	CbtWorkspaceControl *workspace_control = nullptr;

public:
	static CbtWorkspacePlugin *get_singleton() { return singleton; }

	virtual String get_plugin_name() const override { return "CBT Workspace"; }
	virtual bool has_main_screen() const override { return true; }
	virtual void make_visible(bool p_visible) override;

	void set_view(int p_view_id, const String &p_name, const String &p_category);
	void toggle_left_panel();
	void toggle_right_panel();
	bool is_left_panel_visible() const;
	bool is_right_panel_visible() const;

	CbtWorkspacePlugin();
	~CbtWorkspacePlugin();
};
