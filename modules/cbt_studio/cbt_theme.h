/**************************************************************************/
/*  cbt_theme.h - CBT Content Studio Visual Design System & Theme Tokens  */
/**************************************************************************/

#pragma once

#include "core/math/color.h"
#include "scene/gui/control.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/theme.h"

class CbtTheme {
public:
	// Semantic Surface & Background Colors
	static const Color COLOR_BG_DARKEST;        // #101114
	static const Color COLOR_BG_PANEL;          // #16171B
	static const Color COLOR_BG_PANEL_SUBTLE;   // #121316
	static const Color COLOR_SURFACE;           // #1E2025
	static const Color COLOR_SURFACE_HOVER;     // #26282F
	static const Color COLOR_SURFACE_ACTIVE;    // #1E2638
	static const Color COLOR_NAV_SELECTED;      // #1B2334

	// Borders & Dividers
	static const Color COLOR_BORDER_SUBTLE;     // #1E2026
	static const Color COLOR_BORDER;            // #26282F
	static const Color COLOR_BORDER_STRONG;     // #363943
	static const Color COLOR_BORDER_HOVER;      // #383B45

	// Brand & State Accents
	static const Color COLOR_PRIMARY;           // #3B82F6 (Electric Blue)
	static const Color COLOR_PRIMARY_HOVER;     // #2563EB
	static const Color COLOR_PRIMARY_ACTIVE;    // #1D4ED8
	static const Color COLOR_SUCCESS;           // #10B981
	static const Color COLOR_WARNING;           // #F59E0B
	static const Color COLOR_DANGER;            // #EF4444

	// Typography Contrast
	static const Color COLOR_TEXT_PRIMARY;      // #F3F4F6
	static const Color COLOR_TEXT_SECONDARY;    // #CBD5E1
	static const Color COLOR_TEXT_MUTED;        // #94A3B8

	// Helper StyleBox Generators
	static Ref<StyleBoxFlat> create_flat_box(
			const Color &p_bg_color,
			const Color &p_border_color = Color(0, 0, 0, 0),
			int p_border_width = 0,
			int p_corner_radius = 4,
			int p_pad_h = 8,
			int p_pad_v = 4);

	static Ref<StyleBoxFlat> create_tab_box(bool p_selected, bool p_hover);
	static Ref<StyleBoxFlat> create_domain_tab_box(bool p_selected, bool p_hover);
	static Ref<StyleBoxFlat> create_button_box(bool p_primary, bool p_hover, bool p_pressed);
	static Ref<StyleBoxFlat> create_panel_box();
	static Ref<StyleBoxFlat> create_card_box(bool p_selected = false, bool p_hover = false);
	static Ref<StyleBoxFlat> create_badge_box(const Color &p_bg, const Color &p_text);

	static void apply_to_control(Control *p_control);
};
