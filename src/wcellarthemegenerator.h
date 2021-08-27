/*
 * wcellarthemegenerator.h
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

#ifndef _WCELLARTHEMEGENERATOR_H_
#define _WCELLARTHEMEGENERATOR_H_

#include <glib-object.h>

#define WCELLAR_TYPE_THEMEGENERATOR wcellar_themegenerator_get_type()
G_DECLARE_FINAL_TYPE (WCellarThemeGenerator, wcellar_themegenerator, WCELLAR, THEMEGENERATOR,  GObject)

WCellarThemeGenerator* wcellar_themegenerator_new();

gboolean wcellar_themegenerator_set_gtk_colors(WCellarThemeGenerator* self);

gboolean wcellar_themegenerator_create_reg_file(WCellarThemeGenerator* self, gchar* path);

#endif /* _WCELLARTHEMEGENERATOR_H_ */
