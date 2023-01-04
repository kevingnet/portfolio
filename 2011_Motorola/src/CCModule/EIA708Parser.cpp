#include <EIA708Parser.h>
#include <Parse708.h>

#define SYSID PCLCCPDS
#define CLASS "EIA708Parser"
#define LOG_ENABLE 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

static const uint8_t USER_DATA_TYPE_CODE = 0x03;
static const uint8_t PROCESS_EM_DATA_MASK = 0x80;
static const uint8_t PROCESS_CC_DATA_MASK = 0x40;
static const uint8_t ADDITIONAL_DATA_MASK = 0x20;
static const uint8_t DATA_COUNT_MASK = 0x1F;
static const uint8_t MARKER_BITS_MASK = 0xF8;
static const uint8_t VALID_MASK = 0x04;
static const uint8_t DTV_MASK = 0x02;
static const uint8_t PACKET_HEADER = 0x03;
static const uint8_t TYPE_MASK = 0x03;
static const uint8_t SEQUENCE_NUMBERS = 0x04;
static const uint32_t ATSC_ID = 0x47413934;
static const uint8_t ATSC_ID_OFFSET = 0;
static const uint8_t DTV_TYPE_OFFSET = 0;
static const uint8_t PACKET_DATA = 2;
static const uint8_t CONSTRUCT_SIZE = 3;
static const uint8_t USER_DATA_TYPE_CODE_OFFSET = 4;
static const uint8_t DATA_COUNT_OFFSET = 5;
static const uint8_t USER_DATA_HEADER_LENGTH = 7;
static const uint8_t INVALID_SEQUENCE = 0xFF;

EIA708Parser::EIA708Parser(Parse708 * parser) :
  Parser(parser),
  PreviousSequence(INVALID_SEQUENCE),
  IsAssemblyingPacket(false)
{
}

bool EIA708Parser::Callback2(const uint8_t * data, uint32_t size)
{
  if (!Parser || !data || size < CONSTRUCT_SIZE) {
    LWARNING("Callback2: Invalid data %p: size: %d", data, size);
    return false;
  }

  uint32_t len = size;
  const uint8_t * packet = data;
  while (len >= CONSTRUCT_SIZE) {
    if ((packet[DTV_TYPE_OFFSET] & DTV_MASK) != 0) {
      if ((packet[DTV_TYPE_OFFSET] & MARKER_BITS_MASK) == MARKER_BITS_MASK) {
        AssemblePacket(packet);
      }
      else {
        LWARNING("Callback2: Invalid marker bits:%d", packet[DTV_TYPE_OFFSET]);
      }
    }
    else {
#ifdef DEBUG_VERBOSE
      LINFO("Callback: EIA-708 NTSC line 21 data should be included");
#endif
      Packet.SubstractLine21Info();
    }
    packet += CONSTRUCT_SIZE;
    len -= CONSTRUCT_SIZE;
  }
  //return ProcessPacket(); //just in freaking case!
  return true;
}

bool EIA708Parser::Callback(const uint8_t * data, uint32_t size)
{
//  PreviousSequence = 0;
//  IsAssemblyingPacket = false;
#ifdef DEBUG_VERBOSE
    LINFO("Callback: size: %d", size);
#endif
  if (!Parser || !data || size <= USER_DATA_HEADER_LENGTH) {
    LERROR("Callback: Bad data!!! - size: %d (exiting)", size);
    return false;
  }

  if ((data[DATA_COUNT_OFFSET] & PROCESS_CC_DATA_MASK) == 0) {
//#ifdef DEBUG_VERBOSE
    LWARNING("Callback: Invalid data: data %x (exiting)",
        (data[DATA_COUNT_OFFSET] & PROCESS_CC_DATA_MASK));
//#endif
    return false;
  }

  uint8_t triplets = (data[DATA_COUNT_OFFSET] & DATA_COUNT_MASK);
  uint32_t len = size - USER_DATA_HEADER_LENGTH;
  //KG: HACK!... let's see...
  //len += 2;
  const uint8_t * packet = data + USER_DATA_HEADER_LENGTH;
#ifdef DEBUG_VERBOSE
    LDEBUG("Callback: START LOOP triplets: %d len %d\n",
        triplets, len);
#endif
  while (len >= CONSTRUCT_SIZE && triplets > 0) {
#ifdef DEBUG_VERBOSE
    LTRACE("Callback: LOOP while: len %d triplets %d\n",
        len, triplets);
#endif
    if ((packet[DTV_TYPE_OFFSET] & DTV_MASK) != 0) {
      if ((packet[DTV_TYPE_OFFSET] & MARKER_BITS_MASK) == MARKER_BITS_MASK) {
        AssemblePacket(packet);
      }
      else {
        LWARNING("Callback: Invalid marker bits:%d", packet[DTV_TYPE_OFFSET]);
      }
    }
    else {
#ifdef DEBUG_VERBOSE
      LTRACE("Callback: EIA-708 NTSC line 21 data should be included");
#endif
      Packet.SubstractLine21Info();
      /* Per EIA-708 NTSC line 21 data should be included before any
         DTVCC Packet. However, we will be flexible and continue to
         process this data if defined after a DTVCC construct.*/
    }
    packet += CONSTRUCT_SIZE;
    len -= CONSTRUCT_SIZE;
    triplets--;
  }
#ifdef DEBUG_VERBOSE
    LDEBUG("Callback: END LOOP triplets: %d len %d\n",
        triplets, len);
#endif
    //return ProcessPacket(); //just in freaking case!
    return true;
}

bool EIA708Parser::IsValidPacketSequence() const
{
  if (!Packet.GetSequenceNumber()) {
    return true;
  }
  if (PreviousSequence != INVALID_SEQUENCE) {
    if (((PreviousSequence + 1) % SEQUENCE_NUMBERS) !=
        Packet.GetSequenceNumber()) {
#ifdef DEBUG_VERBOSE
    LWARNING("IsValidPacketSequence: Continuity error: cur: %d last: %d - seq",
        Packet.GetSequenceNumber(), PreviousSequence);
#endif
    }
  }
  else {
    return true;
  }
  return false;
}

//This function is a hack!
//sometimes we have an incomplete packet (actually in my tests, everytime!)
//this packets are usually WindowsDefine, which is a pretty important one.
//This function will COMPLETE! the packet and fill with zeroes so that the
//math happens right. That is, the 708 parser will NOT! screw up the rest of
//the packets.
//TODO: we need to find out what's going on...
bool EIA708Parser::ProcessPacket()
{
  bool result = false;
  if (!Packet.GetStoredLength()) {
    //We should have nothing else to do, since the packet should have
    //been processed in AssemblePacket(), this is why it has no data...
    return true;
  }
  if (!Packet.IsComplete()) {
    //Fill the packet with zeroes...actually the Zero() already did that...,
    //just advance the pointer...
#ifdef DEBUG_VERBOSE
  LWARNING("ProcessPacket: Incomplete packet, setting as complete!: "
      "Missing bytes: %d - Stored bytes: %d",
      Packet.GetMissingLength(), Packet.GetStoredLength());
#endif
    Packet.CompletePacket();
  }
  bool hasPackets = true;
#ifdef DEBUG_VERBOSE
  LDEBUG("ProcessPacket: START LOOP\n");
#endif
  while (hasPackets) {
#ifdef DEBUG_VERBOSE
    LTRACE("ProcessPacket: LOOP while: PreviousSequence: %d ",
        PreviousSequence);
#endif
    Packet.ProcessNextService();
    if (Packet.HasValidService()) {
      if (Packet.GetServiceNumber() != 0) {
#ifdef DEBUG_VERBOSE
        LTRACE("ProcessPacket: Parser->Callback(): AT LAST!!!");
#endif
        if ( true == Parser->Callback(
            Packet.GetServiceNumber(),
            Packet.GetExtensionNumber(),
            Packet.GetPacketSize(),
            Packet.IsInError(),
            Packet.GetCaptionData())) {
          result = true;
        }
      }
#ifdef DEBUG_VERBOSE
      else {
        LTRACE("ProcessPacket: Packet.GetServiceNumber() was ZERO!");
      }
#endif
      Packet.ZeroService();
    }
    else {
#ifdef DEBUG_VERBOSE
      LTRACE("ProcessPacket: Packet.HasValidService() "
  "was false: Packet.ZeroService() and exit while loop soon...");
#endif
      hasPackets = false;
      Packet.ZeroService();
    }
  }
  Packet.Zero();
  if (!result) {
    //LTRACE("ProcessPacket: FALSE result: %d ", result);
  }
  return result;
}

bool EIA708Parser::AssemblePacket(const uint8_t * data)
{
  bool result = false;
  uint8_t type = data[0] & 0x3;
#ifdef DEBUG_VERBOSE
  const char * stype = "OTHER";
  if (type == PACKET_HEADER) {
    stype = (const char*)"HEADER";
  }
  else if (type == PACKET_DATA) {
    stype = (const char*)"DATA";
  }
  //LINFO("AssemblePacket: type=%s %x %x %x %x", stype,
  //    data[0], data[1], data[2], data[3]);
  LINFO("AssemblePacket: type=%s %c %c %c %c", stype,
      data[0]& 0x7F, data[1]& 0x7F, data[2]& 0x7F, data[3]& 0x7F);
#endif
  bool isConstructValid = (data[0] & VALID_MASK) >> 2;
#ifdef DEBUG_VERBOSE
  LDEBUG("AssemblePacket: type: %d isConstructValid: %d "
      "IsAssemblyingPacket: %d PreviousSequence: %d",
      type, isConstructValid, IsAssemblyingPacket, PreviousSequence);
  if (type != PACKET_DATA && type != PACKET_HEADER) {
    LCRITICAL("AssemblePacket: !!! type: %d", type);
  }
#endif

  if (IsAssemblyingPacket) {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: IsAssemblyingPacket");
#endif
    if ((!isConstructValid && (type == PACKET_DATA)) ||
        (type == PACKET_HEADER)) {
      IsAssemblyingPacket = false;
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: set IsAssemblyingPacket = FALSE!");
#endif
      IsValidPacketSequence();
      if ((Packet.IsValid())) {
//#ifdef DEBUG_VERBOSE
    //LWARNING("AssemblePacket: Packet.IsValid(): was true, oh well");
//#endif
      }
      if (Packet.IsComplete()) {
        bool hasPackets = true;
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: START LOOP\n");
#endif
        while (hasPackets) {
#ifdef DEBUG_VERBOSE
    LTRACE("AssemblePacket: LOOP while: PreviousSequence: %d ",
        PreviousSequence);
#endif
          Packet.ProcessNextService();
          if (Packet.HasValidService()) {
            if (Packet.GetServiceNumber() != 0) {
#ifdef DEBUG_VERBOSE
              LTRACE("AssemblePacket: Parser->Callback(): AT LAST!!!");
#endif
              if ( true == Parser->Callback(
                  Packet.GetServiceNumber(),
                  Packet.GetExtensionNumber(),
                  Packet.GetPacketSize(),
                  Packet.IsInError(),
                  Packet.GetCaptionData())) {
                result = true;
              }
            }
//#ifdef DEBUG_VERBOSE
            else {
              LTRACE("AssemblePacket: Packet.GetServiceNumber() was ZERO!");
            }
//#endif
            Packet.ZeroService();
          }
          else {
#ifdef DEBUG_VERBOSE
            LTRACE("AssemblePacket: Packet.HasValidService() "
        "was false: Packet.ZeroService() and exit while loop soon...");
#endif
            hasPackets = false;
            Packet.ZeroService();
          }
        }
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: END LOOP\n");
#endif
        PreviousSequence = Packet.GetSequenceNumber();
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: PreviousSequence: %d ", PreviousSequence);
#endif
        if (Packet.IsComplete()) {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: Packet.IsComplete() "
        "is now true: Packet.Zero()");
#endif
          Packet.Zero();
          result = true;
        }
      }
      else {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: Packet.IsComplete() "
        "was false: set IsAssemblyingPacket = true");
#endif
        IsAssemblyingPacket = true;
      }
    }
  }
  if (isConstructValid) {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: isConstructValid");
#endif
    if (type == PACKET_HEADER) {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: isConstructValid Start of packet: "
        "set IsAssemblyingPacket = true and Packet.Initialize();");
#endif
      IsAssemblyingPacket = true;
      result = Packet.Initialize(&data[1]);
    }
    else if (IsAssemblyingPacket) {
#ifdef DEBUG_VERBOSE
    LDEBUG("AssemblePacket: isConstructValid "
        "IsAssemblyingPacket Packet.Store();");
#endif
      result = Packet.Store(&data[1]);
    }
  }
//#ifdef DEBUG_VERBOSE
  else {
    //LWARNING("AssemblePacket: WTF!\n\n");
  }
//#endif
  return result;
}

