#include <TDisplay.h>
#include <WindowDefinitions.h>

#define SYSID PCLCCGS
#define CLASS "TDisplay"
#define LOG_ENABLE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

//#define DEBUG_VERBOSE 1

#define RETURN_IF_INVALID_MAP( flag ) \
  if (flag & 0xFFFFFF00 ) { \
    LWARNING("Invalid map parameter"); \
    return; \
  }

TDisplay::TDisplay() :
  Current(INVALID)
{
}

TDisplay::~TDisplay()
{
}

void TDisplay::WriteChar(uint8_t c)
{
  RETURN_IF_INVALID_WINDOWID(Current);
  Windows[Current].RenderChar(c);
}

void TDisplay::CommandDefineWindowAsCurrent(WindowDef& def,
    WindowAttr& attr, PenAttr& penAttr, PenColorAttr& penColor)
{
#ifdef DEBUG_VERBOSE
  LERROR("CommandDefineWindow: ID: ^%d^", def.ID);
#endif

  RETURN_IF_INVALID_WINDOWID(def.ID);
  Current = def.ID;
  Windows[Current].Define(def, attr, penAttr, penColor);
#ifdef DEBUG_VERBOSE
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (Windows[i].Definition.ID != INVALID) {
    LWARNING("WINDOW: #%d ID: ^%d^ - visible:%s - rows/cols: %d:%d - "
    "xy: %d:%d - wh: %d:%d", i, Windows[i].Definition.ID,
    Windows[i].Definition.IsVisible
    ? "yes" : "no", Windows[i].Definition.Rows, Windows[i].Definition.Columns,
    Windows[i].X, Windows[i].Y, Windows[i].Width, Windows[i].Height);
    }
  }
#endif
//  CommandFlush();
}

void TDisplay::CommandSetWindowAttributes(WindowAttr& attr)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandSetWindowAttributes");
#endif
  RETURN_IF_INVALID_WINDOWID(Current);
  Windows[Current].SetAttributes(attr);
}

void TDisplay::CommandSetPenAttributes(PenAttr& attr)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandSetPenAttributes");
#endif
  RETURN_IF_INVALID_WINDOWID(Current);
  Windows[Current].SetPenAttributes(attr);
}

void TDisplay::CommandSetPenColor(PenColorAttr& color)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandSetPenColor");
#endif
  RETURN_IF_INVALID_WINDOWID(Current);
  Windows[Current].SetPenColor(color);
}

void TDisplay::CommandSetPenLocation(uint32_t row, uint32_t col)
{
  //yet another HACK!!!, this... setpenlocation command, usually...
  //keeps setting to row one, that's all very nice, but!!!
  //we CAN'T! scroll, and are left with only one row to view!!!
  //not very nice
//  if (row < 2) {
//    row = 2;
//  }
#ifdef DEBUG_VERBOSE
  LINFO("CommandSetPenLocation: row: %d - col: %d", row, col);
#endif
  RETURN_IF_INVALID_WINDOWID(Current);
  Windows[Current].SetPenLocation(row, col);
}

void TDisplay::CommandSetCurrentWindow(uint32_t id)
{
#ifdef DEBUG_VERBOSE
  LERROR("CommandSetCurrentWindow");
#endif
  RETURN_IF_INVALID_WINDOWID(id);
  if (Windows[id].IsValid()) {
    Current = id;
//    LERROR("CommandSetCurrentWindow: Current ID: ^%d^", Current);
  }
//  CommandFlush();
}

void TDisplay::CommandFlush()
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandFlush");
#endif
  TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (Windows[i].IsValid()) {
      Windows[i].Update();
    }
  }
}

void TDisplay::Redraw(uint32_t map)
{
  map &= 0xff;
#ifdef DEBUG_VERBOSE
  LINFO("Redraw: map %d", map);
#endif
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (Windows[i].IsValid() && map & (0x00000001 << i)) {
      Windows[i].Update();
    }
  }
}

void TDisplay::CommandDeleteWindows(uint32_t map)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandDeleteWindows: map %d", map);
#endif
  RETURN_IF_INVALID_MAP(map)
  TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (map & (0x00000001 << i)) {
      if (i == Current) {
        Current = INVALID;
      }
//      if (Windows[i].IsValid()) {
        Windows[i].Delete();
//      }
//      else {
//        LERROR("CommandDeleteWindows: window #%d is not valid", i);
//      }
    }
  }
  Redraw(~map);
}

void TDisplay::CommandClearWindows(uint32_t map)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandClearWindows: map %d", map);
#endif
  RETURN_IF_INVALID_MAP(map)
  TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (map & (0x00000001 << i)) {
//      if (Windows[i].IsValid()) {
        Windows[i].Clear();
//      }
//      else {
//        LERROR("CommandClearWindows: window #%d is not valid", i);
//      }
    }
  }
  Redraw(~map);
}

void TDisplay::CommandDisplayWindows(uint32_t map)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandDisplayWindows: map %d", map);
#endif
  RETURN_IF_INVALID_MAP(map)
  //TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (map & (0x00000001 << i)) {
//      if (Windows[i].IsValid()) {
        Windows[i].Display();
//      }
//      else {
//        LERROR("CommandDisplayWindows: window #%d is not valid", i);
//      }
    }
  }
  //Redraw(~map);
}

void TDisplay::CommandHideWindows(uint32_t map)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandHideWindows: map %d", map);
#endif
  RETURN_IF_INVALID_MAP(map)
  TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (map & (0x00000001 << i)) {
//      if (Windows[i].IsValid()) {
        Windows[i].Hide();
//      }
//      else {
//        LERROR("CommandHideWindows: window #%d is not valid", i);
//      }
    }
  }
  Redraw(~map);
}

void TDisplay::CommandToggleWindows(uint32_t map)
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandToggleWindows: map %d", map);
#endif
  RETURN_IF_INVALID_MAP(map)
  //TWindow::ClearWindows();
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
    if (map & (0x00000001 << i)) {
//      if (Windows[i].IsValid()) {
        Windows[i].Toggle();
//      }
//      else {
//        LERROR("CommandToggleWindows: window #%d is not valid", i);
//      }
    }
  }
  //Redraw(~map);
}

void TDisplay::CommandReset()
{
#ifdef DEBUG_VERBOSE
  LINFO("CommandReset");
#endif
  TWindow::ClearWindows();
  Current = INVALID;
  for (uint32_t i = 0; i < MAX_WINDOWS; i++) {
//    if (Windows[i].IsValid()) {
      Windows[i].Delete();
//    }
  }
  CancelTimer();
}

void TDisplay::CommandDelay(uint32_t lapse)
{
#ifdef DEBUG_VERBOSE
  LWARNING("\n\n\n\n\nCommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
  LWARNING("CommandDelay: lapse %d", lapse);
#endif
  //CommandFlush();
  if (lapse > 255) {
    LWARNING("Invalid delay");
    return;
  }
}

void TDisplay::CommandDelayCancel()
{
  //CommandFlush();
#ifdef DEBUG_VERBOSE
  LWARNING("CommandDelayCancel");
  LWARNING("CommandDelayCancel");
  LWARNING("CommandDelayCancel");
  LWARNING("CommandDelayCancel");
#endif
}

enum {
  kFlashOffDelay = 500,
  kFlashOnDelay = 1000,
  kFlashInterDelay = 250
};

void TDisplay::CommandFlashWindows()
{
#ifdef DEBUG_VERBOSE
  LWARNING("CommandFlashWindows");
#endif
  //CommandFlush();

  uint32_t  i;
  uint32_t  k;
  bool FlashFlag = false;
  uint32_t  ms = 0;

  //
  // The following for loop is used to create the flashing effect.
  // Previously, a timer was use to turn on and off characters,
  // unfortunately this produced a side effect where some characters would be
  // turned on and others off.  So we will process the characters we have
  // right now twice for the flash effect without a delay.
  //
  uint32_t WindowFlashMap = 0;
  for (k = 0; k < 2; k++) {
    for (i = 0; i < MAX_WINDOWS; i++) {
      if (Windows[i].IsValid()) {
        if ((WindowFlashMap & (0x00000001 << i)
             && Windows[i].IsVisible())) {
          FlashFlag = false;
          if (!FlashFlag) {
            WindowFlashMap = (WindowFlashMap & (~(0x00000001 << i)));
            ms = kFlashOffDelay; // 500
          }
          else {
            ms = kFlashOnDelay; // 1000
          }
        }
      }
    }
  }
  // Delaying would make the flashing more esthetiquelly pleasing to the
  // eye but wreaks havoc with the stack.  For now don't delay!
  if (ms > 0) {
    // Set timer
    SetTimer(ms);
  }
}

void TDisplay::SetTimer(uint32_t /*ms*/)
{
  // if (gDisplayTimerMsg.Data.TimerEvent.bActive) {
  // timer already active - ignore
  // return;
  // }
  // gDisplayTimerMsg.Data.TimerEvent.bActive = true;
  // set Active
  // gDisplayTimerMsg.Data.TimerEvent.u32Milliseconds = ms;
  // LOGUTIL_DEBUG(PCLCCGS, "SetTimer() - ms %d", ms);
}

void TDisplay::CancelTimer()
{
  // if (gDisplayTimerMsg.Data.TimerEvent.bActive) {
  // If active cancel
  // LOGUTIL_DEBUG(PCLCCGS, "CancelTimer()- active timer");
  // gDisplayTimerMsg.Data.TimerEvent.bActive = false;
  // SetTimer(0);
  // }
}

