/**************************************************************************/
/*  cbt_studio_plugin.h - CBT Content Studio Main EditorPlugin            */
/**************************************************************************/

#pragma once

#include "cbt_shell.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"

class CbtTopBarSwitcher : public HBoxContainer {
	GDCLASS(CbtTopBarSwitcher, HBoxContainer);

private:
	Button *brand_btn = nullptr;
	Vector<Button *> domain_buttons;

	void _on_brand_pressed();
	void _on_domain_pressed(int p_domain);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void update_active_domain(int p_domain);

	CbtTopBarSwitcher();
	~CbtTopBarSwitcher();
};

class CbtStudioPlugin : public EditorPlugin {
	GDCLASS(CbtStudioPlugin, EditorPlugin);

private:
	static CbtStudioPlugin *singleton;

	CbtApplicationShell *shell = nullptr;
	CbtTopBarSwitcher *top_switcher = nullptr;
	Button *spatial_menu_btn = nullptr;

	bool is_initialized = false;

	void _on_shell_domain_changed(int p_domain);
	void _on_shell_view_changed(int p_view);
	void _on_spatial_return_pressed();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	static CbtStudioPlugin *get_singleton() { return singleton; }

	virtual String get_plugin_name() const override { return "CBT Studio"; }
	virtual bool has_main_screen() const override { return true; }
	virtual void make_visible(bool p_visible) override;

	CbtApplicationShell *get_shell() const { return shell; }
	void select_domain(int p_domain);
	void select_view(int p_view);

	CbtStudioPlugin();
	~CbtStudioPlugin();
};
