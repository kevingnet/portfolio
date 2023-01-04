#include <Classify.h>

#define SYSID PCLCCPDS
#define CLASS "Classify"
#define LOG_ENABLE 1
//#define LOG_DISABLE_TRACE 1
//#define LOG_DISABLE_DEBUG 1
//#define LOG_DISABLE_INFO 1
//#define LOG_DISABLE_WARNING 1
#include "debug.h"

#define DEBUG_VERBOSE 1

static const uint8_t CC_QUEUE_FLAG_TOP = 0x01;
static const uint8_t CC_QUEUE_FLAG_BOTTOM = 0x02;
static const uint32_t USER_DATA_START_CODE = 0xb2010000;
//static const uint32_t USER_DATA_START_CODE = 0x0000b201;
static const uint32_t EMPTY_PACKET = 0x00fa8080;
static const uint8_t COUNTRY_CODE = 0xb5;
static const uint16_t PROVIDER_CODE = 0x3100;
static const uint32_t ATSC_ID = 0x34394147;
static const uint32_t ATSC_ID2 = 0x00314147;
static const uint8_t USER_DATA_TYPE_CODE = 0x03;
static const uint8_t BAR_DATA_TYPE_CODE = 0x06;
static const uint32_t COUNTRY_MASK = 0x000000ff;
static const uint32_t PROVIDER_MASK = 0x00ffff00;

uint8_t FieldFromPicInfo(uint32_t pic, uint32_t index, uint32_t numPairs)
{
#ifdef DEBUG_VERBOSE
    LINFO("FieldFromPicInfo: pic: %d - index: %d", pic, index);
#endif
  if ((pic & 3) == 1) {
    return (CC_QUEUE_FLAG_TOP);
  }
  else if ((pic & 3) == 2) {
    return (CC_QUEUE_FLAG_BOTTOM);
  }
  else if ((pic & 3) == 3) {
    // progressive frame
    if ((numPairs == 1) || ((numPairs == 2) && (pic & 0x10))) {
      //0x10 means repeat first field, this is very strange
      return (CC_QUEUE_FLAG_TOP);
    }
    else {
      switch (pic & 0x30) {
      case 0x00:
        // bottom field first, repeat first field = 0
        return (index ? CC_QUEUE_FLAG_TOP : CC_QUEUE_FLAG_BOTTOM);
      case 0x10:
        // bottom field first, repeat first field = 1
        return ((index == 1) ? CC_QUEUE_FLAG_TOP : CC_QUEUE_FLAG_BOTTOM);
      case 0x20:
        // top    field first, repeat first field = 0
        return (index ? CC_QUEUE_FLAG_BOTTOM : CC_QUEUE_FLAG_TOP);
      case 0x30:
        // top    field first, repeat first field = 1
        return ((index == 1) ? CC_QUEUE_FLAG_BOTTOM : CC_QUEUE_FLAG_TOP);
      default:
        break;
      }
    }
  }
  return (CC_QUEUE_FLAG_BOTTOM); // ???
}

uint32_t getnextbit(uint8_t * pBuf, int * pbyteoff, int * pbitoff)
{
  uint8_t mask = static_cast<uint8_t>(0x80 >> *pbitoff);
  uint8_t maskedbyte = pBuf[*pbyteoff] & mask;
  if (++(*pbitoff) == 8) {
    *pbitoff = 0;
    (*pbyteoff)++;
  }
  return (maskedbyte ? 1 : 0);
}

uint32_t getnextbits(uint8_t * pBuf,
    int * pbyteoff,
    int * pbitoff,
    int numbits,
    int bufsize)
{
  uint32_t retbits = 0;
  while (numbits--) {
    if (bufsize <= *pbyteoff) {
      LWARNING("getnextbits: data parsing is out of the input buffer!");
      break;
    }
    else {
      uint32_t newbit = getnextbit(pBuf, pbyteoff, pbitoff);
      retbits = (retbits << 1) | newbit;
    }
  }
  return (retbits);
}

uint32_t getnextbits_rev(uint8_t * pBuf,
    int * pbyteoff,
    int * pbitoff,
    int numbits,
    int bufsize)
{
  uint32_t retbits = 0;
  uint32_t bitmask = 1;
  while (numbits--) {
    if (bufsize <= *pbyteoff) {
      LWARNING("getnextbits_rev: data parsing is out of the input buffer!");
      break;
    }
    else {
      uint32_t newbit = getnextbit(pBuf, pbyteoff, pbitoff);
      if (newbit) {
        retbits |= bitmask;
      }
      bitmask <<= 1;
    }
  }
  return (retbits);
}

uint32_t nextbits(uint8_t * pBuf, int byteoff, int bitoff,
    int numbits, int bufsize)
{
  return (getnextbits(pBuf, &byteoff, &bitoff, numbits, bufsize));
}

#define MESSAGELIMIT 229

int next_start_code(uint32_t * buf, uint32_t byteCount)
{
  uint32_t byteoff = 0;
  //uint32_t numwords = byteCount / 4;
  uint32_t numwords = byteCount;
  uint8_t * pByte = (uint8_t*)buf;
  while (byteoff < numwords) {
    uint32_t * pWord = (uint32_t*)(&pByte[byteoff]);
    if (*pWord == USER_DATA_START_CODE) {
#ifdef DEBUG_VERBOSE
      LDEBUG("next_start_code: USER_DATA %x byteCount %d", *pWord, byteCount);
#endif
      return 4;
    }
    else if (((*pWord & COUNTRY_MASK) == COUNTRY_CODE)
             && (((*pWord & PROVIDER_MASK) >> 8) == PROVIDER_CODE)) {
#ifdef DEBUG_VERBOSE
      LDEBUG("next_start_code: PROVIDER %x byteCount %d", *pWord, byteCount);
#endif
      return 3;
    }
    byteoff++;
  }
#ifdef DEBUG_VERBOSE
  LWARNING("next_start_code: Invalid code: byteCount was %d", byteCount);
#endif
  return -1;
}


#ifdef DEBUG_VERBOSE
  DigitalMask Parse053(uint8_t * data, uint32_t size, uint32_t pic,
      BufferInfo& buf608, BufferInfo& bufDTV)
  {
    LINFO("Parse053: size: %d - pic: %d", size, pic);
#else
    DigitalMask Parse053(uint8_t * data, uint32_t size, uint32_t /*pic*/,
        BufferInfo& buf608, BufferInfo& bufDTV)
    {
#endif
  uint32_t i;
  uint32_t pairCount;
  uint8_t ccType;
  uint8_t dtvType;
  DigitalMask type = DigitalMask_None;

  i = 4;
  // Check if can take user_data_type and cc_data_flag safely.
  if (size <= i + 1) {
//#ifdef DEBUG_VERBOSE
    LWARNING("Parse053: data exceeds the input buffer! (exiting)");
//#endif
    return (type);
  }

  //if (data[i] != 0x03 && data[i] != 0x06) {
  if (data[i] != 0x03) {
//#ifdef DEBUG_VERBOSE
    LWARNING("Parse053: data is not 0x03 or 0x06 (exiting...)");
//#endif
    return (type);
  }
  i++;

  if (!(data[i] & 0x40)) {
//#ifdef DEBUG_VERBOSE
    LWARNING("Parse053 data & 0x40): %d", data[i]);
//#endif
    return (type);
  }

  pairCount = data[i] & 0x1f;
#ifdef DEBUG_VERBOSE
    LDEBUG("Parse053: pairCount %d", pairCount);
#endif
  i += 2; // skip em_data
  for (uint32_t j = 0; j < pairCount; j++) {
#ifdef DEBUG_VERBOSE
    LDEBUG("Parse053: for loop j = %d", j);
#endif

    if (size <= i + 2) {
      break;
    }
    if ((data[i] & 0xF8) != 0xF8) {
//#ifdef DEBUG_VERBOSE
      LWARNING("Parse053: marker bits not correct: %d", data[i]);
//#endif
      continue;
    }
    if (!(data[i] & 0x04)) {
      // if cc_valid != 1, continue to next pair
#ifdef DEBUG_VERBOSE
      LWARNING("Parse053: no cc_valid, (continue)");
#endif
      i += 3;
      continue;
    }


    ccType = data[i] & 0x03;
    dtvType = data[i] & 0x02;
    i++;
#ifdef DEBUG_VERBOSE
      LDEBUG("Parse053: dtvType: %d ccType: %d", dtvType, ccType);
#endif
    if (dtvType) {
      type = (DigitalMask)(type | DigitalMask_053_708);
      if ((bufDTV.bytes + 3) > bufDTV.size) {
        bufDTV.error = BufferErrorOutputBufTooSmall;
          LERROR("Parse053: BufferErrorOutputBufTooSmall");
      }
      else {
        bufDTV.data[(bufDTV.bytes) + 0] = ccType;
        bufDTV.data[(bufDTV.bytes) + 1] = data[i];

        bufDTV.data[(bufDTV.bytes) + 2] = data[i + 1];
        //}
        bufDTV.bytes += 3;
      }
    }
    else {
      type = (DigitalMask)(type | DigitalMask_053_608);
      if ((buf608.bytes + 3) > buf608.size) {
        buf608.error = BufferErrorOutputBufTooSmall;
          LERROR("Parse053: BufferErrorOutputBufTooSmall");
      }
      else {
        // Check the field type. 0x00 = Field 1, 0x01 = Field 2
        if (ccType == 0x00) {
          buf608.data[(buf608.bytes) + 0] = CC_QUEUE_FLAG_TOP;
        }
        else if (ccType == 0x01) {
          buf608.data[(buf608.bytes) + 0] = CC_QUEUE_FLAG_BOTTOM;
        }
        buf608.data[(buf608.bytes) + 1] = data[i + 0];
        buf608.data[(buf608.bytes) + 2] = data[i + 1];
        buf608.bytes += 3;
      }
    }
    i += 2;
  }
  if (((type & DigitalMask_053_708) == DigitalMask_053_708) &&
      ((type & DigitalMask_053_608) == DigitalMask_053_608)) {
    //Both Types present in stream, default to 708
    type = DigitalMask_053_708;
  }
#ifdef DEBUG_VERBOSE
    LINFO("Parse053: bufDTV.bytes: %d - buf608.bytes: %d\n\n",
        bufDTV.bytes, buf608.bytes);
#endif
  return (type);
}

DigitalMask Parse157(uint8_t *data, uint32_t size, uint32_t pic,
    BufferInfo& buf608)
{
#ifdef DEBUG_VERBOSE
    LINFO("Parse157: size: %d - pic: %d", size, pic);
#endif
  int byteOffset = 0;
  int bitOffset = 0;
  uint32_t field;
  uint32_t lineOffset;
  uint8_t  cc_data[2];
  uint32_t pairCount;
#ifdef DEBUG_VERBOSE
    LDEBUG("Parse157: size: %d --- buf608.bytes: %d", size, buf608.bytes);
#endif

  DigitalMask type = DigitalMask_None;

  if (getnextbits(data, &byteOffset, &bitOffset, 8, size) == 0x03) {
    // ignore the next 7 bits
    getnextbits(data, &byteOffset, &bitOffset, 7, size);
    // check vbi_data_flag
    if (getnextbits(data, &byteOffset, &bitOffset, 1, size) != 1) {
#ifdef DEBUG_VERBOSE
      LWARNING("Parse053: no VBI flag (exiting)");
#endif
      return (type);
    }
    pairCount = getnextbits(data, &byteOffset, &bitOffset, 5, size);
#ifdef DEBUG_VERBOSE
    LDEBUG("Parse157: pairCount %d", pairCount);
#endif
    for (uint32_t j = 0; j < pairCount; j++) {
      getnextbits(data, &byteOffset, &bitOffset, 2, size);
      field = getnextbits(data, &byteOffset, &bitOffset, 2, size);
      lineOffset = getnextbits(data, &byteOffset, &bitOffset, 5, size);
      cc_data[0] = (uint8_t)getnextbits_rev(data,
          &byteOffset, &bitOffset, 8, size);
      cc_data[1] = (uint8_t)getnextbits_rev(data,
          &byteOffset, &bitOffset, 8, size);
      getnextbits(data, &byteOffset, &bitOffset, 1, size);
#ifdef DEBUG_VERBOSE
    LINFO("Parse157: field: %d lineOffset: %d"
        " d cc_data[0]: %d cc_data[1]: %d",
        field, lineOffset, cc_data[0], cc_data[1]);
#endif
      if ((field != 0) && (lineOffset == 11)) {
        type = (DigitalMask)(type | DigitalMask_157_608);
        if ((buf608.bytes + 3) > buf608.size) {
          buf608.error = BufferErrorOutputBufTooSmall;
#ifdef DEBUG_VERBOSE
          LERROR("Parse157: BufferErrorOutputBufTooSmall");
#endif
        }
        else {
          buf608.data[(buf608.bytes)+ 0] = FieldFromPicInfo(pic, j, pairCount);
          buf608.data[(buf608.bytes)+ 1] = cc_data[0];
          buf608.data[(buf608.bytes)+ 2] = cc_data[1];
          buf608.bytes += 3;
        }
      }
#ifdef DEBUG_VERBOSE
      else {
        LWARNING("Parse157: could not add data. field=0 AND lineOffset != 11");
      }
#endif
    }
  }
#ifdef DEBUG_VERBOSE
    LINFO("Parse157: buf608.bytes: %d\n\n", buf608.bytes);
#endif
  return (type);
}

DigitalMask Classify(
  uint8_t * data, uint32_t size, uint32_t pic,
  BufferInfo& buf608157, BufferInfo& buf60853,
  BufferInfo& buf70853, uint8_t ** userField, uint32_t * userFieldSize)
{
#ifdef DEBUG_VERBOSE
    LINFO("Classify: size: %d - pic: %d", size, pic);
#endif
  uint8_t * buffer = 0;
  int32_t bytes;
  DigitalMask type = DigitalMask_None;

#ifdef DEBUG_VERBOSE
    LDEBUG("Classify: START LOOP size: %d\n", size);
#endif
  while ((bytes = next_start_code((uint32_t*)data, size)) >= 0) {
#ifdef DEBUG_VERBOSE
  LTRACE("Classify: LOOP while: size %d", size);
#endif
    data += bytes;
    buffer = data;
    size -= bytes;
#ifdef DEBUG_VERBOSE
  LTRACE("Classify: type code: buffer[0] %x prev %x next %x",
      buffer[0], *(buffer-1), buffer[1]);
#endif
    if (buffer[0] == USER_DATA_TYPE_CODE ||
        buffer[0] == BAR_DATA_TYPE_CODE) {
#ifdef DEBUG_VERBOSE
  LTRACE("Classify: Parse157");
#endif
      type = Parse157(buffer, size, pic, buf608157);
      data += 4;
      buffer = data;
      size -= 4;
    }
    else if ((*(uint32_t *)buffer == ATSC_ID) &&
        (buffer[4] == USER_DATA_TYPE_CODE)) {
#ifdef DEBUG_VERBOSE
  LTRACE("Classify: Parse053");
#endif
      *userField = buffer;
      *userFieldSize = size;
      type = Parse053(buffer, size, pic, buf60853, buf70853);
      // Skip 2 words processed for above pattern.
      data += 8;
      buffer = data;
      size -= 8;
    }
    else {
#ifdef DEBUG_VERBOSE
      LWARNING("Classify: Invalid code %x", *(uint32_t *)buffer);
#endif
    }
  }
#ifdef DEBUG_VERBOSE
    LDEBUG("Classify: END LOOP\n");
#endif
#ifdef DEBUG_VERBOSE
  LDEBUG("Classify: size: %d - buf608157.bytes: %d "
      "buf60853.bytes: %d buf70853.bytes: %d\n\n",
      size, buf608157.bytes, buf60853.bytes, buf70853.bytes);
#endif
  return (type);
}

