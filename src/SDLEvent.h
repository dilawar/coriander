/*
 * Copyright (C) 2000-2003 Damien Douxchamps  <ddouxchamps@users.sf.net>
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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __SDLEVENT_H__
#define __SDLEVENT_H__

#ifdef HAVE_SDLLIB

#include <gnome.h>
#include "preferences.h"
#include "definitions.h"
#include "thread_base.h"
#include "watch_thread.h"
#include "thread_display.h"
#include "tools.h"
#include "update_frames.h"

#define EVENTS_SLEEP_MS 50

int
SDLEventStartThread(chain_t *display_service);

void*
SDLEventThread(void *arg);

void
SDLEventStopThread(chain_t *display_service);

int
SDLHandleEvent(chain_t *display_service);

void
SDLResizeDisplay(chain_t *display_service, int width, int height);

void
OnKeyPressed(chain_t *display_service, int key, int mod);

void
OnKeyReleased(chain_t *display_service, int key, int mod);

void
OnMouseDown(chain_t *display_service, int button, int x, int y);

void
OnMouseUp(chain_t *display_service, int button, int x, int y);

void
OnMouseMotion(chain_t *display_service, int x, int y);

void
SDLCropImage(chain_t *display_service);

void
SDLSetMaxSize(chain_t *display_service);

#endif

#endif
