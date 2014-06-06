#ifndef STDTYPES_H
#define STDTYPES_H

#if defined(_WIN32) && _MSC_VER < 1600
#include <limits.h>

namespace {
#if !defined(int8_t)
  typedef __int8            int8_t;
#endif

#if !defined(int16_t)
  typedef __int16           int16_t;
#endif

#if !defined(int32_t)
  typedef __int32           int32_t;
#endif

#if !defined(int64_t)
  typedef __int64           int64_t;
#endif

#if !defined(uint8_t)
  typedef unsigned __int8   uint8_t;
#endif

#if !defined(uint26_t)
  typedef unsigned __int16  uint16_t;
#endif

#if !defined(uint32_t)
  typedef unsigned __int32  uint32_t;
#endif

#if !defined(uint64_t)
  typedef unsigned __int64  uint64_t;
#endif
}

#else
#include <stdint.h>
#endif

#endif // STDTYPES_H
