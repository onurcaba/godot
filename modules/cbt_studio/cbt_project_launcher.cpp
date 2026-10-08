/**************************************************************************/
/*  cbt_project_launcher.cpp - CBT Content Studio Project Launcher UI     */
/**************************************************************************/

#include "cbt_project_launcher.h"
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/io/config_file.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/os/os.h"
#include "editor/file_system/editor_paths.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "main/main.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"
#include "scene/main/scene_tree.h"

void CbtProjectLauncher::_bind_methods() {
	ADD_SIGNAL(MethodInfo("project_selected", PropertyInfo(Variant::STRING, "path")));
	ADD_SIGNAL(MethodInfo("close_requested"));
}

CbtProjectLauncher::CbtProjectLauncher() {
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);

	// Root Styling: #101114 Dark Background
	Ref<StyleBoxFlat> bg = CbtTheme::create_flat_box(CbtTheme::COLOR_BG_DARKEST, Color(0, 0, 0, 0), 0, 0, 0, 0);
	add_theme_style_override("panel", bg);

	MarginContainer *margin = memnew(MarginContainer);
	margin->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	float s = EDSCALE;
	margin->add_theme_constant_override("margin_left", Math::round(32 * s));
	margin->add_theme_constant_override("margin_right", Math::round(32 * s));
	margin->add_theme_constant_override("margin_top", Math::round(24 * s));
	margin->add_theme_constant_override("margin_bottom", Math::round(24 * s));
	add_child(margin);

	main_layout = memnew(VBoxContainer);
	main_layout->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	main_layout->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	margin->add_child(main_layout);

	_build_header();
	_build_home_view();
	_build_repo_view();
	_build_dialogs();

	refresh();
}

CbtProjectLauncher::~CbtProjectLauncher() {
}

void CbtProjectLauncher::_notification(int p_what) {
	if (p_what == NOTIFICATION_ENTER_TREE) {
		refresh();
	}
}

void CbtProjectLauncher::_build_header() {
	HBoxContainer *header = memnew(HBoxContainer);
	header->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	main_layout->add_child(header);

	// Brand title: COMPUTER BASED TRAINING · CONTENT STUDIO
	Label *brand = memnew(Label);
	brand->set_text(TTR("COMPUTER BASED TRAINING · CONTENT STUDIO"));
	brand->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
	brand->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	brand->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	header->add_child(brand);

	close_button = memnew(Button);
	close_button->set_text("✕");
	close_button->set_flat(true);
	close_button->set_tooltip_text(TTR("Close Launcher"));
	close_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_close_pressed));
	header->add_child(close_button);

	HSeparator *sep = memnew(HSeparator);
	sep->add_theme_constant_override("separation", Math::round(16 * EDSCALE));
	main_layout->add_child(sep);
}

void CbtProjectLauncher::_build_home_view() {
	home_view = memnew(VBoxContainer);
	home_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	home_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_layout->add_child(home_view);

	// Welcome & Subtitle
	Label *title = memnew(Label);
	title->set_text(TTR("Welcome to Content Studio"));
	title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	title->add_theme_font_size_override("font_size", Math::round(20 * EDSCALE));
	home_view->add_child(title);

	Label *subtitle = memnew(Label);
	subtitle->set_text(TTR("Continue working or start something new."));
	subtitle->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	subtitle->add_theme_font_size_override("font_size", Math::round(13 * EDSCALE));
	home_view->add_child(subtitle);

	// Error / Status banner
	error_label = memnew(Label);
	error_label->add_theme_color_override("font_color", CbtTheme::COLOR_DANGER);
	error_label->set_visible(false);
	home_view->add_child(error_label);

	// Primary Actions Bar: [+ New Project]  [Open Project]
	HBoxContainer *actions_row = memnew(HBoxContainer);
	actions_row->add_theme_constant_override("separation", Math::round(12 * EDSCALE));
	actions_row->set_custom_minimum_size(Vector2(0, Math::round(48 * EDSCALE)));
	home_view->add_child(actions_row);

	new_project_button = memnew(Button);
	new_project_button->set_text(TTR("+ New Project"));
	new_project_button->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	new_project_button->add_theme_style_override("hover", CbtTheme::create_button_box(true, true, false));
	new_project_button->add_theme_style_override("pressed", CbtTheme::create_button_box(true, false, true));
	new_project_button->add_theme_color_override("font_color", Color(1, 1, 1));
	new_project_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_new_project_pressed));
	actions_row->add_child(new_project_button);

	open_project_button = memnew(Button);
	open_project_button->set_text(TTR("Open Project"));
	open_project_button->add_theme_style_override("normal", CbtTheme::create_button_box(false, false, false));
	open_project_button->add_theme_style_override("hover", CbtTheme::create_button_box(false, true, false));
	open_project_button->add_theme_style_override("pressed", CbtTheme::create_button_box(false, false, true));
	open_project_button->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	open_project_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_open_project_pressed));
	actions_row->add_child(open_project_button);

	// Recent Projects Section
	recent_section = memnew(VBoxContainer);
	recent_section->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	recent_section->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	home_view->add_child(recent_section);

	Label *recent_heading = memnew(Label);
	recent_heading->set_text(TTR("Recent Projects"));
	recent_heading->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	recent_heading->add_theme_font_size_override("font_size", Math::round(14 * EDSCALE));
	recent_section->add_child(recent_heading);

	ScrollContainer *scroll = memnew(ScrollContainer);
	scroll->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	recent_section->add_child(scroll);

	recent_list = memnew(VBoxContainer);
	recent_list->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	recent_list->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	scroll->add_child(recent_list);

	// Empty state
	empty_state = memnew(VBoxContainer);
	empty_state->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	empty_state->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	empty_state->set_custom_minimum_size(Vector2(0, Math::round(120 * EDSCALE)));
	empty_state->set_visible(false);
	home_view->add_child(empty_state);

	Label *empty_label = memnew(Label);
	empty_label->set_text(TTR("Create your first project or open an existing one."));
	empty_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	empty_label->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	empty_state->add_child(empty_label);

	// Quick-Add Repository Section
	HSeparator *sep2 = memnew(HSeparator);
	sep2->add_theme_constant_override("separation", Math::round(16 * EDSCALE));
	home_view->add_child(sep2);

	VBoxContainer *quick_repo = memnew(VBoxContainer);
	quick_repo->add_theme_constant_override("separation", Math::round(6 * EDSCALE));
	home_view->add_child(quick_repo);

	Label *repo_label = memnew(Label);
	repo_label->set_text(TTR("Have a project repository URL?"));
	repo_label->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_SECONDARY);
	repo_label->add_theme_font_size_override("font_size", Math::round(12 * EDSCALE));
	quick_repo->add_child(repo_label);

	HBoxContainer *quick_repo_row = memnew(HBoxContainer);
	quick_repo_row->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
	quick_repo->add_child(quick_repo_row);

	quick_repo_url_input = memnew(LineEdit);
	quick_repo_url_input->set_placeholder("https://github.com/company/training-project.git");
	quick_repo_url_input->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	quick_repo_row->add_child(quick_repo_url_input);

	quick_add_button = memnew(Button);
	quick_add_button->set_text(TTR("Add"));
	quick_add_button->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	quick_add_button->add_theme_style_override("hover", CbtTheme::create_button_box(true, true, false));
	quick_add_button->add_theme_style_override("pressed", CbtTheme::create_button_box(true, false, true));
	quick_add_button->add_theme_color_override("font_color", Color(1, 1, 1));
	quick_add_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_quick_add_pressed));
	quick_repo_row->add_child(quick_add_button);

	advanced_button = memnew(Button);
	advanced_button->set_text(TTR("Advanced…"));
	advanced_button->set_flat(true);
	advanced_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_advanced_pressed));
	quick_repo_row->add_child(advanced_button);

	status_label = memnew(Label);
	status_label->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	status_label->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
	status_label->set_visible(false);
	quick_repo->add_child(status_label);
}

void CbtProjectLauncher::_build_repo_view() {
	repo_view = memnew(VBoxContainer);
	repo_view->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	repo_view->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	repo_view->set_visible(false);
	main_layout->add_child(repo_view);

	HBoxContainer *title_row = memnew(HBoxContainer);
	repo_view->add_child(title_row);

	Button *back_btn = memnew(Button);
	back_btn->set_text("←");
	back_btn->set_flat(true);
	back_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_repo_cancel_pressed));
	title_row->add_child(back_btn);

	VBoxContainer *titles = memnew(VBoxContainer);
	title_row->add_child(titles);

	Label *title = memnew(Label);
	title->set_text(TTR("Get Project from Repository"));
	title->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
	title->add_theme_font_size_override("font_size", Math::round(18 * EDSCALE));
	titles->add_child(title);

	Label *sub = memnew(Label);
	sub->set_text(TTR("Clone a Git training project directly into your workspace."));
	sub->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
	titles->add_child(sub);

	// Fields
	VBoxContainer *fields = memnew(VBoxContainer);
	fields->add_theme_constant_override("separation", Math::round(12 * EDSCALE));
	fields->set_custom_minimum_size(Vector2(Math::round(480 * EDSCALE), 0));
	repo_view->add_child(fields);

	Label *l_url = memnew(Label);
	l_url->set_text(TTR("Repository URL:"));
	fields->add_child(l_url);

	repo_url_field = memnew(LineEdit);
	repo_url_field->set_placeholder("https://github.com/company/training-project.git");
	fields->add_child(repo_url_field);

	Label *l_dest = memnew(Label);
	l_dest->set_text(TTR("Destination Parent Directory:"));
	fields->add_child(l_dest);

	dest_parent_field = memnew(LineEdit);
	dest_parent_field->set_text(OS::get_singleton()->get_user_data_dir());
	fields->add_child(dest_parent_field);

	Label *l_name = memnew(Label);
	l_name->set_text(TTR("Project Folder Name:"));
	fields->add_child(l_name);

	project_name_field = memnew(LineEdit);
	project_name_field->set_placeholder("HydraulicTraining");
	fields->add_child(project_name_field);

	HBoxContainer *repo_actions = memnew(HBoxContainer);
	repo_actions->set_alignment(BoxContainer::ALIGNMENT_END);
	repo_actions->add_theme_constant_override("separation", Math::round(12 * EDSCALE));
	repo_view->add_child(repo_actions);

	repo_cancel_button = memnew(Button);
	repo_cancel_button->set_text(TTR("Cancel"));
	repo_cancel_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_repo_cancel_pressed));
	repo_actions->add_child(repo_cancel_button);

	repo_confirm_button = memnew(Button);
	repo_confirm_button->set_text(TTR("Get Project"));
	repo_confirm_button->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
	repo_confirm_button->add_theme_style_override("hover", CbtTheme::create_button_box(true, true, false));
	repo_confirm_button->add_theme_color_override("font_color", Color(1, 1, 1));
	repo_confirm_button->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_repo_confirm_pressed));
	repo_actions->add_child(repo_confirm_button);
}

void CbtProjectLauncher::_build_dialogs() {
	// New Project Dialog
	new_project_dialog = memnew(ConfirmationDialog);
	new_project_dialog->set_title(TTR("Create New CBT Project"));
	new_project_dialog->set_min_size(Vector2(Math::round(440 * EDSCALE), Math::round(180 * EDSCALE)));

	VBoxContainer *vb = memnew(VBoxContainer);
	vb->add_theme_constant_override("separation", Math::round(10 * EDSCALE));
	new_project_dialog->add_child(vb);

	Label *l_name = memnew(Label);
	l_name->set_text(TTR("Project Name:"));
	vb->add_child(l_name);

	new_project_name_edit = memnew(LineEdit);
	new_project_name_edit->set_placeholder("MyTrainingProject");
	vb->add_child(new_project_name_edit);

	Label *l_path = memnew(Label);
	l_path->set_text(TTR("Project Location:"));
	vb->add_child(l_path);

	HBoxContainer *path_hb = memnew(HBoxContainer);
	vb->add_child(path_hb);

	new_project_path_edit = memnew(LineEdit);
	new_project_path_edit->set_text(OS::get_singleton()->get_user_data_dir());
	new_project_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	path_hb->add_child(new_project_path_edit);

	Button *browse_btn = memnew(Button);
	browse_btn->set_text(TTR("Browse…"));
	browse_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_on_open_project_pressed));
	path_hb->add_child(browse_btn);

	new_project_dialog->connect(SceneStringName(confirmed), callable_mp(this, &CbtProjectLauncher::_on_new_project_confirmed));
	add_child(new_project_dialog);

	// File Dialog
	file_dialog = memnew(FileDialog);
	file_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_DIR);
	file_dialog->set_access(FileDialog::ACCESS_FILESYSTEM);
	file_dialog->connect("dir_selected", callable_mp(this, &CbtProjectLauncher::_on_dir_selected));
	add_child(file_dialog);
}

void CbtProjectLauncher::refresh() {
	_refresh_recent_projects();
}

void CbtProjectLauncher::show_home() {
	current_view = VIEW_HOME;
	home_view->set_visible(true);
	repo_view->set_visible(false);
	error_label->set_visible(false);
}

void CbtProjectLauncher::show_error(const String &p_message) {
	error_label->set_text(p_message);
	error_label->set_visible(!p_message.is_empty());
}

void CbtProjectLauncher::_refresh_recent_projects() {
	// Clear existing list
	while (recent_list->get_child_count() > 0) {
		Node *child = recent_list->get_child(0);
		recent_list->remove_child(child);
		memdelete(child);
	}

	Vector<String> paths_to_show;

	if (EditorPaths::get_singleton()) {
		String cfg_path = EditorPaths::get_singleton()->get_data_dir().path_join("projects.cfg");
		if (FileAccess::exists(cfg_path)) {
			ConfigFile cf;
			if (cf.load(cfg_path) == OK) {
				Vector<String> sections = cf.get_sections();
				for (int i = 0; i < sections.size(); i++) {
					String p = sections[i];
					if (DirAccess::dir_exists_absolute(p) && !paths_to_show.has(p)) {
						paths_to_show.push_back(p);
					}
				}
			}
		}
	}

	// Also check known default locations like /Users/onur/Documents/GitHub/Sample-CBT and cbt-authoring-poc
	Vector<String> default_candidates;
	default_candidates.push_back("/Users/onur/Documents/GitHub/Sample-CBT");
	default_candidates.push_back("/Users/onur/Documents/GitHub/cbt-authoring-poc");
	for (int i = 0; i < default_candidates.size(); i++) {
		if (DirAccess::dir_exists_absolute(default_candidates[i]) && !paths_to_show.has(default_candidates[i])) {
			paths_to_show.push_back(default_candidates[i]);
		}
	}

	if (paths_to_show.is_empty()) {
		recent_section->set_visible(false);
		empty_state->set_visible(true);
		return;
	}

	recent_section->set_visible(true);
	empty_state->set_visible(false);

	for (int i = 0; i < paths_to_show.size(); i++) {
		String path = paths_to_show[i];
		String project_name;
		bool is_cbt = _validate_cbt_project(path, project_name);

		PanelContainer *card = memnew(PanelContainer);
		card->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		card->add_theme_style_override("panel", CbtTheme::create_card_box(false, false));
		recent_list->add_child(card);

		HBoxContainer *row = memnew(HBoxContainer);
		row->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		row->add_theme_constant_override("separation", Math::round(12 * EDSCALE));
		card->add_child(row);

		// Left info: Name and Path
		VBoxContainer *left_vb = memnew(VBoxContainer);
		left_vb->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		row->add_child(left_vb);

		HBoxContainer *name_row = memnew(HBoxContainer);
		name_row->add_theme_constant_override("separation", Math::round(8 * EDSCALE));
		left_vb->add_child(name_row);

		Label *name_lbl = memnew(Label);
		name_lbl->set_text(project_name);
		name_lbl->add_theme_font_size_override("font_size", Math::round(14 * EDSCALE));
		name_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_PRIMARY);
		name_row->add_child(name_lbl);

		if (is_cbt) {
			Label *cbt_badge = memnew(Label);
			cbt_badge->set_text(TTR("CBT Workspace"));
			cbt_badge->add_theme_style_override("normal", CbtTheme::create_badge_box(Color(0.23f, 0.51f, 0.96f, 0.25f), CbtTheme::COLOR_PRIMARY));
			cbt_badge->add_theme_color_override("font_color", CbtTheme::COLOR_PRIMARY);
			cbt_badge->add_theme_font_size_override("font_size", Math::round(10 * EDSCALE));
			name_row->add_child(cbt_badge);
		}

		Label *path_lbl = memnew(Label);
		path_lbl->set_text(path);
		path_lbl->add_theme_color_override("font_color", CbtTheme::COLOR_TEXT_MUTED);
		path_lbl->add_theme_font_size_override("font_size", Math::round(11 * EDSCALE));
		left_vb->add_child(path_lbl);

		// Action buttons
		Button *open_btn = memnew(Button);
		open_btn->set_text(TTR("Open →"));
		open_btn->add_theme_style_override("normal", CbtTheme::create_button_box(true, false, false));
		open_btn->add_theme_style_override("hover", CbtTheme::create_button_box(true, true, false));
		open_btn->add_theme_color_override("font_color", Color(1, 1, 1));
		open_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_open_project).bind(path));
		row->add_child(open_btn);

		Button *remove_btn = memnew(Button);
		remove_btn->set_text("✕");
		remove_btn->set_flat(true);
		remove_btn->set_tooltip_text(TTR("Remove from recent projects list"));
		remove_btn->connect(SceneStringName(pressed), callable_mp(this, &CbtProjectLauncher::_remove_recent_project).bind(path));
		row->add_child(remove_btn);
	}
}

bool CbtProjectLauncher::_validate_cbt_project(const String &p_path, String &r_project_name) {
	String pj = p_path.path_join("project.json");
	if (FileAccess::exists(pj)) {
		Error err;
		String json_str = FileAccess::get_file_as_string(pj, &err);
		if (err == OK) {
			Ref<JSON> json;
			json.instantiate();
			if (json->parse(json_str) == OK) {
				Dictionary d = json->get_data();
				if (d.has("name")) {
					r_project_name = d["name"];
					return true;
				}
			}
		}
	}

	String pg = p_path.path_join("project.godot");
	if (FileAccess::exists(pg)) {
		Ref<ConfigFile> cf;
		cf.instantiate();
		if (cf->load(pg) == OK) {
			r_project_name = cf->get_value("application", "config/name", p_path.get_file());
			return false;
		}
	}

	r_project_name = p_path.get_file();
	return false;
}

void CbtProjectLauncher::_open_project(const String &p_path) {
	if (EditorPaths::get_singleton()) {
		String cfg_path = EditorPaths::get_singleton()->get_data_dir().path_join("projects.cfg");
		ConfigFile cf;
		cf.load(cfg_path);
		cf.set_value(p_path, "favorite", false);
		cf.save(cfg_path);
	}

	if (is_standalone_mode) {
		List<String> args;
		for (const String &a : Main::get_forwardable_cli_arguments(Main::CLI_SCOPE_TOOL)) {
			args.push_back(a);
		}
		args.push_back("--path");
		args.push_back(p_path);
		args.push_back("--editor");

		Error err = OS::get_singleton()->create_instance(args);
		if (err != OK) {
			show_error(vformat(TTR("Failed to launch editor for project at: %s"), p_path));
			return;
		}
		get_tree()->quit();
	} else {
		emit_signal("project_selected", p_path);
	}
}

void CbtProjectLauncher::_remove_recent_project(const String &p_path) {
	if (EditorPaths::get_singleton()) {
		String cfg_path = EditorPaths::get_singleton()->get_data_dir().path_join("projects.cfg");
		ConfigFile cf;
		if (cf.load(cfg_path) == OK) {
			if (cf.has_section(p_path)) {
				cf.erase_section(p_path);
				cf.save(cfg_path);
			}
		}
	}
	refresh();
}

void CbtProjectLauncher::_on_new_project_pressed() {
	new_project_name_edit->set_text("NewCbtProject");
	new_project_path_edit->set_text("/Users/onur/Documents/GitHub");
	new_project_dialog->popup_centered();
}

void CbtProjectLauncher::_on_new_project_confirmed() {
	String name = new_project_name_edit->get_text().strip_edges();
	String parent = new_project_path_edit->get_text().strip_edges();
	if (name.is_empty() || parent.is_empty()) {
		show_error(TTR("Please provide valid project name and parent path."));
		return;
	}

	String target_dir = parent.path_join(name);
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (!da->dir_exists(target_dir)) {
		da->make_dir_recursive(target_dir);
	}

	// Create project.json
	String pj = target_dir.path_join("project.json");
	if (!FileAccess::exists(pj)) {
		Ref<FileAccess> fa = FileAccess::open(pj, FileAccess::WRITE);
		if (fa.is_valid()) {
			fa->store_string(vformat(
				"{\n"
				"  \"projectId\": \"project.%s\",\n"
				"  \"name\": \"%s\",\n"
				"  \"description\": \"CBT Training Project\",\n"
				"  \"schemaVersion\": 4,\n"
				"  \"contentVersion\": \"1.0.0\"\n"
				"}\n",
				name.to_lower(), name));
		}
	}

	// Create project.godot
	String pg = target_dir.path_join("project.godot");
	if (!FileAccess::exists(pg)) {
		Ref<FileAccess> fa = FileAccess::open(pg, FileAccess::WRITE);
		if (fa.is_valid()) {
			fa->store_string(vformat(
				"; Engine configuration file for CBT Content Studio\n"
				"config_version=5\n\n"
				"[application]\n\n"
				"config/name=\"%s\"\n"
				"config/features=PackedStringArray(\"4.6\", \"Forward Plus\")\n\n"
				"[rendering]\n\n"
				"renderer/rendering_method=\"forward_plus\"\n",
				name));
		}
	}

	_open_project(target_dir);
}

void CbtProjectLauncher::_on_open_project_pressed() {
	file_dialog->popup_centered_ratio(0.7);
}

void CbtProjectLauncher::_on_dir_selected(const String &p_path) {
	_open_project(p_path);
}

void CbtProjectLauncher::_on_quick_add_pressed() {
	String url = quick_repo_url_input->get_text().strip_edges();
	if (url.is_empty()) {
		show_error(TTR("Please enter a valid repository URL."));
		return;
	}

	String project_name = _infer_project_name(url);
	String parent_dir = "/Users/onur/Documents/GitHub";
	String target_dir = parent_dir.path_join(project_name);

	status_label->set_text(vformat(TTR("Cloning repository into %s…"), target_dir));
	status_label->set_visible(true);

	List<String> git_args;
	git_args.push_back("clone");
	git_args.push_back(url);
	git_args.push_back(target_dir);

	int exit_code = 0;
	Error err = OS::get_singleton()->execute("git", git_args, nullptr, &exit_code);
	if (err == OK && exit_code == 0) {
		status_label->set_text(TTR("Clone successful! Opening project…"));
		_open_project(target_dir);
	} else {
		show_error(vformat(TTR("Git clone failed (exit code %d). Check repository URL and permissions."), exit_code));
	}
}

void CbtProjectLauncher::_on_advanced_pressed() {
	repo_url_field->set_text(quick_repo_url_input->get_text());
	project_name_field->set_text(_infer_project_name(quick_repo_url_input->get_text()));
	current_view = VIEW_REPOSITORY;
	home_view->set_visible(false);
	repo_view->set_visible(true);
}

void CbtProjectLauncher::_on_repo_confirm_pressed() {
	String url = repo_url_field->get_text().strip_edges();
	String parent_dir = dest_parent_field->get_text().strip_edges();
	String project_name = project_name_field->get_text().strip_edges();

	if (url.is_empty() || parent_dir.is_empty() || project_name.is_empty()) {
		show_error(TTR("All repository fields are required."));
		return;
	}

	String target_dir = parent_dir.path_join(project_name);
	List<String> git_args;
	git_args.push_back("clone");
	git_args.push_back(url);
	git_args.push_back(target_dir);

	int exit_code = 0;
	Error err = OS::get_singleton()->execute("git", git_args, nullptr, &exit_code);
	if (err == OK && exit_code == 0) {
		_open_project(target_dir);
	} else {
		show_error(vformat(TTR("Git clone failed with code %d."), exit_code));
	}
}

void CbtProjectLauncher::_on_repo_cancel_pressed() {
	show_home();
}

void CbtProjectLauncher::_on_close_pressed() {
	if (is_standalone_mode) {
		get_tree()->quit();
	} else {
		emit_signal("close_requested");
	}
}

String CbtProjectLauncher::_infer_project_name(const String &p_url) {
	String clean = p_url.strip_edges().trim_suffix("/").trim_suffix("\\");
	if (clean.ends_with(".git")) {
		clean = clean.substr(0, clean.length() - 4);
	}
	int slash = clean.rfind("/");
	if (slash >= 0) {
		clean = clean.substr(slash + 1);
	}
	return clean.is_empty() ? "ClonedCbtProject" : clean;
}
