/*
 * wcellarcommon.h
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

#ifndef _WCELLARCOMMON_H_
#define _WCELLARCOMMON_H_

#include <gio/gio.h>

typedef enum WCellarWinArch
{
	WCELLAR_ARCH_DEFAULT,
	WCELLAR_ARCH_WIN32 = 32,
	WCELLAR_ARCH_WIN64 = 64
} WCellarWinArch;

typedef struct WCellarPrefix
{
	gchar* 			name;
	GFile*			location;
	gchar*			path;
	gchar*			arch;
	guint64			size;
	gchar*			comment;
} WCellarPrefix;

extern gchar* wcellar_default_prefix_path;

WCellarWinArch wcellar_common_detect_win_arch(const gchar* path);

gchar* wcellar_common_get_prefix_arch(const gchar* path);

gboolean wcellar_common_check_prefix(const gchar* path);

void wcellar_prefix_free(WCellarPrefix* pref);

gboolean wcellar_common_run_application_simple(gchar* command_line,
											   gchar* prefix_path,
											   gchar* arch,
											   GError** error);

 #endif /* _WCELLARCOMMON_H_ */