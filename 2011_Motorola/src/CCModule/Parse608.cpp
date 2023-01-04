#pragma GCC diagnostic ignored "-Wunreachable-code"

#include "Parse608.h"
#include <Resource.h>
#include <Commands.h>

#define SYSID PCLCCPDS
#define CLASS "Parse608"
#define LOG_ENABLE 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

// Character mapping for 0x20 to 0x7F
const uint8_t Chars20[0x7F - 0x20 + 1] = {
  ' ',   // 0x20 - Standard space
  '!',   // 0x21 - Exclamation mark
  '"',   // 0x22 - Quotation mark
  '#',   // 0x23 - Pounds (number) sign
  '$',   // 0x24 - Dollar sign
  '%',   // 0x25 - Percentage sign
  '&',   // 0x26 - Ampersand
  39,    // 0x27 - Apostrope
  '(',   // 0x28 - Open parentheses
  ')',   // 0x29 - Close parentheses
  225,   // 0x2A - Lower-case a with acute accent
  '+',   // 0x2B - Plus sign
  ',',   // 0x2C - Comma
  '-',   // 0x2D - Minus (hyphen) sign
  '.',   // 0x2E - Period
  '/',   // 0x2F - Slash
  '0',   // 0x30 - Zero
  '1',   // 0x31 - One
  '2',   // 0x32 - Two
  '3',   // 0x33 - Three
  '4',   // 0x34 - Four
  '5',   // 0x35 - Five
  '6',   // 0x36 - Six
  '7',   // 0x37 - Seven
  '8',   // 0x38 - Eight
  '9',   // 0x39 - Nine
  ':',   // 0x3A - Colon
  ';',   // 0x3B - Semi-colon
  '<',   // 0x3C - Less than sign
  '=',   // 0x3D - Equal sign
  '>',   // 0x3E - Greater then sign
  '?',   // 0x3F - Question mark
  '@',   // 0x40 - At sign
  'A',   // 0x41 - Upper-case A
  'B',   // 0x42 - Upper-case B
  'C',   // 0x43 - Upper-case C
  'D',   // 0x44 - Upper-case D
  'E',   // 0x45 - Upper-case E
  'F',   // 0x46 - Upper-case F
  'G',   // 0x47 - Upper-case G
  'H',   // 0x48 - Upper-case H
  'I',   // 0x49 - Upper-case I
  'J',   // 0x4A - Upper-case J
  'K',   // 0x4B - Upper-case K
  'L',   // 0x4C - Upper-case L
  'M',   // 0x4D - Upper-case M
  'N',   // 0x4E - Upper-case N
  'O',   // 0x4F - Upper-case O
  'P',   // 0x50 - Upper-case P
  'Q',   // 0x51 - Upper-case Q
  'R',   // 0x52 - Upper-case R
  'S',   // 0x53 - Upper-case S
  'T',   // 0x54 - Upper-case T
  'U',   // 0x55 - Upper-case U
  'V',   // 0x56 - Upper-case V
  'W',   // 0x57 - Upper-case W
  'X',   // 0x58 - Upper-case X
  'Y',   // 0x59 - Upper-case Y
  'Z',   // 0x5A - Upper-case Z
  '[',   // 0x5B - Open bracket
  233,   // 0x5C - Lower-case e with acute accent
  ']',   // 0x5D - Close bracket
  237,   // 0x5E - Lower-case i with acute accent
  243,   // 0x5F - Lower-case o with acute accent
  250,   // 0x60 - Lower-case u with acute accent
  'a',   // 0x61 - Lower-case a
  'b',   // 0x62 - Lower-case b
  'c',   // 0x63 - Lower-case c
  'd',   // 0x64 - Lower-case d
  'e',   // 0x65 - Lower-case e
  'f',   // 0x66 - Lower-case f
  'g',   // 0x67 - Lower-case g
  'h',   // 0x68 - Lower-case h
  'i',   // 0x69 - Lower-case i
  'j',   // 0x6A - Lower-case j
  'k',   // 0x6B - Lower-case k
  'l',   // 0x6C - Lower-case l
  'm',   // 0x6D - Lower-case m
  'n',   // 0x6E - Lower-case n
  'o',   // 0x6F - Lower-case o
  'p',   // 0x70 - Lower-case p
  'q',   // 0x71 - Lower-case q
  'r',   // 0x72 - Lower-case r
  's',   // 0x73 - Lower-case s
  't',   // 0x74 - Lower-case t
  'u',   // 0x75 - Lower-case u
  'v',   // 0x76 - Lower-case v
  'w',   // 0x77 - Lower-case w
  'x',   // 0x78 - Lower-case x
  'y',   // 0x79 - Lower-case y
  'z',   // 0x7A - Lower-case z
  231,   // 0x7B - Lower-case c with cedilla
  247,   // 0x7C - Division sign
  209,   // 0x7D - Upper-case N with tilde
  241,   // 0x7E - Lower-case n with tilde
  19,    // 0x7F - Solid block (we don't have so use center dot)
};

// Special character mapping for 0x1130 to 0x113F
// NOTE : these characters are remapped to definitions in the Motorola fonts.
const uint8_t Chars1130[0x3F - 0x30 + 1] = {
  174, // 0x30 - Registered mark symbol
  27,  // 0x31 - Degree sign
  189, // 0x32 - 1/2
  191, // 0x33 - Inverse query
  20,  // 0x34 - Trademark symbol
  162, // 0x35 - Cents sign
  163, // 0x36 - Pounds Sterling sign
  26,  // 0x37 - Music note (was 127)
  224, // 0x38 - Lower-case a with grave accent
  160, // 0x39 - Transparent space verify
  232, // 0x3A - Lower-case e with grave accent
  226, // 0x3B - Lower-case a with circumflex
  234, // 0x3C - Lower-case e with circumflex
  238, // 0x3D - Lower-case i with circumflex
  244, // 0x3E - Lower-case o with circumflex
  251  // 0x3F - Lower-case u with circumflex
};

// Special character mapping for 0x1220 to 0x123F
const uint8_t Chars1220[0x3F - 0x20 + 1] = {
  193, /* 0x20 - Capital A with acute accent */
  201, /* 0x21 - Capital E with acute accent */
  211, /* 0x22 - Capital O with acute accent */
  218, /* 0x23 - Capital U with acute accent */
  220, /* 0x24 - Capital U with diaeresis or umlaut */
  252, /* 0x25 - Small u with diaeresis or umlaut */
  145, /* 0x26 - Opening single quote */
  161, /* 0x27 - Inverted exclamation mark */
  '*', /* 0x28 - Asterisk */
  '\'',/* 0x29 - Plain single quote */
  '_', /* 0x2A - Em dash */
  169, /* 0x2B -   */
  23,  /* 0x2C - Service mark */
  183, /* 0x2D - Round bullet */
  147, /* 0x2E - Opening double quotes */
  148, /* 0x2F - Closing double quotes */
  192, /* 0x30 - Capital A with grave accent */
  194, /* 0x31 - Capital A with circumflex accent */
  199, /* 0x32 - Capital C with cedilla */
  200, /* 0x33 - Capital E with grave accent */
  202, /* 0x34 - Capital E with circumflex accent */
  203, /* 0x35 - Capital E with diaeresis or umlaut */
  235, /* 0x36 - Small e with diaeresis or umlaut */
  206, /* 0x37 - Capital I with circumflex accent */
  207, /* 0x38 - Capital I with diaeresis or umlaut */
  239, /* 0x39 - Small i with diaeresis or umlaut */
  212, /* 0x3A - Capital O with circumflex */
  217, /* 0x3B - Capital U with grave accent */
  249, /* 0x3C - Small u with grave accent */
  219, /* 0x3D - Capital U with circumflex accent */
  171, /* 0x3E - Opening guillemets */
  187  /* 0x3F - Closing guillemets */
};

// Special character mapping for 0x1320 to 0x133F
const uint8_t Chars1320[0x3F - 0x20 + 1] = {
  195,   // 0x20 - Capital A with tilde
  227,   // 0x21 - Small a with tilde
  205,   // 0x22 - Capital I with acute accent
  204,   // 0x23 - Capital I with grave accent
  236,   // 0x24 - Small i with grave accent
  210,   // 0x25 - Capital O with grave accent
  242,   // 0x26 - Small o with grave accent
  213,   // 0x27 - Capital O with tilde
  245,   // 0x28 - Small o with tilde
  '{',   // 0x29 - Opening brace
  '}',   // 0x2A - Closing brace
  '\\',  // 0x2B - Backslash
  '^',   // 0x2C - Caret
  '_',   // 0x2D - Underbar
  '|',   // 0x2E - Pipe
  '~',   // 0x2F - Tilde
  196,   // 0x30 - Capital A with diaeresis or umlaut mark
  228,   // 0x31 - Small a with diaeresis or umlaut mark
  214,   // 0x32 - Capital O with diaeresis or umlaut mark
  246,   // 0x33 - Small o with diaeresis or umlaut mark
  223,   // 0x34 - Small sharp
  165,   // 0x35 - Yen
  164,   // 0x36 - Non-specific currency sign
  24,    // 0x37 - Vertical bar
  197,   // 0x38 - Capital A with ring
  229,   // 0x39 - Small a with ring
  216,   // 0x3A - Capital O with slash
  248,   // 0x3B - Small o with slash
  28,    // 0x3C - Upper left corner
  29,    // 0x3D - Upper right corner
  30,    // 0x3E - Lower left corner
  31     // 0x3F - Lower right corner
};

static const RGBAColor ColorHashTable[] = {
  RGBA_WHITE, RGBA_GREEN, RGBA_BLUE, RGBA_CYAN,
  RGBA_RED, RGBA_YELLOW, RGBA_MAGENTA
};

static const uint8_t  CHANNEL_MASK = 0x18;
static const uint8_t  CHANNEL_1 = 0x00;
static const uint8_t  CHANNEL_2 = 0x08;

Parse608::Parse608() :
  pCommands(0),
  IsLastCommand(false),
  IsValidData(false),
  LastData1(0),
  LastData2(0)
{
  INITPTR_COMMAND( pCommands )
  Params.Field = CC_608_TOP_FIELD;
  Params.Channel = 0;
  Params.IsInCaptionMode = false;
}

Parse608::~Parse608()
{
}

bool Parse608::Callback(uint8_t byte1, uint8_t byte2)
{
  bool hasBadData = false;
  Data608 data;

  // Copy and strip parity bit.
  uint8_t data1 = byte1 & 0x7F;
  uint8_t data2 = byte2 & 0x7F;
  // Ignore double nulls, they are fillers
  if ((data1 == 0x00) && (data2 == 0x00)) {
    return true;
  }
#ifdef DEBUG_VERBOSE
    LINFO("Callback: byte1: %c %x - byte2: %c %x", byte1, byte2, byte1, byte2);
#endif
  // check for control pair redundancy
  if ((0x10 <= data1) && (data1 <= 0x1F) && (data1 == LastData1)
      && (data2 == LastData2)) {
    // Ignore, but change so we don't ignore the next one
    LastData1 = !data1;
    return true;
  }
  // initialize the data structure this set of data1, data2
  data.Character = 0;
  data.Row = 0;
  data.Indent = 0;
  data.Attributes = 0;
  data.Color = RGBA_UNDEFINED;
  // keep 'last' commands for redundancy checking
  LastData1 = data1;
  LastData2 = data2;
  // if between 0x20 and 0x7f we have an ASCII character
  if (IsValidData && (0x20 <= data1) && (data1 <= 0x7F)) {
    if (Window.Info.Style == Style_Pop_On) {
      if (Window.Info.Row) {
        data.Character = Chars20[data1 - 0x20];
        Window.ProcessCharacter(&data);
        // Process data2 immediately if ASCII
        if ((0x20 <= data2) && (data2 <= 0x7F)) {
          data.Character = Chars20[data2 - 0x20];
          Window.ProcessCharacter(&data);
        }
      }
    }
    else {
      data.Character = Chars20[data1 - 0x20];
      Window.ProcessCharacter(&data);
      // Process data2 immediately if ASCII
      if ((0x20 <= data2) && (data2 <= 0x7F)) {
        data.Character = Chars20[data2 - 0x20];
        Window.ProcessCharacter(&data);
      }
    }
  }
  else {
    LTRACE("2nd Level 0x%x,0x%x", data1, data2);
    switch (data1) {
    case 0x00:
      // data2 is our character
    {
      if ((0x20 <= data2) && (data2 <= 0x7F)) {
        // is valid character?
        data.Character = Chars20[data2 - 0x20];
      }
      else {
        hasBadData = true;
      }
      break;
    }
    // XDS Control characters
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      IsValidData = false;
      break;
    case 0x10:
      // Control
    case 0x18:
      if ((0x20 <= data2) && (data2 <= 0x2F)) {
        // Background attributes - dont care about
        // Character becomes a space - EIA-608 3.2
        data.Character = Chars20[' ' - 0x20];
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 11
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)11, &data);
      }
      break;
    case 0x11: // Control
    case 0x19:
      if ((0x20 <= data2) && (data2 <= 0x2F)) {
        // Mid-Row Codes
        data.Attributes |= CC_608_WAS_MID_ROW;
        if ((data2 & 0x01) == 0x01) {
          // Underline flag set.
          data.Attributes |= CC_608_UNDERLINE;
        }
        // italics?
        if ((data2 - 0x20) >= 0x0E) {
          data.Attributes |= CC_608_ITALICS;
        }
        else {
          int32_t ColorIndex = 0;
          // Calculate color index based on EIA-608 Spec.
          ColorIndex = (data2 - 0x20) >> 1;
          // Look up color in hash.
          data.Color = ColorHashTable[ColorIndex];
        }
        // Process Mid Row Command
        // MRC become a space - 91-119 App. B (h)(1)
        data.Character = Chars20[' ' - 0x20];
      }
      else if ((0x30 <= data2) && (data2 <= 0x3F)) {
        // Special Characters
        data.Character = Chars1130[data2 - 0x30];
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 1
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)1, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 2
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)2, &data);
      }
      break;
    case 0x12:
      // Control
    case 0x1A: {
      if ((0x20 <= data2) && (data2 <= 0x3F)) {
        // Special Characters, Backspace previous character from spec
        Window.WriteChar(0x08);
        data.Character = Chars1220[data2 - 0x20];
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 3
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)3, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 4
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)4, &data);
      }
      break;
    }
    case 0x13:
      // Control
    case 0x1B: {
      if ((0x20 <= data2) && (data2 <= 0x3F)) {
        // Special Characters, Backspace previous character from spec
        Window.WriteChar(0x08);
        // Now draw extended character
        data.Character = Chars1320[data2 - 0x20];
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 12
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)12, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 13
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)13, &data);
      }
      break;
    }
    case 0x14:
      // Control
    case 0x1C: {
      if ((0x20 <= data2) && (data2 <= 0x2F)) {
        // Miscellaneous Commands
        ProcessMiscCodes(data1, data2);
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 14
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)14, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 15
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)15, &data);
      }
      break;
    }
    case 0x15:
      // Control
    case 0x1D: {
      if ((0x20 <= data2) && (data2 <= 0x2F)) {
        // Miscellaneous Commands
        ProcessMiscCodes(data1, data2);
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 5
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)5, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 6
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)6, &data);
      }
      break;
    }
    case 0x16:
      // Control
    case 0x1E: {
      if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 7
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)7, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 8
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)8, &data);
      }
      break;
    }
    case 0x17:
      // Control
    case 0x1F: {
      if ((0x21 <= data2) && (data2 <= 0x23)) {
        // Tab Offset, Only valid immediately after a PAC
        if (IsLastCommand) {
          data.Attributes |= CC_608_TAB;
          data.Indent = (uint8_t)(data2 - 0x21 + 1);
        }
      }
      else if ((0x40 <= data2) && (data2 <= 0x5F)) {
        // Row 9
        ProcessPACData((uint8_t)(data2 - 0x40), (uint8_t)9, &data);
      }
      else if ((0x60 <= data2) && (data2 <= 0x7F)) {
        // Row 10
        ProcessPACData((uint8_t)(data2 - 0x60), (uint8_t)10, &data);
      }
      break;
    }
    // 0x01-0x0F are all EDS Data
    // 0x10-0x17 are all data channel 1 controls - handled above
    // 0x18-0x1F are all data channel 2 controls - handled above
    // 0x20-0x7F are ASCII and should have been handled above
    default: {
      LWARNING("What???");
      break;
    }
    }
    IsLastCommand = data.Attributes & CC_608_WAS_PAC;
    // process the new information
    if (hasBadData == false) {
      if (IsValidData) {
        Window.ProcessCharacter(&data);
      }
    }
  }
  return true;
}

void Parse608::ProcessPACData(uint8_t byte, uint8_t row, Data608 * data)
{
  if (NULL == data) {
#ifdef DEBUG_VERBOSE
    LWARNING("ProcessPACData: Data is null");
#endif
    return;
  }
#ifdef DEBUG_VERBOSE
    LINFO("ProcessPACData: byte: %c %x - row: %d", byte, byte, row);
#endif
  // Hash table Containing all possible values of the PAC 608 colors.
  int32_t ColorIndex = 0;
  LTRACE("Processibng PAC data");
  // indicate that we recieved a PAC
  data->Attributes |= CC_608_WAS_PAC;
  // set the new row from the PAC
  data->Row = row;
  if ((byte & 0x01) == 0x01) { // Underline flag set.
    data->Attributes |= CC_608_UNDERLINE;
  }
  // Indent?
  if (byte >= 0x10) {
    data->Attributes |= CC_608_INDENT;
    // Set Underline flag if present.
    data->Indent = (uint8_t)(((byte & 0xFE) - 0x10) << 1);
  }
  else {
    if (byte >= 0x0E) {
      data->Attributes |= CC_608_ITALICS;
    }
    else {
      ColorIndex = byte >> 1;
      data->Color = ColorHashTable[ColorIndex];
    }
  }
}

void Parse608::ProcessMiscCodes(uint8_t channel, uint8_t byte)
{
#ifdef DEBUG_VERBOSE
    LINFO("ProcessMiscCodes: byte: %c %x - channel: %d", byte, byte, channel);
#endif
  switch (byte) {
  case 0x20: {
    // RCL - Resume Caption Loading - this starts the Pop-On Style
    if (Params.IsInCaptionMode) {
      //ResetEraseTimer();
      // an RCL indicates the start of Caption Data, allow printing
      IsValidData = true;
      if (Style_Pop_On != Window.Info.Style) {
        Window.OnStyleChange(Style_Pop_On);
      }
    }
    else {
      IsValidData = false;
    }
    break;
  }
  case 0x21: {
    // BS - Backspace
    // Check if we are receiving data for the selected data channel.
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      // Note: used only during Pop-On captioning
      IsValidData = false;
    }
    break;
  }
  case 0x22: {
    // reserved (formerly Alarm Off)
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      // Unsupported, dont allow printing
      IsValidData = false;
    }
    break;
  }
  case 0x23: {
    // reserved (formerly Alarm On)
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      // Unsupported, dont allow printing
      IsValidData = false;
    }
    break;
  }
  case 0x24: {
    // DER - Delete to End of Row
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      IsValidData = true;
    }
    break;
  }
  case 0x25: // RU2 - Roll-Up Captions-2 Rows
  case 0x26: // RU3 - Roll-Up Captions-3 Rows
  case 0x27: // RU4 - Roll-Up Captions-4 Rows
  {
    if (((channel & CHANNEL_MASK) == Params.Channel) &&
        (Params.IsInCaptionMode == true)) {
      //ResetEraseTimer();
      // an RU2 indicates the start of Caption Data, allow printing
      IsValidData = true;
      if (Style_Roll_Up != Window.Info.Style) {
        Window.OnStyleChange(Style_Roll_Up);
      }
      // save tbe nubmer of rows to use...
      if (0x25 == byte) {
        Window.Info.Window1.Rows = 2;
      }
      else if (0x26 == byte) {
        Window.Info.Window1.Rows = 3;
      }
      else if (0x27 == byte) {
        Window.Info.Window1.Rows = 4;
      }
    }
    else {
      IsValidData = false;
    }
    break;
  }
  case 0x28: {
    // FON - Flash On
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      // Unsupported, dont allow printing
      IsValidData = false;
    }
    break;
  }
  case 0x29: {
    // RDC - Resume Direct Captioning
    if (((channel & CHANNEL_MASK) == Params.Channel) &&
        (Params.IsInCaptionMode)) {
      //ResetEraseTimer();
      // allow Caption printing
      IsValidData = true;
      Window.OnStyleChange(Style_Paint_On);
    }
    else {
      IsValidData = false;
    }
    break;
  }
  case 0x2A:
    // TR - Style_Text Restart
  case 0x2B:
    // RTD - Resume Style_Text Display
  {
    if (((channel & CHANNEL_MASK) == Params.Channel) &&
        (Params.IsInCaptionMode == false)) {
      //ResetEraseTimer();
      Window.DefineTextWindow();
      IsValidData = true;
    }
    else {
      IsValidData = false;
    }
    // not supported
    break;
  }
  case 0x2C: {
    // EDM - Erase Displayed Memory
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      if (Style_Roll_Up == Window.Info.Style) {
        Window.OnStyleChange(Style_None);
      }
      else if (Style_Pop_On == Window.Info.Style) {
        if (Window.Info.Window1.IsDefined && Window.Info.Window1.IsVisible) {
          pCommands->ClearWindows(CC_608_WINDOW1);
        }
        else if (Window.Info.Window2.IsDefined &&
            Window.Info.Window2.IsVisible) {
          pCommands->ClearWindows(CC_608_WINDOW2);
        }
      }
      else if (Style_Paint_On == Window.Info.Style) {
        if (Window.Info.Window1.IsDefined) {
          pCommands->ClearWindows(CC_608_WINDOW1);
        }
      }
      else {
        Window.OnStyleChange(Style_None);
      }
    }
    break;
  }
  case 0x2D: {
    // CR - Carriage Return
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      //ResetEraseTimer();
      // carriage returns only apply to Roll-Up captioning...
      if ((Style_Roll_Up == Window.Info.Style) ||
          (Style_Text == Window.Info.Style)) {
        // if we have been writing characters, send a carriage return and
        // update the pen location...
        if (Window.Info.Window1.Characters) {
          if (IsValidData) {
            //pCommands->WriteChar(CARRIAGE_RETURN);
            Window.Info.Attributes |= WAS_CARRIAGE_RET;
          }
        }
      }
    }
    // note - dont disable IsValidData here!
    break;
  }
  case 0x2E: {
    if ((channel & CHANNEL_MASK) == Params.Channel) {
      // ENM - Erase Non-Displayed Memory
      if (Style_Pop_On == Window.Info.Style) {
        if (Window.Info.Window1.IsDefined && Window.Info.Window1.IsActive) {
          pCommands->ClearWindows(CC_608_WINDOW1);
        }
        else if (Window.Info.Window2.IsDefined &&
            Window.Info.Window2.IsActive) {
          pCommands->ClearWindows(CC_608_WINDOW2);
        }
      }
    }
    break;
  }
  case 0x2F: {
    // EOC - End of Caption (Flip Memories)
    if (((channel & CHANNEL_MASK) == Params.Channel) &&
        (Params.IsInCaptionMode)) {
      //ResetEraseTimer();
      if (Style_Pop_On == Window.Info.Style) {
        Window.DefinePopOn();
      }
    }
    break;
  }
  default: {
    break;
  }
  }
}

void Parse608::UpdateDecoderParameters(AnalogTypes analog)
{
#ifdef DEBUG_VERBOSE
    LINFO("UpdateDecoderParameters: analog: %d", analog);
#endif
  switch (analog) {
  case ANALOG_CC1:
  case ANALOG_CC2:
  case ANALOG_T1:
  case ANALOG_T2:
    Params.Field = CC_608_TOP_FIELD;
    if ((analog == ANALOG_CC1) || (analog == ANALOG_T1)) {
      Params.Channel = 0x10;
    }
    else {
      Params.Channel = 0x18;
    }
    if ((analog == ANALOG_T1) || (analog == ANALOG_T2)) {
      Params.IsInCaptionMode = false;
    }
    else {
      Params.IsInCaptionMode = true;
    }
    break;
  case ANALOG_CC3:
  case ANALOG_T3:
  case ANALOG_CC4:
  case ANALOG_T4:
    Params.Field = CC_608_BOTTOM_FIELD;
    if ((analog == ANALOG_CC3) || (analog == ANALOG_T3)) {
      Params.Channel = 0x10;
    }
    else {
      Params.Channel = 0x18;
    }
    if ((analog == ANALOG_T3) || (analog == ANALOG_T4)) {
      Params.IsInCaptionMode = false;
    }
    else {
      Params.IsInCaptionMode = true;
    }
    break;
  case ANALOG_INVALID: //fallthrough
  case ANALOG_FORCE_uint32_t:
  default:
    Params.Field = CC_608_TOP_FIELD;
    Params.Channel = 0x10;
    Params.IsInCaptionMode = true;
    break;
  }
}

