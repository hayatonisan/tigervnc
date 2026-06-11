/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011-2021 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include <core/Rect.h>

#include <FL/Fl_Widget.H>

#include "EmulateMB.h"
#include "Keyboard.h"
#include "ShortcutHandler.h"

class Fl_Menu_Button;
class Fl_RGB_Image;

class CConn;
class Keyboard;
class PlatformPixelBuffer;
class Surface;

class Viewport : public Fl_Widget, protected EmulateMB,
                 protected KeyboardHandler {
public:

  Viewport(int w, int h, CConn* cc_);
  ~Viewport();

  // Most efficient format (from Viewport's point of view)
  const rfb::PixelFormat &getPreferredPF();

  // Flush updates to screen
  void updateWindow();

  // New image for the locally rendered cursor
  void setCursor();

  // Change client LED state
  void setLEDState(unsigned int state);

  void draw(Surface* dst);

  // Clipboard events
  void handleClipboardRequest();
  void handleClipboardAnnounce(bool available);
  void handleClipboardData(const char* data);

  // kit-custom: client-side scaling. Widget size is the scaled size,
  // the framebuffer keeps the server's logical size.
  int scaleValue(int v) const { return v * scale / 100; }
  int scaleCeil(int v) const { return (v * scale + 99) / 100; }
  int unscaleValue(int v) const { return v * 100 / scale; }
  int unscaleCeil(int v) const { return (v * 100 + scale - 1) / scale; }
  int serverWidth() const;
  int serverHeight() const;

  // kit-custom: resize to a new server framebuffer size (reallocates
  // the framebuffer and sets the widget to the scaled size)
  void serverResize(int w, int h);

  // Fl_Widget callback methods

  void draw() override;

  void resize(int x, int y, int w, int h) override;

  int handle(int event) override;

protected:
  void sendPointerEvent(const core::Point& pos,
                        uint16_t buttonMask) override;

private:
  bool hasFocus();

  // kit-custom: clip rect (widget coords) -> logical src rect + its
  // exact scaled dst rect
  void computeScaledRects(int X, int Y, int W, int H,
                          int* sx, int* sy, int* sw, int* sh,
                          int* dx, int* dy, int* dw, int* dh);

  // Show the currently set (or system) cursor
  void showCursor();

  static void handleClipboardChange(int source, void *data);

  void flushPendingClipboard();

  void handlePointerEvent(const core::Point& pos, uint16_t buttonMask);
  static void handlePointerTimeout(void *data);

  void resetKeyboard();

  void handleKeyPress(int systemKeyCode,
                      uint32_t keyCode, uint32_t keySym) override;
  void sendKeyPress(int systemKeyCode,
                    uint32_t keyCode, uint32_t keySym);
  void handleKeyRelease(int systemKeyCode) override;
  void sendKeyRelease(int systemKeyCode);

  static int handleSystemEvent(void *event, void *data);

  void pushLEDState();

  void initContextMenu();
  void popupContextMenu();

  static void handleOptions(void *data);

private:
  CConn* cc;

  PlatformPixelBuffer* frameBuffer;

  // kit-custom: display scale in percent (100 = off)
  int scale;

  core::Point lastPointerPos;
  uint16_t lastButtonMask;

  Keyboard* keyboard;
  ShortcutHandler shortcutHandler;
  bool shortcutBypass;
  bool shortcutActive;
  std::set<int> pressedKeys;

  bool firstLEDState;

  bool pendingClientClipboard;

  int clipboardSource;

  Fl_Menu_Button *contextMenu;

  bool menuCtrlKey;
  bool menuAltKey;

  Fl_RGB_Image *cursor;
  core::Point cursorHotspot;
  bool cursorIsBlank;
};

#endif
