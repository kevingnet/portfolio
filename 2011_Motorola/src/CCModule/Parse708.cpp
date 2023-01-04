#pragma GCC diagnostic ignored "-Wunreachable-code"

#include "Parse708.h"

#define SYSID PCLCCPDS
#define CLASS "Parse708"
#define LOG_ENABLE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

#ifdef DEBUG_VERBOSE
#define VERBOSE_COUNTER_INC code_counter++; result = true;
#else
#define VERBOSE_COUNTER_INC result = true;
#endif


Parse708::Parse708() :
  ExtendedCode(false),
  Parser(this)
{
}

Parse708::~Parse708()
{
}

#ifdef DEBUG_VERBOSE
bool Parse708::Callback(uint8_t service, uint8_t ext, uint32_t size,
    bool error, const uint8_t data[]) {
#else
bool Parse708::Callback(uint8_t service, uint8_t, uint32_t size, bool,
    const uint8_t data[]) {
#endif
  uint32_t i;
  bool resetTimer = true;
  bool result = false;

#ifdef DEBUG_VERBOSE
  uint32_t code_counter = 0;
  LINFO("Callback: service: %d - ext: %d - size: %d",
      service, ext, size);
#endif
  // only process if we are not muted and CC Rendering and PDS are enabled
  if (!size || !data || !Window.ValidateService(service)) {
    LERROR("Invalid Caption service 0x%x",service);
    return false;
  }

  for (i = 0; i < size; i++) {
    if (ExtendedCode) {
      ExtendedCode = false;
      if (data[i] <= 0x1F) {
        // C2 code space  - - - no legal values here however we are
        // requried to move the buffer pointer differing ammounts
        // depending on the value
#ifdef DEBUG_VERBOSE
        LTRACE("C2 code Space = 0x%02x", data[i]);
#endif
        if (data[i] >= 0x08) {
          i++; VERBOSE_COUNTER_INC
          if (data[i] > 0x0f) {
            i++; VERBOSE_COUNTER_INC
            if (data[i] > 0x17) {
              i++; VERBOSE_COUNTER_INC
            }
          }
        }
      }
      else if ((data[i] >= 0x20) && (data[i] <= 0x7F)) {
        // G2 code space remap char value, then send char to be rendered
#ifdef DEBUG_VERBOSE
        LTRACE("G2 code Space = 0x%02x", data[i]);
#endif
        uint8_t c = data[i];
        switch (c) {
        case 0x20:
        case 0x21:
          c = 0x20;
          break;
        case 0x25:
          c = 0x5F;
          break;
        case 0x2A:
          c = 0x11; //MotoFont C0
          break;
        case 0x2C:
          c = 0x12; //MotoFont C0
          break;
        case 0x30:
          c = 0x13; //MotoFont C0
          break;
        case 0x31:
        case 0x32:
          c = 0x27;
          break;
        case 0x33:
        case 0x34:
          c = 0x22;
          break;
        case 0x35:
          c = 0xB7;
          break;
        case 0x39:
          c = 0x14; //MotoFont C0
          break;
        case 0x3A:
          c = 0x15; //MotoFont C0
          break;
        case 0x3C:
          c = 0x16; //MotoFont C0
          break;
        case 0x3D:
          c = 0x17; //MotoFont C0
          break;
        case 0x3F:
          c = 0x19; //MotoFont C0
          break;
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
          c = 0x25;
          break;
        case 0x7A:
          c = 0x7C;
          break;
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
          c = 0x2D;
          break;
        default:
          c = 0x20;
          break;
        }
        Window.WriteChar(c); VERBOSE_COUNTER_INC
      } // G2 code space
      else if ((data[i] >= 0x80) && (data[i] <= 0x9F)) {
        // C3 code space  - - - no legal values here however we are
        // requried to move the buffer pointer differing ammounts
        // depending on the value
        VERBOSE_COUNTER_INC
        if (data[i] >= 0x80) {
          i += 4;
          if ((data[i] > 0x87) && (data[i] <= 0x8f)) {
            i++;
          }
          VERBOSE_COUNTER_INC
        }
        break;
      }
      else if (data[i] >= 0xA0) {
        // G3 code space - - - no legal values here however we are
        // requried to move the buffer pointer differing ammounts
        // depending on the value
#ifdef DEBUG_VERBOSE
        LTRACE("G3 code Space = 0x%02x", data[i]);
#endif
        Window.WriteChar(data[i]);
        VERBOSE_COUNTER_INC
        break;
      }
    }
    else {
      if (data[i] <= 0x1F) {
        // C0 code space
        //LTRACE("C0 code Space = 0x%02x", data[i]);
        switch (data[i]) {
        // case 0x00: // NULL
        case 0x10: // EXT1 - - - next char is in the extended codes set
#ifdef DEBUG_VERBOSE
          LTRACE("Extended code space code Space = 0x%02x", data[i]);
#endif
          ExtendedCode = true; VERBOSE_COUNTER_INC
          break;
        // ETX - Required at the end of a caption text segment to terminate
        // the segment when the segment is not immediately followed by
        // another caption command.
        case 0x03: // ET - End of text         passthrough
        case 0x08: // BS - Backspace           passthrough
        case 0x0C: // FF - Form Feed           passthrough
        case 0x0D: // CR - Carriage Return.    passthrough
        case 0x0E: // SO - Shift Out           passthrough
        case 0x18: // CC - Cancel              passthrough
          //LTRACE("EXT Codes = 0x%02x", data[i]);
          Window.WriteChar(data[i]); VERBOSE_COUNTER_INC
          break;
        default:
          //LTRACE("Invalid EXT Code = 0x%02x", data[i]);
          break;
        }
        resetTimer = false;
      }
      else if ((data[i] >= 0x20) && (data[i] <= 0x7F)) {
        // Remap musical note. (0x7F to 0x1A)
        uint8_t c = data[i];
        if (c == 0x7F) {
          c = 0x1A;
        }
        Window.WriteChar(c); VERBOSE_COUNTER_INC
      }
      else if ((data[i] >= 0x80) && (data[i] <= 0x9F)) {
        // C1 code space - - - process DTVCC (708) command
        switch (data[i]) {
        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
          Window.SetCurrent(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
        LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x88:
          i++;
          Window.Clear(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x89:
          i++;
          Window.Display(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8A:
          i++;
          Window.Hide(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8B:
          i++;
          Window.Toggle(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8C:
          i++;
          Window.Delete(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8D:
          i++;
          Window.Delay(data[i]); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8E:
          Window.DelayCancel(); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
        LTRACE("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x8F:
          //Window.ResetTimer();
          // Cancel any active timer
          // MOStimerCancel(mu32TimerId, NULL);
          ExtendedCode = false;
          break;
        case 0x90:
          i++;
          Window.SetPenAttributes(&data[i], 0); VERBOSE_COUNTER_INC
          i++;
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x91:
          i++;
          Window.SetPenColor(&data[i], 0); VERBOSE_COUNTER_INC
          i += 2;
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x92:
          i++;
          Window.SetPenLocation(&data[i], 0); VERBOSE_COUNTER_INC
          i++;
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
          Window.Flush(); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LWARNING( "CMD :: Invalid");
#endif
          break;
        case 0x97:
          i++;
          Window.SetAttributes(&data[i], 0); VERBOSE_COUNTER_INC
          i += 3;
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          break;
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
          Window.DefineAsCurrent(&data[i], 0); VERBOSE_COUNTER_INC
#ifdef DEBUG_VERBOSE
          LDEBUG("    i: %d packet codes: %d", i, code_counter);
#endif
          //abort();
          i += 6;
          break;
        default:
          break;
        }
      }
      else if (data[i] >= 0xA0) {
        uint8_t c = data[i];
        // G1 code space - - - just send char to be rendered
#ifdef DEBUG_VERBOSE
        LTRACE("G1 code Space = 0x%02x", c);
#endif
        if (c == 0xA0) {
          c = 0x18;
        }
        else if (data[i] == 0xB0) {
          c = 0x1B;
        }
        Window.WriteChar(c); VERBOSE_COUNTER_INC
      }
    }
    if (resetTimer == true) {
      //resetTimer the Erase timer only wiht valid characters
      //Window.Reset();
    }
  }
#ifdef DEBUG_VERBOSE
        LTRACE("Processed %d packet codes, from size %d", code_counter, size);
#endif
  return result;
}
