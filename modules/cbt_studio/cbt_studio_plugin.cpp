/**************************************************************************/
/*  cbt_studio_plugin.cpp - CBT Content Studio Main EditorPlugin          */
/**************************************************************************/

#include "cbt_studio_plugin.h"
#include "cbt_theme.h"
#include "editor/editor_interface.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/separator.h"

// --- CbtTopBarSwitcher ---

void CbtTopBarSwitcher::_bind_methods() {}

CbtTopBarSwitcher::CbtTopBarSwitcher() {
	set_alignment(BoxContainer::ALIGNMENT_CENTER);
	add_theme_constant_override("separation", Math::round(4 * EDSCALE));

	// Brand Button
	brand_btn = memnew(Button);
	brand_btn->set_text(TTR("⚡ CBT Studio"));
	brand_btn->set_flat(true);
	brand_btn->set_tooltip_text(TTR("Switch to CBT Content Studio Workspace"));
	brand_btn->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
	brand_btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	brand_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtTopBarSwitcher::_on_brand_pressed));
	add_child(brand_btn);

	VSeparator *sep = memnew(VSeparator);
	add_child(sep);

	// Quick Domain Buttons
	const char *domain_names[CbtApplicationShell::DOMAIN_MAX] = {
		"Authoring",
		"Content",
		"Product",
		"System",
		"Development"
	};

	for (int i = 0; i < CbtApplicationShell::DOMAIN_MAX; i++) {
		Button *btn = memnew(Button);
		btn->set_text(TTR(domain_names[i]));
		btn->set_flat(true);
		btn->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
		btn->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
		btn->connect(SceneStringName(pressed), callable_mp(this, &CbtTopBarSwitcher::_on_domain_pressed).bind(i));
		add_child(btn);
		domain_buttons.push_back(btn);
	}
}

CbtTopBarSwitcher::~CbtTopBarSwitcher() {}

void CbtTopBarSwitcher::_notification(int p_what) {
	if (p_what == NOTIFICATION_THEME_CHANGED) {
		if (brand_btn) {
			brand_btn->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
		}
	}
}

void CbtTopBarSwitcher::_on_brand_pressed() {
	if (EditorInterface::get_singleton()) {
		EditorInterface::get_singleton()->set_main_screen_editor("CBT Studio");
	}
}

void CbtTopBarSwitcher::_on_domain_pressed(int p_domain) {
	if (CbtStudioPlugin::get_singleton()) {
		CbtStudioPlugin::get_singleton()->select_domain(p_domain);
	}
}

void CbtTopBarSwitcher::update_active_domain(int p_domain) {
	for (int i = 0; i < domain_buttons.size(); i++) {
		bool is_active = (i == p_domain);
		domain_buttons[i]->add_theme_color_override(
				"font_color",
				is_active ? Color(1, 1, 1) : CbtTheme::COLOR_TEXT_SECONDARY);
		domain_buttons[i]->add_theme_style_override(
				"normal",
				CbtTheme::create_domain_tab_box(is_active, false));
		domain_buttons[i]->add_theme_style_override(
				"hover",
				CbtTheme::create_domain_tab_box(is_active, true));
	}
}

// --- CbtStudioPlugin ---

CbtStudioPlugin *CbtStudioPlugin::singleton = nullptr;

void CbtStudioPlugin::_bind_methods() {}

CbtStudioPlugin::CbtStudioPlugin() {
	singleton = this;
}

CbtStudioPlugin::~CbtStudioPlugin() {
	if (spatial_menu_btn) {
		remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, spatial_menu_btn);
		memdelete(spatial_menu_btn);
		spatial_menu_btn = nullptr;
	}

	if (top_switcher) {
		remove_control_from_container(CONTAINER_TOOLBAR, top_switcher);
		memdelete(top_switcher);
		top_switcher = nullptr;
	}

	if (shell) {
		if (shell->get_parent()) {
			shell->get_parent()->remove_child(shell);
		}
		memdelete(shell);
		shell = nullptr;
	}

	if (singleton == this) {
		singleton = nullptr;
	}
}

void CbtStudioPlugin::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		if (!is_initialized) {
			is_initialized = true;

			// 1. Create and add CbtApplicationShell to Editor Main Screen
			shell = memnew(CbtApplicationShell);
			if (EditorInterface::get_singleton() && EditorInterface::get_singleton()->get_editor_main_screen()) {
				EditorInterface::get_singleton()->get_editor_main_screen()->add_child(shell);
			}
			shell->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
			shell->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			shell->hide();

			shell->initialize_with_editor(EditorNode::get_singleton());
			shell->connect("domain_changed", callable_mp(this, &CbtStudioPlugin::_on_shell_domain_changed));
			shell->connect("view_changed", callable_mp(this, &CbtStudioPlugin::_on_shell_view_changed));

			// 2. Add Top Bar Switcher to CONTAINER_TOOLBAR
			top_switcher = memnew(CbtTopBarSwitcher);
			add_control_to_container(CONTAINER_TOOLBAR, top_switcher);

			// 3. Add Quick Return button to 3D Viewport header (CONTAINER_SPATIAL_EDITOR_MENU)
			spatial_menu_btn = memnew(Button);
			spatial_menu_btn->set_text(TTR("⚡ Return to CBT Studio"));
			spatial_menu_btn->set_flat(true);
			spatial_menu_btn->set_tooltip_text(TTR("Return to CBT Content Studio Workspace"));
			spatial_menu_btn->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
			spatial_menu_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtStudioPlugin::_on_spatial_return_pressed));
			add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, spatial_menu_btn);

			// 4. Default to CBT Studio on startup
			if (EditorInterface::get_singleton()) {
				callable_mp(EditorInterface::get_singleton(), &EditorInterface::set_main_screen_editor).call_deferred("CBT Studio");
			}
		}
	}
}

void CbtStudioPlugin::make_visible(bool p_visible) {
	if (shell) {
		shell->set_visible(p_visible);
		if (p_visible) {
			shell->on_main_screen_activated();
		}
	}
}

void CbtStudioPlugin::select_domain(int p_domain) {
	if (p_domain == CbtApplicationShell::DOMAIN_CONTENT && shell && shell->get_active_view() == CbtApplicationShell::VIEW_SCENES) {
		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_main_screen_editor("3D");
		}
	} else {
		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_main_screen_editor("CBT Studio");
		}
		if (shell) {
			shell->switch_to_domain((CbtApplicationShell::DomainId)p_domain);
		}
	}
}

void CbtStudioPlugin::select_view(int p_view) {
	if (p_view == CbtApplicationShell::VIEW_SCENES) {
		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_main_screen_editor("3D");
		}
		if (shell) {
			shell->switch_to_view(CbtApplicationShell::VIEW_SCENES);
		}
	} else {
		if (EditorInterface::get_singleton()) {
			EditorInterface::get_singleton()->set_main_screen_editor("CBT Studio");
		}
		if (shell) {
			shell->switch_to_view((CbtApplicationShell::ViewId)p_view);
		}
	}
}

void CbtStudioPlugin::_on_spatial_return_pressed() {
	if (EditorInterface::get_singleton()) {
		EditorInterface::get_singleton()->set_main_screen_editor("CBT Studio");
	}
	if (shell && shell->get_active_view() == CbtApplicationShell::VIEW_SCENES) {
		shell->switch_to_view(CbtApplicationShell::VIEW_SCENARIOS);
	}
}

void CbtStudioPlugin::_on_shell_domain_changed(int p_domain) {
	if (top_switcher) {
		top_switcher->update_active_domain(p_domain);
	}
}

void CbtStudioPlugin::_on_shell_view_changed(int p_view) {
	// Synchronize any view-level states if needed
}
