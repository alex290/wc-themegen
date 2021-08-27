/*
 * wcellarthemegenerator.c
 *
 * Copyright 2018 meliafaro <meliafaro at inbox dot ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include "wcellarthemegenerator.h"

#define WCELLAR_COLOR_STRIP_TO_CHAR(x) ((x) > 255 ? (x) = 255 : (x))
#define WCELLAR_COLOR_MEDIAN_VALUE 127

typedef struct
{
	guchar r;
	guchar g;
	guchar b;
} WColor;

struct _WCellarThemeGenerator
{
	GObject parent;
	GTree* color_map;
	WColor* main_color;
	WColor* highlight_color;
	WColor* active_color;
	WColor* inactive_color;
	WColor* text_color;
	gdouble contrast;
};

G_DEFINE_TYPE (WCellarThemeGenerator, wcellar_themegenerator, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_MAIN_COLOR,
	PROP_HIGHLIGHT_COLOR,
	PROP_ACTIVE_COLOR,
	PROP_INACTIVE_COLOR,
	PROP_TEXT_COLOR,
	PROP_CONTRAST,
	N_PROPERTIES
};

static GParamSpec* wcellar_themegenerator_properties[N_PROPERTIES] = {NULL, };

static gchar*
wcolor_to_string(const WColor* wc)
{
    GString* str;

    if (wc)
    {
        str = g_string_new("#");
        g_string_append_printf(str, "%02x%02x%02x", wc->r, wc->g, wc->b);
        return g_string_free(str, FALSE);
    }
	return NULL;
}

static gboolean
string_to_wcolor(const gchar* string, WColor* wcolor)
{
    uint color;

    g_assert(wcolor);

    if (!g_regex_match_simple("^#[A-Fa-f0-9]{6}$", string, 0, 0))
        return FALSE;
    if (sscanf(string + 1, "%x", &color) != 1)
        return FALSE;

    wcolor->r = (color & 0xff0000) >> 16;
    wcolor->g = (color & 0x00ff00) >> 8;
    wcolor->b = (color & 0x0000ff);

	return TRUE;
}

static WColor*
rgba_to_wcolor(const GdkRGBA* rgba)
{
	g_assert(rgba);

	WColor* color = g_new(WColor, 1);
	color->r = rgba->red * 255;
	color->g = rgba->green * 255;
	color->b = rgba->blue * 255;

	return color;
}

static gboolean
color_is_dark(const WColor* rgb, guint limit)
{
	return (rgb->r + rgb->g + rgb->b) / 3 < limit;
}

static WColor*
copy_wcolor(const WColor* orig)
{
	WColor* new = g_new(WColor, 1);

	new->r = orig->r;
	new->g = orig->g;
	new->b = orig->b;

	return new;
}

static gchar*
color_to_reg_string(const WColor* rgb)
{
	GString* str = g_string_new(NULL);

	g_string_append_printf(str, "%u %u %u", rgb->r, rgb->g, rgb->b);

	return g_string_free(str, FALSE);
}

static void
set_brighter(WColor* rgb, gdouble rate)
{
	rgb->r = rgb->r + (255 - rgb->r) * rate;
	rgb->r = WCELLAR_COLOR_STRIP_TO_CHAR (rgb->r);
	rgb->g = rgb->g + (255 - rgb->g) * rate;
	rgb->g = WCELLAR_COLOR_STRIP_TO_CHAR (rgb->g);
	rgb->b = rgb->b + (255 - rgb->b) * rate;
	rgb->b = WCELLAR_COLOR_STRIP_TO_CHAR (rgb->b);
}

static void
set_darker(WColor* rgb, gdouble rate)
{
	rgb->r = rgb->r - rgb->r * rate;
	rgb->g = rgb->g - rgb->g * rate;
	rgb->b = rgb->b - rgb->b * rate;
}

static void
set_text_color(WColor* main_color, WColor* text_color)
{
	if (color_is_dark(main_color, WCELLAR_COLOR_MEDIAN_VALUE))
	{
		text_color->r = 235;
		text_color->g = 235;
		text_color->b = 235;
	}
	else
	{
		text_color->r = 16;
		text_color->g = 16;
		text_color->b = 16;
	}
}

static gboolean
traverse_color_map(gchar* key, gchar* value, GString* str)
{
	g_string_append_printf(str, "\"%s\"=\"%s\"\n", key, value);
	return FALSE;
}

static GTree*
fill_color_map()
{
	GTree* map = g_tree_new_full((GCompareDataFunc) g_strcmp0,
								 NULL, NULL, g_free);
	g_tree_insert(map, "ButtonFace", NULL);
	g_tree_insert(map, "Menu", NULL);
	g_tree_insert(map, "Scrollbar", NULL);
	g_tree_insert(map, "Window", NULL);
	g_tree_insert(map, "Background", NULL);
	g_tree_insert(map, "Hilight", NULL);
	g_tree_insert(map, "MenuHilight", NULL);
	g_tree_insert(map, "ButtonText", NULL);
	g_tree_insert(map, "WindowText", NULL);
	g_tree_insert(map, "HilightText", NULL);
	g_tree_insert(map, "MenuText", NULL);
	g_tree_insert(map, "GrayText", NULL);
	g_tree_insert(map, "ActiveTitle", NULL);
	g_tree_insert(map, "GradientActiveTitle", NULL);
	g_tree_insert(map, "TitleText", NULL);
	g_tree_insert(map, "InactiveTitle", NULL);
	g_tree_insert(map, "GradientInactiveTitle", NULL);
	g_tree_insert(map, "InactiveTitleText", NULL);
	g_tree_insert(map, "ButtonHilight", NULL);
	g_tree_insert(map, "ButtonLight", NULL);
	g_tree_insert(map, "ButtonShadow", NULL);
	g_tree_insert(map, "ButtonDkShadow", NULL);
	g_tree_insert(map, "InfoWindow", NULL);
	g_tree_insert(map, "InfoText", NULL);
	g_tree_insert(map, "ActiveBorder", NULL);
	g_tree_insert(map, "InactiveBorder", NULL);
	g_tree_insert(map, "WindowFrame", NULL);
	g_tree_insert(map, "AppWorkSpace", NULL);
	g_tree_insert(map, "ButtonAlternateFace", NULL);
	g_tree_insert(map, "HotTrackingColor", NULL);
	g_tree_insert(map, "MenuBar", NULL);
	return map;
}

static void
wcellar_themegenerator_get_property(GObject* obj,
									guint property_id,
									GValue* value,
									GParamSpec* pspec)
{
	WCellarThemeGenerator* self = WCELLAR_THEMEGENERATOR (obj);
	gchar* color;

	switch (property_id)
	{
		case PROP_MAIN_COLOR:
			color = wcolor_to_string(self->main_color);
			break;
		case PROP_HIGHLIGHT_COLOR:
			color = wcolor_to_string(self->highlight_color);
			break;
		case PROP_ACTIVE_COLOR:
			color = wcolor_to_string(self->active_color);
			break;
		case PROP_INACTIVE_COLOR:
			color = wcolor_to_string(self->inactive_color);
			break;
		case PROP_TEXT_COLOR:
			color = wcolor_to_string(self->text_color);
			break;
		case PROP_CONTRAST:
			g_value_set_double(value, self->contrast);
			return;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
			return;
	}

	g_assert(color);

	g_value_set_string(value, color);
	g_free(color);
}

static void
wcellar_themegenerator_set_property(GObject* obj,
									guint property_id,
									const GValue* value,
									GParamSpec* pspec)
{
	WCellarThemeGenerator* self = WCELLAR_THEMEGENERATOR (obj);
	gboolean result;

	switch (property_id)
	{
		case PROP_MAIN_COLOR:
			result = string_to_wcolor(g_value_get_string(value), self->main_color);
			break;
		case PROP_HIGHLIGHT_COLOR:
			result = string_to_wcolor(g_value_get_string(value), self->highlight_color);
			break;
		case PROP_ACTIVE_COLOR:
			result = string_to_wcolor(g_value_get_string(value), self->active_color);
			break;
		case PROP_INACTIVE_COLOR:
			result = string_to_wcolor(g_value_get_string(value), self->inactive_color);
			break;
		case PROP_TEXT_COLOR:
			result = string_to_wcolor(g_value_get_string(value), self->text_color);
			break;
		case PROP_CONTRAST:
			self->contrast = g_value_get_double(value);
			return;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, property_id, pspec);
			return;
	}
	if (!result)
		g_error("Not valid color value: '%s'\n", g_value_get_string(value));
}

static void
wcellar_themegenerator_finalize(GObject* obj)
{
	WCellarThemeGenerator* self = WCELLAR_THEMEGENERATOR (obj);

	g_tree_destroy(self->color_map);
	g_free(self->main_color);
	g_free(self->highlight_color);
	g_free(self->active_color);
	g_free(self->inactive_color);
	g_free(self->text_color);

	G_OBJECT_CLASS (wcellar_themegenerator_parent_class)->finalize(obj);
}

static void
wcellar_themegenerator_class_init (WCellarThemeGeneratorClass *klass)
{
	GObjectClass* obj_class = G_OBJECT_CLASS (klass);

	obj_class->finalize = wcellar_themegenerator_finalize;
	obj_class->get_property = wcellar_themegenerator_get_property;
	obj_class->set_property = wcellar_themegenerator_set_property;

	wcellar_themegenerator_properties[PROP_MAIN_COLOR] =
				g_param_spec_string("main-color", NULL, NULL, NULL, G_PARAM_READWRITE);
	wcellar_themegenerator_properties[PROP_HIGHLIGHT_COLOR] =
				g_param_spec_string("highlight-color", NULL, NULL, NULL, G_PARAM_READWRITE);
	wcellar_themegenerator_properties[PROP_ACTIVE_COLOR] =
				g_param_spec_string("active-color", NULL, NULL, NULL, G_PARAM_READWRITE);
	wcellar_themegenerator_properties[PROP_INACTIVE_COLOR] =
				g_param_spec_string("inactive-color", NULL, NULL, NULL, G_PARAM_READWRITE);
	wcellar_themegenerator_properties[PROP_TEXT_COLOR] =
				g_param_spec_string("text-color", NULL, NULL, NULL, G_PARAM_READWRITE);
	wcellar_themegenerator_properties[PROP_CONTRAST] =
				g_param_spec_double("contrast", NULL, NULL, 0.0, 2.0, 1.0,
									G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
	g_object_class_install_properties(obj_class, N_PROPERTIES,
									  wcellar_themegenerator_properties);
}

static void
wcellar_themegenerator_init (WCellarThemeGenerator *self)
{
	self->color_map = fill_color_map();

	self->main_color = g_new(WColor, 1);
	self->highlight_color = g_new(WColor, 1);
	self->active_color = g_new(WColor, 1);
	self->inactive_color = g_new(WColor, 1);
	self->text_color = NULL;
}


WCellarThemeGenerator*
wcellar_themegenerator_new ()
{
	return g_object_new (WCELLAR_TYPE_THEMEGENERATOR, NULL);
}

gboolean
wcellar_themegenerator_set_gtk_colors(WCellarThemeGenerator* self)
{
	g_assert(self);

	GtkWidget* win;
	GdkRGBA rgba;

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_ref_sink(win);
	GtkStyleContext* context = gtk_widget_get_style_context(win);

	if (gtk_style_context_lookup_color(context, "theme_bg_color", &rgba))
	{
		g_free(self->main_color);
		self->main_color = rgba_to_wcolor(&rgba);
	}
	else
		goto on_failure;
	if (gtk_style_context_lookup_color(context, "theme_selected_bg_color", &rgba))
	{
		g_free(self->highlight_color);
		self->highlight_color = rgba_to_wcolor(&rgba);
	}
	else
		goto on_failure;
	if (gtk_style_context_lookup_color(context, "theme_selected_bg_color", &rgba))
	{
		g_free(self->active_color);
		self->active_color = rgba_to_wcolor(&rgba);
	}
	else
		goto on_failure;
	if (gtk_style_context_lookup_color(context, "theme_bg_color", &rgba))
	{
		g_free(self->inactive_color);
		self->inactive_color = rgba_to_wcolor(&rgba);
	}
	else
		goto on_failure;

	if (!self->text_color)
	{
		self->text_color = g_new(WColor, 1);
		set_text_color(self->main_color, self->text_color);
	}

	g_object_unref(G_OBJECT (context));
	gtk_widget_destroy(win);

	return TRUE;

on_failure:
	g_object_unref(G_OBJECT (context));
	gtk_widget_destroy(win);
	g_error("Failed to get gtk colors!\n");
}

static void
build_color_map(WCellarThemeGenerator* self)
{
	GTree* tree = self->color_map;
	WColor* color;
	//~ gchar* temp_str;
	gboolean dark = color_is_dark(self->main_color, WCELLAR_COLOR_MEDIAN_VALUE);
	gdouble contr = self->contrast;

	/* Main elements */
	g_tree_replace(tree, "ButtonFace", color_to_reg_string(self->main_color));
	g_tree_replace(tree, "Menu", color_to_reg_string(self->main_color));
	if (dark)
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .4 * contr);
		g_tree_replace(tree, "Scrollbar", color_to_reg_string(color));
		g_free(color);
	}
	else
	{
		color = copy_wcolor(self->main_color);
		set_darker(color, .1 * contr);
		g_tree_replace(tree, "Scrollbar", color_to_reg_string(color));
		g_free(color);
	}

	if (dark)
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .3 * contr);
		g_tree_replace(tree, "Window", color_to_reg_string(color));
		g_free(color);
	}
	else
		g_tree_replace(tree, "Window", g_strdup("247 247 247"));

	color = copy_wcolor(self->highlight_color);
	set_brighter(color, .6 * contr);
	g_tree_replace(tree, "Background", color_to_reg_string(color));
	g_free(color);

	g_tree_replace(tree, "Hilight", color_to_reg_string(self->highlight_color));
	g_tree_replace(tree, "MenuHilight", color_to_reg_string(self->highlight_color));

	/* text color */
	g_tree_replace(tree, "ButtonText", color_to_reg_string(self->text_color));
	g_tree_replace(tree, "WindowText", color_to_reg_string(self->text_color));
	g_tree_replace(tree, "MenuText", color_to_reg_string(self->text_color));
	if (color_is_dark(self->highlight_color, 150))
		g_tree_replace(tree, "HilightText", g_strdup("247 247 247"));
	else
		g_tree_replace(tree, "HilightText", g_strdup("16 16 16"));
	g_tree_replace(tree, "GrayText", g_strdup("127 127 127"));

	/* window title */
	g_tree_replace(tree, "ActiveTitle", color_to_reg_string(self->active_color));
	g_tree_replace(tree, "GradientActiveTitle", color_to_reg_string(self->active_color));
	g_tree_replace(tree, "InactiveTitle", color_to_reg_string(self->inactive_color));
	g_tree_replace(tree, "GradientInactiveTitle", color_to_reg_string(self->inactive_color));

	if (color_is_dark(self->active_color, WCELLAR_COLOR_MEDIAN_VALUE))
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .9 * contr);
		g_tree_replace(tree, "TitleText", color_to_reg_string(color));
		g_free(color);
	}
	else
	{
		color = copy_wcolor(self->main_color);
		set_darker(color, .9 * contr);
		g_tree_replace(tree, "TitleText", color_to_reg_string(color));
		g_free(color);
	}
	if (color_is_dark(self->inactive_color, WCELLAR_COLOR_MEDIAN_VALUE))
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .4 * contr);
		g_tree_replace(tree, "InactiveTitleText", color_to_reg_string(color));
		g_free(color);
	}
	else
	{
		color = copy_wcolor(self->main_color);
		set_darker(color, .4 * contr);
		g_tree_replace(tree, "InactiveTitleText", color_to_reg_string(color));
		g_free(color);
	}

	/* borders */
	if (dark)
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .1 * contr);
		g_tree_replace(tree, "ButtonHilight", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_brighter(color, .2 * contr);
		g_tree_replace(tree, "ButtonLight", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_darker(color, .3 * contr);
		g_tree_replace(tree, "ButtonShadow", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_darker(color, .6 * contr);
		g_tree_replace(tree, "ButtonDkShadow", color_to_reg_string(color));
		g_free(color);
	}
	else
	{
		color = copy_wcolor(self->main_color);
		set_brighter(color, .5 * contr);
		g_tree_replace(tree, "ButtonHilight", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_brighter(color, .7 * contr);
		g_tree_replace(tree, "ButtonLight", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_darker(color, .3 * contr);
		g_tree_replace(tree, "ButtonShadow", color_to_reg_string(color));
		g_free(color);

		color = copy_wcolor(self->main_color);
		set_darker(color, .5 * contr);
		g_tree_replace(tree, "ButtonDkShadow", color_to_reg_string(color));
		g_free(color);
	}
	g_tree_replace(tree, "ActiveBorder", color_to_reg_string(self->main_color));
	g_tree_replace(tree, "InactiveBorder", color_to_reg_string(self->main_color));
	if (dark)
		g_tree_replace(tree, "WindowFrame", g_strdup(g_tree_lookup(tree, "ButtonHilight")));
	else
		g_tree_replace(tree, "WindowFrame", g_strdup(g_tree_lookup(tree, "ButtonDkShadow")));

	/* popup text */
	g_tree_replace(tree, "InfoWindow", g_strdup("0 0 0"));
	g_tree_replace(tree, "InfoText", g_strdup("247 247 247"));

	/* other */
	g_tree_replace(tree, "AppWorkSpace", g_strdup(g_tree_lookup(tree, "Window")));
	g_tree_replace(tree, "ButtonAlternateFace", g_strdup("0 255 0"));
	g_tree_replace(tree, "HotTrackingColor", g_strdup("0 0 255"));
	g_tree_replace(tree, "MenuBar", g_strdup(g_tree_lookup(tree, "ButtonFace")));
}

gboolean wcellar_themegenerator_create_reg_file(WCellarThemeGenerator* self, gchar* path)
{
	GString* content;

	build_color_map(self);

	content = g_string_new(NULL);
	g_string_append(content,
				"REGEDIT4\n\n[HKEY_USERS\\S-1-5-21-0-0-0-1000\\Control Panel\\Colors]\n\n");
	g_tree_foreach(self->color_map, (GTraverseFunc) traverse_color_map, content);

	g_file_set_contents(path, content->str, content->len, NULL);
	g_string_free(content, TRUE);
	return TRUE;
}
