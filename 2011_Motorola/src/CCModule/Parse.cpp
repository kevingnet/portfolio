#include "Parse.h"
#include "Classify.h"
//#include <VBIManager.h>
#include <XDSManager.h>
#include <Commands.h>
#include <UserSettings.h>
#include <iostream>
#include <fstream>
#include "hal/IHaiMpegVideoDecoder.h"

#define WRITE_BIN_FILE 1
#ifdef WRITE_BIN_FILE
#include <sys/time.h>
#include <unistd.h>
#endif

#define SYSID PCLCCPDS
#define CLASS "Parse"
#define LOG_ENABLE 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

typedef enum {
  VIDEO_DF_UNDEFINED,
  VIDEO_DF_TOP_FIELD,
  VIDEO_DF_BOTTOM_FIELD,
  VIDEO_DF_COMPLETE_FRAME,
  VIDEO_DF_RESERVED,
  VIDEO_DF_INVALID
} VideoDisplayFields;

#define MESSAGELIMIT 229

typedef enum{
  StreamTypeInvalid = 0xFF,
  StreamTypeNone = 0x00,
  StreamType708 = 0x01,
  StreamType608 = 0x02,
  StreamType708and608 = (StreamType708 | StreamType608)
} StreamTypes;

bool abort_parsing = false;
int process_next_packets = 7;

static const uint8_t  EIA708_MAX_CAPTION_SERVICES = 6;

Parse::Parse() :
  pSettings(0),
  pCommands(0),
  pXDSStream(0),
  pXDSManager(0),
  DVS_53_608Counter(0),
  DVS_157_608Counter(0),
  DVS_53_708Counter(0),
  cc_format_state(PacketFormat_FORMAT_UNKNOWN),
  cc_tsi(0)
  // pVBIManager(NULL),
{
  INITPTR_USERSETTINGS( pSettings )
  INITPTR_COMMAND( pCommands )

  pXDSManager = XDSManager::InstanceGet();
  if (pXDSManager != NULL) {
    pXDSStream = pXDSManager->GetXDSstream(1);
    if (pXDSStream == NULL) {
      LERROR("Could not get XDS stream");
    }
  }
  //Get instance of VBI interface to reinsert CC data.
  //pVBIManager = VBIManager::InstanceGet();
  abort_parsing = false;
  process_next_packets = 7;
}

Parse::~Parse()
{
}

#ifdef WRITE_BIN_FILE
bool Parse::Callback(uint32_t size, uint8_t * data, uint32_t mstime)
#else
bool Parse::Callback(uint32_t size, uint8_t * data, uint32_t)
#endif
{
  IHaiMpegVideoDecoder_TUserdataPacket * header =
    reinterpret_cast<IHaiMpegVideoDecoder_TUserdataPacket*>(data);
  if (size <= (sizeof(IHaiMpegVideoDecoder_TUserdataPacket) + 1)) {
    LCRITICAL("Callback: Invalid size %d", size);
    return false;
  }
#ifdef DEBUG_VERBOSE
  LDEBUG("Parser.Callback sizeof(IHaiMpegVideoDecoder_TUserdataPacket) %u",
      (unsigned int)sizeof(IHaiMpegVideoDecoder_TUserdataPacket));
#endif
  data += sizeof(IHaiMpegVideoDecoder_TUserdataPacket);
  size -= static_cast<uint32_t>(sizeof(IHaiMpegVideoDecoder_TUserdataPacket));

//#define WRITE_BIN_FILE 0
#ifdef WRITE_BIN_FILE
#define WRITE_MARKER( m ) \
  marker = m; \
  file.write((char*)(&marker), sizeof(uint32_t));

  if (size > 0) {

    uint32_t marker = 0;

    std::ifstream ifile ("/tmp/data.bin", std::ios::in | std::ios::binary);
    ifile.seekg(0, std::ios::end);
    long end = ifile.tellg();
    ifile.close();
    if (end > 11048576) {
      remove("/tmp/data.bin");
    }

    std::ofstream file;
    file.open ("/tmp/data.bin",
        std::ios::out | std::ios::binary | std::ios::app);

    WRITE_MARKER( 0x1abe11ed )
    WRITE_MARKER( 0xffffffff )

    WRITE_MARKER( 0x713e713e )
    WRITE_MARKER( 0xffffffff )
    file.write((char*)(&mstime), sizeof(uint32_t));
    WRITE_MARKER( 0xffffffff )

    WRITE_MARKER( 0x515e515e )
    WRITE_MARKER( 0xffffffff )

    file.write((char*)(&size), sizeof(uint32_t));

    WRITE_MARKER( 0xffffffff )
    WRITE_MARKER( 0xda7ada7a )
    WRITE_MARKER( 0xffffffff )

    file.write((char*)data, size);

    WRITE_MARKER( 0xffffffff )
    WRITE_MARKER( 0x00ddba11 )
    WRITE_MARKER( 0xffffffff )
    WRITE_MARKER( 0xffffffff )
    WRITE_MARKER( 0xffffffff )
    file.close();
  }
  data++;
  size--;
#endif
  return ParseData(data, size, (uint32_t)(header->PtsValid ? header->Pts : 0));
}

bool Parse::ParseData(uint8_t *data, uint32_t size, uint32_t pic)
{
  //DigitalMask type = DigitalMask_None;
  BufferInfo Buf608157;
  BufferInfo Buf60853;
  BufferInfo Buf70853;
  uint8_t * userData = 0;
  uint32_t userSize = 0;
  uint32_t i = 0;
  bool result = false;

  if (size > 2048) {
    //LERROR("ParseData wrong size: %u ", size);
    //return false;
  }
#ifdef DEBUG_VERBOSE
  LCRITICAL("\n\n\n ParseData size: %d ", size);
#endif

  Buf608157.bytes = 0;
  Buf60853.bytes = 0;
  Buf70853.bytes = 0;
  Classify(data, size, pic, Buf608157, Buf60853, Buf70853,
      &userData, &userSize);
#ifdef DEBUG_VERBOSE
  LDEBUG("\n\n\n ParseData 708: %d - 608: %d %d",
      Buf70853.bytes, Buf60853.bytes, Buf608157.bytes);
#endif

  if (!IsEnabled()) {
    //LWARNING("Decoder is NOT enabled!!!");
    Enable();
  }

  // Right now the Decoder with focus reinserts all Data back into the VBI
  // and we are NOT hiding.

  // Handle the VBI data reinsertion before any further processing!
  //pVBIManager->VbiReinsert(buf608157, buf60853);
  // Only log after MESSAGELIMIT messages have been received and
  // processed. Logging is extremely cpu intensive. The following log event
  // reports the various counters and bytes processed.

  cc_type_samples[cc_tsi] = StreamTypeNone;
  if (Buf70853.bytes) {
    cc_type_samples[cc_tsi] |= StreamType708;
  }
  if (Buf608157.bytes || Buf60853.bytes) {
    cc_type_samples[cc_tsi] |= StreamType608;
  }
  if (++cc_tsi == NUM_CC_TYPE_SAMPLES) {
    cc_tsi = 0;
  }

  if (Buf70853.bytes) {
    //LINFO("ParseData: Got 708 packet: cur format: %d", cc_processing_format);
    // 1. DVS-53 708 data is selected if available regardless of the
    // closed caption data stream that was previously being processed.
    // Increment the DVS-53 708 data counter for debug purposes.
    DVS_53_708Counter++;
    if (cc_format_state != PacketFormat_053_708) {
      //LINFO("ParseData: Changing to 708 mode");
      cc_format_state = PacketFormat_053_708;
      LINFO("ParseData: CHANGED TO PacketFormat_053_708");
      if (HasFocus()) {
        pCommands->Reset();
        Parser608.OnStyleChange(Style_None);
      }
    }
    DVS_157_608Counter = 0;
    DVS_53_608Counter = 0;
    //if (IsEnabled() && HasFocus()) {
      //if (!(count % 20)) {
//    LWARNING("ParseData: Buf70853.bytes %d", Buf70853.bytes);
      //}
    //result = Parser708.Callback2(Buf70853.data, Buf70853.bytes);
    result = Parser708.Callback(userData, userSize);
    //}
  }
  else if (Buf608157.bytes) {
    //LINFO("ParseData: Got 608 157 packet !!!");
    // 2. DVS-157 608 data is the secondary caption source.
    // We should always select the DVS-157 stream for rendering
    // even if a DVS-53 608 data stream is present.
    // Increment the DVS-157 608 data counter: data %p: size: %d", data,
    //size. The counter is used to
    // determine if we are transitioning from DVS-53 708 to DVS-157 608
    // captions
    DVS_157_608Counter++;
    if (cc_format_state == PacketFormat_053_708) {
      //LINFO("ParseData: 608 157 packet !!! currently processing 708");
      if (DVS_157_608Counter >= CC608_HYSTERESIS_THRESHOLD) {
        cc_format_state = PacketFormat_157_608;
        LINFO("ParseData: CHANGED TO PacketFormat_157_608");
        if (HasFocus()) {
          pCommands->Reset();
        }
        Parser608.OnStyleChange(Style_None);
      }
    }
    else {
      // Note: Transition from a DVS-53 608 to a DVS-157 608 stream should
      // happen immediately.
      if (cc_format_state != PacketFormat_157_608) {
        cc_format_state = PacketFormat_157_608;
        LINFO("ParseData: CHANGED TO PacketFormat_157_608");
        if (HasFocus()) {
          pCommands->Reset();
        }
        Parser608.OnStyleChange(Style_None);
      }
    }

    if (cc_format_state == PacketFormat_157_608) {
      DVS_53_608Counter = 0;
      DVS_53_708Counter = 0;
      for (i = 0; i < Buf608157.bytes; i += 3) {
        if (IsEnabled() && (Parser608.GetTargetField() ==
            Buf608157.data[i]) && (HasFocus())) {
          //LINFO("ParseData: 608 157 packet !!! Processing packet!!!");
          result = Parser608.Callback(Buf608157.data[i + 1],
              Buf608157.data[i + 2]);
        }
        if (CC_608_BOTTOM_FIELD == Buf608157.data[i]) {
          pXDSStream->Callback(Buf608157.data[i+ 1], Buf608157.data[i + 2]);
        }
      }
    }
    if ((Buf608157.data[Buf608157.bytes - 1] == 0x80)
        && (Buf608157.data[Buf608157.bytes - 2] == 0x80)) {
      DVS_157_608Counter--;
    }
  }
  else if (Buf60853.bytes) {
    //LINFO("ParseData: Got 608 53 packet !!!");
    // 3. DVS-53 608 data is the last caption source.
    // We should only select DVS-53 608 stream for rendering
    // if there is no other available source present.
    // Increment the DVS-53 608 data counter. The counter is used to
    // determine if we are transitioning from DVS-53 708 or DVS-157 608 to
    // DVS-53 608 captions captions and for debug purposes.
    DVS_53_608Counter++;
    if (cc_format_state != PacketFormat_053_608) {
      if (DVS_53_608Counter >= CC608_HYSTERESIS_THRESHOLD) {
        cc_format_state = PacketFormat_053_608;
        LINFO("ParseData: CHANGED TO PacketFormat_053_608");
        if (HasFocus()) {
          pCommands->Reset();
        }
        Parser608.OnStyleChange(Style_None);
      }
    }
    if (cc_format_state == PacketFormat_053_608) {
      DVS_157_608Counter = 0;
      DVS_53_708Counter = 0;
      //LINFO("ParseData: 608 53 packet !!! Processing packet!!!");
      for (i = 0; i < Buf60853.bytes; i += 3) {
        if (IsEnabled() && (Parser608.GetTargetField() ==
            Buf60853.data[i]) && (HasFocus())) {
          result = Parser608.Callback(Buf60853.data[i + 1],
              Buf60853.data[i + 2]);
        }
      }
    }
    if ((Buf60853.data[Buf60853.bytes - 1] == 0x80) &&
        (Buf60853.data[Buf60853.bytes - 2] == 0x80)) {
      DVS_53_608Counter--;
    }
  }

  if (Buf60853.bytes && cc_format_state != PacketFormat_157_608) {
    for (i = 0; i < Buf60853.bytes; i += 3) {
      if (CC_608_BOTTOM_FIELD == Buf60853.data[i]) {
        pXDSStream->Callback(Buf60853.data[i + 1], Buf60853.data[i + 2]);
      }
    }
  }

#ifdef DEBUG_VERBOSE
  LCRITICAL("ParseData: END! counters: 708 %d - 608 53:%d 157:%d",
      DVS_53_708Counter, DVS_53_608Counter, DVS_157_608Counter);
#endif
  return result;
}

bool Parse::IsEnabled()
{
  return pSettings->IsEnabled();
}

void Parse::Enable()
{
  return pSettings->SetRenderingState(STATE_ENABLE);
}

void Parse::Disable()
{
  return pSettings->SetRenderingState(STATE_DISABLE);
}

