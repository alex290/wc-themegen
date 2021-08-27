/*
 * wc-fast.c
 * 
 * Copyright 2019 meliafaro <meliafaro at inbox dot ru>
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
#include <stdlib.h>

#include <gtk/gtk.h>

#include "wcellarcommon.h"
#include "wcellarthemegenerator.h"
#include "wcellarlocale.h"

int main(int argc, char **argv)
{
	WCellarThemeGenerator* tgen = NULL;
	gchar** env;
	const gchar* loader;
	const gchar* prefix;
	gchar* reg_path;
	gchar* temp_argv[3] = {"regedit", NULL, NULL, };
	gchar user_input;
	GError* error = NULL;

	gtk_init(&argc, &argv);
	wcellar_default_prefix_path = g_build_filename(g_get_home_dir(), ".wine", NULL);
	tgen = wcellar_themegenerator_new();
	wcellar_themegenerator_set_gtk_colors(tgen);
	
	env = g_get_environ();
	prefix = g_environ_getenv(env, "WINEPREFIX");
	if (!prefix)
		prefix = wcellar_default_prefix_path;
	loader = g_environ_getenv(env, "WC_LOADER");
	if (!loader)
		loader = "wine";

	/* check if prefix exists and unbroken */
	if (!wcellar_common_check_prefix(prefix))
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

	/* creating and apply reg file */
	reg_path = g_build_filename(g_get_tmp_dir(), "winetheme.reg", NULL);
	wcellar_themegenerator_create_reg_file(tgen, reg_path);
	temp_argv[1] = reg_path;
	g_spawn_sync(NULL, temp_argv, env, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, NULL, NULL, NULL, &error);
	if (error)
		g_error("%s\n", error->message);

	/* running app */
	argv[0] = g_strdup(loader);
	g_spawn_sync(NULL, argv, env, G_SPAWN_SEARCH_PATH,
				 NULL, NULL, NULL, NULL, NULL, &error);
	if (error)
		g_error("%s\n", error->message);

	if (tgen)
		g_object_unref(tgen);
	g_strfreev(env);
	g_free(reg_path);
	g_free(argv[0]);
	g_free(wcellar_default_prefix_path);
	
	return 0;
}
