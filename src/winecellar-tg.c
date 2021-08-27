/*
 * winecellar-tg.c
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

#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "wcellarcommon.h"
#include "wcellarthemegenerator.h"
#include "wcellarlocale.h"

enum
{
	COLOR_MAIN,
	COLOR_HIGHLIGHTED,
	COLOR_ACTIVE,
	COLOR_INACTIVE,
	COLOR_TEXT,
	N_COLORS
};

static gchar* wcellar_prefix = NULL;
static gboolean wcellar_not_run_winecfg = FALSE;
static gchar* wcellar_loader_dir = NULL;
static gboolean wcellar_set_default = FALSE;
static gchar* wcellar_colors[N_COLORS] = { NULL, };
static gdouble wcellar_contrast;

static GOptionEntry entries[] =
{
	{ "prefix", 'p', 0, G_OPTION_ARG_FILENAME, &wcellar_prefix, "WINEPREFIX environment.", NULL },
	{ "not-run-winecfg", 'w', 0, G_OPTION_ARG_NONE, &wcellar_not_run_winecfg, "Not run winecfg after.", NULL},
	{ "loader-dir", 'l', 0, G_OPTION_ARG_FILENAME, &wcellar_loader_dir, "Custom loader dir.", NULL },
	{ "set-default", 'd', 0, G_OPTION_ARG_NONE, &wcellar_set_default, "Set theme default.", NULL },
	{ "main-color", 'm', 0, G_OPTION_ARG_STRING, &(wcellar_colors[COLOR_MAIN]), "Main color.", NULL },
	{ "highlighted-color", 'h', 0, G_OPTION_ARG_STRING, &wcellar_colors[COLOR_HIGHLIGHTED], "Highlighted color.", NULL },
	{ "active-color", 'a', 0, G_OPTION_ARG_STRING, &wcellar_colors[COLOR_ACTIVE], "Active color.", NULL },
	{ "inactive-color", 'i', 0, G_OPTION_ARG_STRING, &wcellar_colors[COLOR_INACTIVE], "Inactive color.", NULL },
	{ "text-color", 't', 0, G_OPTION_ARG_STRING, &wcellar_colors[COLOR_TEXT], "Text color.", NULL },
	{ "contrast", 'c', 0, G_OPTION_ARG_DOUBLE, &wcellar_contrast, "Contrast value.", NULL },
	{ NULL }
};

static gchar* wcellar_properties[] = { "main-color", "highlight-color", "active-color", "inactive-color", "text-color", NULL };

int main(int argc, char** argv)
{
	WCellarThemeGenerator* tgen = NULL;
	GOptionContext* opt_cont;
	GError* error = NULL;
	gchar user_input;
	gchar** env;
	gchar* reg_path = NULL;
	gchar* loader;
	gchar* child_argv[4] = { NULL, };

	gtk_init(&argc, &argv);

	/* init */
	wcellar_default_prefix_path = g_build_filename(g_get_home_dir(), ".wine", NULL);
	tgen = wcellar_themegenerator_new();
	wcellar_themegenerator_set_gtk_colors(tgen);
	g_object_get(tgen, "contrast", &wcellar_contrast, NULL);

	/* parsing argv */
	opt_cont = g_option_context_new(NULL);
	g_option_context_add_main_entries(opt_cont, entries, GETTEXT_PACKAGE);
	if (!g_option_context_parse(opt_cont, &argc, &argv, &error))
		g_error("Option parsing failure: %s\n", error->message);

	if (!wcellar_prefix)
		wcellar_prefix = g_strdup(wcellar_default_prefix_path);

	/* check if prefix exists and unbroken */
	if (!wcellar_common_check_prefix(wcellar_prefix))
	{
		do
		{
			g_print("Prefix does not exists or corrupted. Create new? (Y, N): ");
			scanf("%c", &user_input);
			g_print("\n");
			if (user_input == 'Y' || user_input == 'y')
				break;
			if (user_input == 'N' || user_input == 'n')
				exit(0);
		} while (TRUE);
	}

	/* prepare for reg file */
	env = g_get_environ();
	env = g_environ_setenv(env, "WINEPREFIX", wcellar_prefix, TRUE);
	if (wcellar_loader_dir)
		loader = g_build_filename(wcellar_loader_dir, "regedit", NULL);
	else
		loader = g_strdup("regedit");
	child_argv[0] = loader;

	if (wcellar_set_default)
	{
		/* set default theme */
		child_argv[1] = "-D";
		child_argv[2] = "HKEY_USERS\\S-1-5-21-0-0-0-1000";
		g_spawn_sync(NULL, child_argv, env, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, NULL, NULL, NULL, &error);
		if (error)
			g_error("%s\n", error->message);
		goto final;
	}

	/* custom colors */
	for (int i = 0; i < N_COLORS; i++)
		if (wcellar_colors[i])
			g_object_set(tgen, wcellar_properties[i], wcellar_colors[i], NULL);
	if (wcellar_contrast >= 0.1 || wcellar_contrast >= 2.0)
		g_object_set(tgen, "contrast", wcellar_contrast, NULL);

	reg_path = g_build_filename(g_get_tmp_dir(), "winetheme.reg", NULL);
	wcellar_themegenerator_create_reg_file(tgen, reg_path);

	/* apply reg-file */
	child_argv[1] = reg_path;

	g_spawn_sync(NULL, child_argv, env, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, NULL, NULL, NULL, &error);
	if (error)
		g_error("%s\n", error->message);
	g_free(loader);

final:
	/* run winecfg */
	if (!wcellar_not_run_winecfg)
	{
		if (wcellar_loader_dir)
			loader = g_build_filename(wcellar_loader_dir, "winecfg", NULL);
		else
			loader = g_strdup("winecfg");
		child_argv[0] = loader;
		child_argv[1] = NULL;
		g_spawn_sync(NULL, child_argv, env, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, NULL, NULL, NULL, &error);
		if (error)
			g_error("%s\n", error->message);
		g_free(loader);
	}

	if (tgen)
		g_object_unref(tgen);
	g_option_context_free(opt_cont);
	g_strfreev(env);
	g_free(reg_path);
	g_free(wcellar_prefix);
	g_free(wcellar_loader_dir);

	return 0;
}
