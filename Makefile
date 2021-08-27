#  Makefile
#
#  Copyright 2018 meliafaro <meliafaro at inbox dot ru>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
#

CFLAGS = -Wall -g `pkg-config --cflags glib-2.0 gobject-2.0 gio-2.0 gtk+-3.0`

LDFLAGS = `pkg-config --libs glib-2.0 gobject-2.0 gio-2.0 gtk+-3.0`

all: wc-themegen wc-fast

wc-themegen: src/winecellar-tg.c src/wcellarthemegenerator.c src/wcellarcommon.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

wc-fast: src/wc-fast.c src/wcellarthemegenerator.c src/wcellarcommon.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	 rm -f src/$(OBJ) wc-themegen
