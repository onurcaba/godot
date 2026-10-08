/**************************************************************************/
/*  cbt_theme.cpp - CBT Content Studio Visual Design System & Tokens      */
/**************************************************************************/

#include "cbt_theme.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/control.h"

// Define Color Constants matching ContentStudioTheme.uss
const Color CbtTheme::COLOR_BG_DARKEST      = Color::hex(0x101114FF);
const Color CbtTheme::COLOR_BG_PANEL        = Color::hex(0x16171BFF);
const Color CbtTheme::COLOR_BG_PANEL_SUBTLE = Color::hex(0x121316FF);
const Color CbtTheme::COLOR_SURFACE         = Color::hex(0x1E2025FF);
const Color CbtTheme::COLOR_SURFACE_HOVER   = Color::hex(0x26282FFF);
const Color CbtTheme::COLOR_SURFACE_ACTIVE  = Color::hex(0x1E2638FF);
const Color CbtTheme::COLOR_NAV_SELECTED    = Color::hex(0x1B2334FF);

const Color CbtTheme::COLOR_BORDER_SUBTLE   = Color::hex(0x1E2026FF);
const Color CbtTheme::COLOR_BORDER          = Color::hex(0x26282FFF);
const Color CbtTheme::COLOR_BORDER_STRONG   = Color::hex(0x363943FF);
const Color CbtTheme::COLOR_BORDER_HOVER    = Color::hex(0x383B45FF);

const Color CbtTheme::COLOR_PRIMARY         = Color::hex(0x3B82F6FF); // Electric Blue
const Color CbtTheme::COLOR_PRIMARY_HOVER   = Color::hex(0x2563EBFF);
const Color CbtTheme::COLOR_PRIMARY_ACTIVE  = Color::hex(0x1D4ED8FF);
const Color CbtTheme::COLOR_SUCCESS         = Color::hex(0x10B981FF);
const Color CbtTheme::COLOR_WARNING         = Color::hex(0xF59E0BFF);
const Color CbtTheme::COLOR_DANGER          = Color::hex(0xEF4444FF);

const Color CbtTheme::COLOR_TEXT_PRIMARY    = Color::hex(0xF3F4F6FF);
const Color CbtTheme::COLOR_TEXT_SECONDARY  = Color::hex(0xCBD5E1FF);
const Color CbtTheme::COLOR_TEXT_MUTED      = Color::hex(0x94A3B8FF);

Ref<StyleBoxFlat> CbtTheme::create_flat_box(
		const Color &p_bg_color,
		const Color &p_border_color,
		int p_border_width,
		int p_corner_radius,
		int p_pad_h,
		int p_pad_v) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	sb->set_bg_color(p_bg_color);

	float scale = EDSCALE;
	if (p_border_width > 0) {
		sb->set_border_width_all(Math::round(p_border_width * scale));
		sb->set_border_color(p_border_color);
	}
	if (p_corner_radius > 0) {
		sb->set_corner_radius_all(Math::round(p_corner_radius * scale));
	}
	sb->set_content_margin(SIDE_LEFT, Math::round(p_pad_h * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(p_pad_h * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(p_pad_v * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(p_pad_v * scale));
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_domain_tab_box(bool p_selected, bool p_hover) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	float scale = EDSCALE;

	if (p_selected) {
		sb->set_bg_color(COLOR_SURFACE_ACTIVE);
		sb->set_border_width(SIDE_BOTTOM, Math::round(2 * scale));
		sb->set_border_color(COLOR_PRIMARY);
	} else if (p_hover) {
		sb->set_bg_color(COLOR_SURFACE_HOVER);
	} else {
		sb->set_bg_color(Color(0, 0, 0, 0)); // transparent
	}

	sb->set_corner_radius(CORNER_TOP_LEFT, Math::round(4 * scale));
	sb->set_corner_radius(CORNER_TOP_RIGHT, Math::round(4 * scale));
	sb->set_content_margin(SIDE_LEFT, Math::round(14 * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(14 * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(6 * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(6 * scale));
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_tab_box(bool p_selected, bool p_hover) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	float scale = EDSCALE;

	if (p_selected) {
		sb->set_bg_color(COLOR_SURFACE_ACTIVE);
		sb->set_border_width(SIDE_BOTTOM, Math::round(2 * scale));
		sb->set_border_color(COLOR_PRIMARY);
	} else if (p_hover) {
		sb->set_bg_color(COLOR_SURFACE_HOVER);
	} else {
		sb->set_bg_color(Color(0, 0, 0, 0));
	}

	sb->set_corner_radius_all(Math::round(4 * scale));
	sb->set_content_margin(SIDE_LEFT, Math::round(10 * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(10 * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(5 * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(5 * scale));
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_button_box(bool p_primary, bool p_hover, bool p_pressed) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	float scale = EDSCALE;

	if (p_primary) {
		if (p_pressed) {
			sb->set_bg_color(COLOR_PRIMARY_ACTIVE);
		} else if (p_hover) {
			sb->set_bg_color(COLOR_PRIMARY_HOVER);
		} else {
			sb->set_bg_color(COLOR_PRIMARY);
		}
		sb->set_border_width_all(Math::round(1 * scale));
		sb->set_border_color(COLOR_PRIMARY_HOVER);
	} else {
		// Secondary / Neutral button
		if (p_pressed) {
			sb->set_bg_color(COLOR_SURFACE_ACTIVE);
		} else if (p_hover) {
			sb->set_bg_color(COLOR_SURFACE_HOVER);
		} else {
			sb->set_bg_color(COLOR_SURFACE);
		}
		sb->set_border_width_all(Math::round(1 * scale));
		sb->set_border_color(p_hover ? COLOR_BORDER_HOVER : COLOR_BORDER);
	}

	sb->set_corner_radius_all(Math::round(5 * scale));
	sb->set_content_margin(SIDE_LEFT, Math::round(12 * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(12 * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(6 * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(6 * scale));
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_panel_box() {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	sb->set_bg_color(COLOR_BG_PANEL);
	sb->set_border_width(SIDE_RIGHT, Math::round(1 * EDSCALE));
	sb->set_border_color(COLOR_BORDER_SUBTLE);
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_card_box(bool p_selected, bool p_hover) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	float scale = EDSCALE;

	if (p_selected) {
		sb->set_bg_color(COLOR_SURFACE_ACTIVE);
		sb->set_border_width_all(Math::round(1 * scale));
		sb->set_border_color(COLOR_PRIMARY);
	} else if (p_hover) {
		sb->set_bg_color(COLOR_SURFACE_HOVER);
		sb->set_border_width_all(Math::round(1 * scale));
		sb->set_border_color(COLOR_BORDER_HOVER);
	} else {
		sb->set_bg_color(COLOR_SURFACE);
		sb->set_border_width_all(Math::round(1 * scale));
		sb->set_border_color(COLOR_BORDER);
	}

	sb->set_corner_radius_all(Math::round(6 * scale));
	sb->set_content_margin(SIDE_LEFT, Math::round(12 * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(12 * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(10 * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(10 * scale));
	return sb;
}

Ref<StyleBoxFlat> CbtTheme::create_badge_box(const Color &p_bg, const Color &p_text) {
	Ref<StyleBoxFlat> sb;
	sb.instantiate();
	float scale = EDSCALE;
	sb->set_bg_color(p_bg);
	sb->set_corner_radius_all(Math::round(3 * scale));
	sb->set_content_margin(SIDE_LEFT, Math::round(6 * scale));
	sb->set_content_margin(SIDE_RIGHT, Math::round(6 * scale));
	sb->set_content_margin(SIDE_TOP, Math::round(2 * scale));
	sb->set_content_margin(SIDE_BOTTOM, Math::round(2 * scale));
	return sb;
}
