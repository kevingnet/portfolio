#include "Window608.h"
#include <UserSettings.h>
#include <Commands.h>

#define SYSID PCLCCPDS
#define CLASS "Window608"
#define LOG_ENABLE 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

static const uint8_t CARRIAGE_RETURN = 0x0D;

Window608::Window608() :
  pCommands(0),
  pSettings(0)
{
  INITPTR_USERSETTINGS( pSettings )
  INITPTR_COMMAND( pCommands )

  pSettings->InsertUserSettings(&Attributes, &Pen, &PenColor);
  Info.IsValidData = false;
  Info.Style = Style_None;
  Info.State = STATE_SET_PEN;
  Info.Row = 0;
  Info.Window1.Rows = 0;
  Info.Window1.Characters = 0;
  Info.Window1.IsActive = false;
  Info.Window1.IsDefined = false;
  Info.Window1.IsVisible = false;
  Info.Window1.Row = 0;
  Info.Window2.Rows = 0;
  Info.Window2.Characters = 0;
  Info.Window2.IsActive = false;
  Info.Window2.IsDefined = false;
  Info.Window2.IsVisible = false;
  Info.Window2.Row = 0;
  Pen.Size = PENSIZE_STANDARD;
  Pen.Font = FONT_0;
  Pen.IsItalic = false;
  Pen.Edge = EDGE_NONE;
}

void Window608::Initialize()
{
  //if (ERR_NO_ERROR == ErrorRet) {
    // Setup the 608 decoder
    //Update608DecoderParameters(UIOptions.ccAnalogCCType);
  //}
}

void Window608::Clear(bool window1,bool window2)
{
#ifdef DEBUG_VERBOSE
    LINFO("Clear: 1: %d 2: %d", window1, window2);
#endif
    pSettings->InsertUserSettings(&Attributes, &Pen, &PenColor);
  if (window1 == true) {
    Info.Window1.IsActive = false;
    Info.Window1.IsDefined = false;
    Info.Window1.IsVisible = false;
    Info.Window1.Rows = 0;
    Info.Window1.Characters = 0;
    Info.Window1.Row = 0;
  }
  if (window2 == true) {
    Info.Window2.IsActive = false;
    Info.Window2.IsDefined = false;
    Info.Window2.IsVisible = false;
    Info.Window2.Rows = 0;
    Info.Window2.Characters = 0;
    Info.Window2.Row = 0;
  }
}

void Window608::OnStyleChange(Window608Styles style)
{
#ifdef DEBUG_VERBOSE
    LINFO("OnStyleChange: %d", (int)style);
#endif
  if (Info.Window1.IsDefined == true) {
    Delete(true, false);
    Clear(true, false);
  }
  if (Info.Window2.IsDefined == true) {
    Delete(false, true);
    Clear(false, true);
  }
  Info.Style = style;
  Info.Attributes = 0;
  Info.State = STATE_SET_PEN;
  Pen.Size = PENSIZE_STANDARD;
  Pen.Font = FONT_0;
  Pen.IsItalic = false;
  Pen.Edge = EDGE_NONE;
  if (Info.Style == Style_Pop_On) {
    DefinePopOn();
  }
}

void Window608::Delete(bool window1, bool window2)
{
#ifdef DEBUG_VERBOSE
    LINFO("Delete: 1: %d 2: %d", window1, window2);
#endif
  if (window1 && Info.Window1.IsDefined) {
    pCommands->HideWindows(CC_608_WINDOW1);
    pCommands->DeleteWindows(CC_608_WINDOW1);
  }
  if (window2 && Info.Window2.IsDefined) {
    pCommands->HideWindows(CC_608_WINDOW2);
    pCommands->DeleteWindows(CC_608_WINDOW2);
  }
}

void Window608::Toggle()
{
#ifdef DEBUG_VERBOSE
    LINFO("Toggle");
#endif
  Definition.IsVisible = !Definition.IsVisible;
  Definition.Rows = CC_MAX_608_ROWS - 1;
  // rows are zero based
  // if italics were set clear them for the currently active window...
  if (true == Pen.IsItalic) {
    Pen.IsItalic = false;
    pSettings->InsertUserSettings(0, &Pen, 0);
    pCommands->SetPenAttributes(Pen);
  }
  //If row is less than 8, center and top justify window, else, center and
  //bottom justify window. This prevents loss of data when using large font.
  if (Info.Row < 8) {
    Definition.AnchorVertical = 0;
    Definition.AnchorPoint = ANCHOR_TOPCENTER;
    Definition.AnchorHorizontal = 79;
  }
  else {
    Definition.AnchorVertical = 74;
    Definition.AnchorPoint = ANCHOR_BOTTOMCENTER;
    Definition.AnchorHorizontal = 79;
  }
  //which window were we writting to...
  if (Info.Window1.IsActive == true) {
    //write a space to the end of the last row, before we display the window
    if (Info.Window1.Characters > 0) {
      pCommands->WriteChar(' ');
    }
    // Reposition window.
    Definition.ID = CC_608_WINDOW1;
    pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
    // hide window 2, now this window will be written to.
    pCommands->HideWindows(CC_608_WINDOW2);
    // window 1 will now be displayed...
    pCommands->DisplayWindows(CC_608_WINDOW1);
    pCommands->ClearWindows(CC_608_WINDOW2);
    //pCommands->SetCurrentWindow(CC_608_WINDOW1);
    //pCommands->SetPenLocation(0, 0);
    // hide window 2, now this window will be written to.
    Clear(true, true);
    Info.Window2.IsActive = true;
    Info.Window1.IsVisible = true;
    Info.Window1.IsDefined = true;
    Info.Window2.IsDefined = true;
  }
  else if (Info.Window2.IsActive == true) {
    //write a space to the end of the last row, before we display the window
    if (Info.Window2.Characters > 0) {
      pCommands->WriteChar(' ');
    }
    // Reposition window.
    Definition.ID = CC_608_WINDOW2; // window ID is zero based
    pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
    // hide window 1, now this window will be written to.
    pCommands->HideWindows(CC_608_WINDOW1);
    // window 2 will now be displayed...
    pCommands->DisplayWindows(CC_608_WINDOW2);
    pCommands->ClearWindows(CC_608_WINDOW1);
//    pCommands->SetCurrentWindow(CC_608_WINDOW1);
//    pCommands->SetPenLocation(0, 0);
    // hide window 1, now this window will be written to.
    Clear(true, true);
    Info.Window1.IsActive = true;
    Info.Window2.IsVisible = true;
    Info.Window1.IsDefined = true;
    Info.Window2.IsDefined = true;
  }
  Info.State = STATE_SET_PEN;
  Info.Row = 0;
  Info.Indent = 0;
}

void Window608::DefinePopOn()
{
#ifdef DEBUG_VERBOSE
    LINFO("DefinePopOn");
#endif
  LTRACE("Define Window");
  Attributes.FillOpacity = OPACITY_TRANSPARENT;
  Attributes.FillOpacity = OPACITY_TRANSLUCENT;
  Attributes.DisplayEffect = DISPLAYEFFECT_FADE;
  Attributes.EffectSpeed = 0;
  Definition.IsVisible = false;
  Definition.Rows = CC_MAX_608_ROWS - 1;
  Definition.ID = CC_608_WINDOW1;
  pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
  // define window2. set the window definition parameters
  Definition.ID = CC_608_WINDOW2;
  // window ID is zero based
  Info.Window1.IsDefined = true;
  Info.Window2.IsDefined = true;
  Info.Window1.IsActive = true;
  pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
}

void Window608::DefineRollUp()
{
#ifdef DEBUG_VERBOSE
    LINFO("DefineRollUp");
#endif
  Attributes.FillOpacity = OPACITY_TRANSPARENT;
  Attributes.FillOpacity = OPACITY_TRANSLUCENT;
  Attributes.DisplayEffect = DISPLAYEFFECT_FADE;
  Attributes.EffectSpeed = 0;
  Definition.ID = CC_608_WINDOW1;
  Definition.IsVisible = false;
  Definition.IsVisible = true;
  Definition.Rows = Info.Window1.Rows - 1;
  uint8_t Row = Info.Row;
  //Calculate window position based on row. If row is less than 8, calculate
  //anchor vertical from the top, else, calculate anchor vertical from the
  //bottom. This prevents loss of data when using large font.
  if (Row < 8) {
    int32_t vertAnchor = 0;
    vertAnchor =
      (int32_t)((double)((Row - 1 - (int32_t)Definition.Rows) * 75. / 15.));
    if (vertAnchor < 0) {
      vertAnchor = 0;
    }
    Definition.AnchorVertical = (uint32_t)vertAnchor;
    Definition.AnchorPoint = ANCHOR_TOPCENTER;
    Definition.AnchorHorizontal = 79;
  }
  else {
    Definition.AnchorVertical = ((uint32_t)(Row * 75. / 15.)) - 1;
    Definition.AnchorPoint = ANCHOR_BOTTOMCENTER;
    Definition.AnchorHorizontal = 79;
  }
  pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
  pCommands->SetPenLocation((uint32_t)0, (uint32_t)Info.Indent);
  // write a space to the new line
  pCommands->WriteChar(' ');
  Info.Window1.IsDefined = true;
  Info.Window1.IsVisible = true;
  Info.Window1.IsActive = true;
  Info.Window1.Row = Info.Row;
  pCommands->DisplayWindows(CC_608_WINDOW1);
}

void Window608::DefinePaintOn()
{
#ifdef DEBUG_VERBOSE
    LINFO("DefinePaintOn");
#endif
  Attributes.FillOpacity = OPACITY_TRANSPARENT;
  Attributes.FillOpacity = OPACITY_TRANSLUCENT;
  Attributes.DisplayEffect = DISPLAYEFFECT_FADE;
  Attributes.EffectSpeed = 0;
  Definition.ID = CC_608_WINDOW1;
  Definition.IsVisible = false;
  Definition.IsVisible = true;
  Definition.Rows = 15 - 1;
  //rows are zero based If row is less than 8, center and top justify window,
  //else, center and bottom justify window.
  //This prevents loss of data when using large font.
  if (Info.Row < 8) {
    Definition.AnchorVertical = 0;
    Definition.AnchorPoint = ANCHOR_TOPCENTER;
    Definition.AnchorHorizontal = 79;
  }
  else {
    Definition.AnchorVertical = 74;
    Definition.AnchorPoint = ANCHOR_BOTTOMCENTER;
    Definition.AnchorHorizontal = 79;
  }
  pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
  Info.Window1.IsDefined = true;
  Info.Window1.IsVisible = true;
  Info.Window1.IsActive = true;
  pCommands->DisplayWindows(CC_608_WINDOW1);
}

void Window608::DefineTextWindow()
{
#ifdef DEBUG_VERBOSE
    LINFO("DefineTextWindow");
#endif
  if ((Info.Window1.IsDefined == false) || (Info.Style != Style_Text)) {
    OnStyleChange(Style_Text);
    Attributes.FillOpacity = OPACITY_SOLID;
    Attributes.DisplayEffect = DISPLAYEFFECT_SNAP;
    Attributes.EffectSpeed = 0;
    Definition.ID = CC_608_WINDOW1;
    Definition.IsVisible = false;
    Definition.IsVisible = true;
    Definition.Rows = 15 - 1;
    //rows are zero based If row is less than 8, center and top justify window
    //else, center and bottom justify window.
    //This prevents loss of data when using large font.
    if (Info.Row < 8) {
      Definition.AnchorVertical = 0;
      Definition.AnchorPoint = ANCHOR_TOPCENTER;
      Definition.AnchorHorizontal = 79;
    }
    else {
      Definition.AnchorVertical = 74;
      Definition.AnchorPoint = ANCHOR_BOTTOMCENTER;
      Definition.AnchorHorizontal = 79;
    }
    pCommands->DefineWindowAsCurrent(Definition, Attributes, Pen, PenColor);
    Info.Window1.IsDefined = true;
    Info.Window1.IsVisible = true;
    Info.Window1.IsActive = true;
    pCommands->DisplayWindows(CC_608_WINDOW1);
  }
}

void Window608::Mute(void)
{
#ifdef DEBUG_VERBOSE
    LINFO("Mute");
#endif
  Info.Style = Style_None;
  Info.Row = 0;
  Info.Indent = 0;
  Info.Attributes = 0;
  Clear(true, true);
  if (pCommands) {
    pCommands->Reset();
  }
}

void Window608::ProcessCharacter(Data608 * data)
{
#ifdef DEBUG_VERBOSE
    LINFO("ProcessCharacter");
#endif
  uint8_t row = 0;
  bool rowHasChanged = false;
  bool windowIsDefined = false;
  UIState options;

  if (NULL == data) {
    return;
  }
  //ResetEraseTimer();
  // if we have row data, save it
  if (data->Row) {
    if (Info.Row != data->Row) {
      rowHasChanged = true;
      Info.State = STATE_SET_PEN;
      // reset indent to zero because sometimes when the row changes,
      // we dont get an indication from the PAC that the indent changed...
      Info.Indent = 0;
      if (true == Pen.IsItalic) {
        Pen.IsItalic = false;
        pSettings->InsertUserSettings(NULL, &Pen, NULL);
        pCommands->SetPenAttributes(Pen);
      }
    }
    Info.Row = data->Row;
  }
  if (data->Attributes & CC_608_INDENT) {
    Info.Indent = data->Indent;
  }
  if (data->Attributes & CC_608_TAB) {
    Info.Indent += (uint8_t)data->Indent;
  }
  if ((data->Attributes & CC_608_WAS_PAC) ||
      (data->Attributes & CC_608_WAS_MID_ROW)) {
    if (data->Attributes & CC_608_UNDERLINE) {
      // PAC says Underline the text.
      Pen.Underline = true;
      pCommands->SetPenAttributes(Pen);
    }
    else {
      // PAC says Underline the text.
      Pen.Underline = false;
      pCommands->SetPenAttributes(Pen);
    }
    // Get the User status to see if Auto Settings Are overridden.
    pSettings->UserStatusGet(&options);
    if (options.FGColor == COLOR_AUTO) {
      // User has not overwritten the settings, process this PAC data.
      PenColorAttr penColor;
      pSettings->InsertUserSettings(NULL, NULL, &penColor);
      if (data->Color != RGBA_UNDEFINED) {
        // PAC says Color the text.
        penColor.FG = data->Color;
      }
      else {
        penColor.FG = COLOR_WHITE;
      }
      pCommands->SetPenColor(penColor);
    }
  }
  LTRACE("Window Style = %d", Info.Style);
  switch (Info.Style) {
  case Style_Pop_On: {
    // Write trailing space when row changes.
    if (rowHasChanged == true) {
      if (Info.Window1.IsActive == true) {
        if (Info.Window1.Characters > 0) {
          pCommands->WriteChar(' ');
        }
        Info.Window1.Characters = 0;
      }
      else if (Info.Window2.IsActive == true) {
        if (Info.Window2.Characters > 0) {
          pCommands->WriteChar(' ');
        }
        Info.Window2.Characters = 0;
      }
      else {
        LTRACE("No windows active");
      }
    }
    // update the pen location when we get a new character to write...
    if ((STATE_SET_PEN == Info.State) && data->Character) {
      if (!Info.Row) {
        Info.Row = 1;
      }
      pCommands->SetPenLocation(Info.Row - 1, Info.Indent);
      // write a space to the start of the new line
      pCommands->WriteChar(' ');
      Info.State = STATE_WRITE_CHAR;
    }
    break;
  }
  case Style_Roll_Up: {
    // ok to define the Roll-Up window once we have characters....
    if ((Info.Window1.IsDefined == false) && (data->Character == true)) {
      // for the ASV CC simulator, which appears to send 0 for a row number...
      if (0 == Info.Row) {
        Info.Row = 1;
      }
      DefineRollUp();
    }
    // if we are defined, get a new list of characters to print...
    if ((Info.Window1.IsDefined == true) && (data->Character == true)) {
      //need to handle the case where we are in roll-up mode and the base
      //row changes. ie if we are in RU2 at row 2 and we get a new RU2 at
      //row 15 we need to re-define a cc window based at the new row...
      if ((Info.Row > (Info.Window1.Row + 1)) ||
          (Info.Row < (Info.Window1.Row - 1))) {
        //save the number of rows and force a style change which will
        //delete the current window and re-initialize 608 CC data variables
        uint8_t rows = Info.Window1.Rows;
        OnStyleChange(Style_Roll_Up);
        // restore the number of rows and re-define the window.
        Info.Window1.Rows = rows;
        DefineRollUp();
      }
      // update the pen location
      else if (Info.Attributes & WAS_CARRIAGE_RET) {
        //pCommands->SetPenLocationInCCGS(INVALID_ROW, Info.Indent);
        pCommands->SetPenLocation(0, Info.Indent);
        Info.Window1.Characters = 0;
        // we need to start each line with a white space...
        pCommands->WriteChar(' ');
        Info.Attributes = 0;
      }
    }
    break;
  }
  case Style_Paint_On: {
    // define window1 if it is not already defined
    if (!Info.Window1.IsDefined && data->Character) {
      DefineRollUp();
    }
    // when the row changes, write a space to the end of the old line
    if (rowHasChanged == true) {
      if (Info.Window1.Characters > 0) {
        Info.Window1.Rows++;
        // reset the number of characters in case the row
        // changes twice, ie 2 PAC commands.
        Info.Window1.Characters = 0;
      }
    }
    row = Info.Row;
    if (Info.Row) {
      row = (uint8_t)(Info.Row - 1);
    }
    // update the pen location when we get a new character to write...
    if ((STATE_SET_PEN == Info.State)
        && data->Character) {
      if (Info.Window1.IsActive) {
        pCommands->SetPenLocation(row, Info.Indent);
      }
      // write a space to the start of the new line
      pCommands->WriteChar(' ');
      Info.State = STATE_WRITE_CHAR;
    }
    break;
  }
  case Style_Text: {
    if (!Info.Window1.IsDefined && data->Character) {
      DefineRollUp();
    }
    // when the row changes, write a space to the end of the old line
    if (rowHasChanged) {
      // when we are writing characters and the row changes...
      if (Info.Window1.Characters > 0) {
        Info.Window1.Rows++;
        // write a space to the end of the old line
        pCommands->WriteChar(' ');
        // reset the number of chars in case the row changes twice,
        //ie 2 PAC commands.
        Info.Window1.Characters = 0;
      }
    }
    if (Info.Row) {
      row = (uint8_t)(Info.Row - 1);
    }
    // update the pen location when we get a new character to write...
    if ((STATE_SET_PEN == Info.State) && data->Character) {
      if (Info.Window1.IsActive) {
        pCommands->SetPenLocation(row, Info.Indent);
      }
      // write a space to the start of the new line
      pCommands->WriteChar(' ');
      Info.State = STATE_WRITE_CHAR;
    }
    break;
  }
  default: {
    LTRACE("Invalid CC STYLE");
    break;
  }
  }
  // clear Italics if they were set...
  if ((data->Attributes & CC_608_WAS_PAC) ||
      (data->Attributes & CC_608_WAS_MID_ROW)) {
    if (true == Pen.IsItalic) {
      Pen.IsItalic = false;
    }
    pSettings->InsertUserSettings(NULL, &Pen, NULL);
    pCommands->SetPenAttributes(Pen);
  }
  // configure pen attributes for Italics...
  if ((false == Pen.IsItalic) &&
      (data->Attributes & CC_608_ITALICS)) {
    Pen.IsItalic = true;
    pSettings->InsertUserSettings(NULL, &Pen, NULL);
    pCommands->SetPenAttributes(Pen);
  }
  //flag used to indicate if we have defined a window before writing a char
  if ((Info.Window1.IsActive && Info.Window1.IsDefined) ||
      (Info.Window2.IsActive && Info.Window2.IsDefined)) {
    windowIsDefined = true;
  }
  if ((windowIsDefined == true) && (data->Character != 0)) {
    if (Info.Window1.IsActive) {
      Info.Window1.Characters++;
    }
    else if (Info.Window2.IsActive) {
      Info.Window2.Characters++;
    }
    if (Style_None != Info.Style) {
      pCommands->WriteChar(data->Character);
    }
  }
}
