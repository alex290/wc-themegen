/*
 * wcellarcommon.c
 *
 * Copyright 2018  <meliafaro at inbox dot ru>
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


#include <gio/gio.h>

#include "wcellarcommon.h"

gchar* wcellar_default_prefix_path;

WCellarWinArch wcellar_common_detect_win_arch(const gchar* path)
{
	gchar* output;
	gchar* out_err;
	gchar* command_line;
	GError* error = NULL;
	gint arch;

	command_line = g_strconcat("objdump -f ", path, NULL);
	if (g_spawn_command_line_sync(command_line, &output, &out_err, NULL, &error))
	{
		if (error)
		{
			g_print("%s", merror->message);
			g_error_free(error);
		}
		if (g_regex_match_simple("[[:space:]]pei-x86-64[[:space:]]", output, 0, 0))
			arch = WCELLAR_ARCH_WIN64;
		else if (g_regex_match_simple("[[:space:]]pei-i386[[:space:]]", output, 0, 0))
			arch = WCELLAR_ARCH_WIN32;
		else
			arch = 0;
	}
	g_free(command_line);
	g_free(output);
	g_free(out_err);
	return arch;
}

gchar*
wcellar_common_get_prefix_arch(const gchar* prefix_path)
{
	gchar* system_path, *rundll_path;
	gint flag1, flag2;

	/* first check if syswow64 exists */
	system_path = g_build_filename(prefix_path, "drive_c",
								   "windows", "syswow64", NULL);
	if (g_file_test(system_path, G_FILE_TEST_EXISTS))
		flag1 = WCELLAR_ARCH_WIN64;
	else
		flag1 = WCELLAR_ARCH_WIN32;

	/* second check rundll32.exe arch */
	rundll_path = g_build_filename(prefix_path, "drive_c",
								   "windows", "system32",
								   "rundll32.exe", NULL);
	flag2 = wcellar_common_detect_win_arch(rundll_path);

	g_free(system_path);
	g_free(rundll_path);

	if (flag1 == flag2)
		if (flag1 == WCELLAR_ARCH_WIN32)
			return "win32";
		else
			return "win64";
	else
		return NULL;
}

static gchar* entries[] = { "drive_c", "userdef.reg", "system.reg", "user.reg",
							"dosdevices", "drive_c/windows", "drive_c/Program Files",
							"drive_c/windows/system32", NULL };

gboolean
wcellar_common_check_prefix(const gchar* path)
{
	gchar* temp; /* temp path */
	gchar** it; /* iterator */

	if (!g_file_test(path, G_FILE_TEST_IS_DIR))
		return FALSE;

	it = entries;
	while (*it)
	{
		temp = g_build_filename(path, *it, NULL);
		if (!g_file_test(temp, G_FILE_TEST_EXISTS))
			return FALSE;
		g_free(temp);
		it++;
	}

	if (!wcellar_common_get_prefix_arch(path))
		return FALSE;

	return TRUE;
}


void
wcellar_prefix_free(WCellarPrefix* pref)
{
	g_free(pref->name);
	g_free(pref->path);
	g_free(pref->comment);
	g_object_unref(pref->location);
	g_free(pref);
}

gboolean
wcellar_common_run_application_simple(gchar* command_line,
									  gchar* prefix_path,
									  gchar* prefix_arch,
									  GError** error)
{
	gchar** child_env;
	gchar* child_argv[] = {command_line, NULL};
	gchar* working_dir;
	gboolean result;

	child_env = g_environ_setenv(g_get_environ(), "WINEPREFIX", prefix_path, TRUE);
	if (prefix_arch)
		child_env = g_environ_setenv(child_env, "WINEARCH", prefix_arch, TRUE);

	working_dir = g_build_filename(prefix_path, "drive_c", NULL);

	result = g_spawn_async(working_dir, child_argv, child_env,
						 G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, error);

	g_strfreev(child_env);
	g_free(working_dir);

	return result;
}
