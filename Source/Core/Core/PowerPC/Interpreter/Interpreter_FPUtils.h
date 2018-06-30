// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <cmath>
#include <limits>

#include "Common/BitUtils.h"
#include "Common/CPUDetect.h"
#include "Common/CommonTypes.h"
#include "Common/FloatUtils.h"
#include "Core/PowerPC/Gekko.h"
#include "Core/PowerPC/PowerPC.h"

constexpr double PPC_NAN = std::numeric_limits<double>::quiet_NaN();

// the 4 less-significand bits in FPSCR[FPRF]
enum class FPCC
{
  FL = 8,  // <
  FG = 4,  // >
  FE = 2,  // =
  FU = 1,  // ?
};

inline void SetFPException(u32 mask)
{
  if ((FPSCR.Hex & mask) != mask)
  {
    FPSCR.FX = 1;
  }

  FPSCR.Hex |= mask;
  FPSCR.VX = (FPSCR.Hex & FPSCR_VX_ANY) != 0;
}

inline void SetFI(int FI)
{
  if (FI)
  {
    SetFPException(FPSCR_XX);
  }
  FPSCR.FI = FI;
}

inline void UpdateFPSCR()
{
  FPSCR.VX = (FPSCR.Hex & FPSCR_VX_ANY) != 0;
  FPSCR.FEX = (FPSCR.VX & FPSCR.VE) | (FPSCR.OX & FPSCR.OE) | (FPSCR.UX & FPSCR.UE) |
              (FPSCR.ZX & FPSCR.ZE) | (FPSCR.XX & FPSCR.XE);
}

inline void Helper_UpdateCR1()
{
  PowerPC::SetCRField(1, (FPSCR.FX << 3) | (FPSCR.FEX << 2) | (FPSCR.VX << 1) | FPSCR.OX);
}

inline double ForceSingle(double value)
{
  // convert to float...
  float x = (float)value;
  if (!cpu_info.bFlushToZero && FPSCR.NI)
  {
    x = Common::FlushToZero(x);
  }
  // ...and back to double:
  return x;
}

inline double ForceDouble(double d)
{
  if (!cpu_info.bFlushToZero && FPSCR.NI)
  {
    d = Common::FlushToZero(d);
  }
  return d;
}

inline double Force25Bit(double d)
{
  u64 integral = Common::BitCast<u64>(d);

  integral = (integral & 0xFFFFFFFFF8000000ULL) + (integral & 0x8000000);

  return Common::BitCast<double>(integral);
}

inline double MakeQuiet(double d)
{
  const u64 integral = Common::BitCast<u64>(d) | Common::DOUBLE_QBIT;

  return Common::BitCast<double>(integral);
}

// these functions allow globally modify operations behaviour
// also, these may be used to set flags like FR, FI, OX, UX

inline double NI_mul(double a, double b)
{
  const double t = a * b;
  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);

    SetFPException(FPSCR_VXIMZ);
    return PPC_NAN;
  }
  return t;
}

inline double NI_div(double a, double b)
{
  const double t = a / b;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);

    if (b == 0.0)
    {
      if (a == 0.0)
      {
        SetFPException(FPSCR_VXZDZ);
      }
      else
      {
        SetFPException(FPSCR_ZX);
      }
    }
    else if (std::isinf(a) && std::isinf(b))
    {
      SetFPException(FPSCR_VXIDI);
    }

    return PPC_NAN;
  }

  return t;
}

inline double NI_add(double a, double b)
{
  const double t = a + b;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);

    SetFPException(FPSCR_VXISI);
    return PPC_NAN;
  }

  if (std::isinf(a) || std::isinf(b))
    FPSCR.ClearFIFR();

  return t;
}

inline double NI_sub(double a, double b)
{
  const double t = a - b;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);

    SetFPException(FPSCR_VXISI);
    return PPC_NAN;
  }

  if (std::isinf(a) || std::isinf(b))
    FPSCR.ClearFIFR();

  return t;
}

// FMA instructions on PowerPC are weird:
// They calculate (a * c) + b, but the order in which
// inputs are checked for NaN is still a, b, c.
inline double NI_madd(double a, double c, double b)
{
  double t = a * c;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b) || Common::IsSNAN(c))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);  // !
    if (std::isnan(c))
      return MakeQuiet(c);

    SetFPException(FPSCR_VXIMZ);
    return PPC_NAN;
  }

  t += b;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(b))
      return MakeQuiet(b);

    SetFPException(FPSCR_VXISI);
    return PPC_NAN;
  }

  if (std::isinf(a) || std::isinf(b) || std::isinf(c))
    FPSCR.ClearFIFR();

  return t;
}

inline double NI_msub(double a, double c, double b)
{
  double t = a * c;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(a) || Common::IsSNAN(b) || Common::IsSNAN(c))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(a))
      return MakeQuiet(a);
    if (std::isnan(b))
      return MakeQuiet(b);  // !
    if (std::isnan(c))
      return MakeQuiet(c);

    SetFPException(FPSCR_VXIMZ);
    return PPC_NAN;
  }

  t -= b;

  if (std::isnan(t))
  {
    if (Common::IsSNAN(b))
      SetFPException(FPSCR_VXSNAN);

    FPSCR.ClearFIFR();

    if (std::isnan(b))
      return MakeQuiet(b);

    SetFPException(FPSCR_VXISI);
    return PPC_NAN;
  }

  if (std::isinf(a) || std::isinf(b) || std::isinf(c))
    FPSCR.ClearFIFR();

  return t;
}

// used by stfsXX instructions and ps_rsqrte
inline u32 ConvertToSingle(u64 x)
{
  u32 exp = (x >> 52) & 0x7ff;
  if (exp > 896 || (x & ~Common::DOUBLE_SIGN) == 0)
  {
    return ((x >> 32) & 0xc0000000) | ((x >> 29) & 0x3fffffff);
  }
  else if (exp >= 874)
  {
    u32 t = (u32)(0x80000000 | ((x & Common::DOUBLE_FRAC) >> 21));
    t = t >> (905 - exp);
    t |= (x >> 32) & 0x80000000;
    return t;
  }
  else
  {
    // This is said to be undefined.
    // The code is based on hardware tests.
    return ((x >> 32) & 0xc0000000) | ((x >> 29) & 0x3fffffff);
  }
}

// used by psq_stXX operations.
inline u32 ConvertToSingleFTZ(u64 x)
{
  u32 exp = (x >> 52) & 0x7ff;
  if (exp > 896 || (x & ~Common::DOUBLE_SIGN) == 0)
  {
    return ((x >> 32) & 0xc0000000) | ((x >> 29) & 0x3fffffff);
  }
  else
  {
    return (x >> 32) & 0x80000000;
  }
}

inline u64 ConvertToDouble(u32 value)
{
  // This is a little-endian re-implementation of the algorithm described in
  // the PowerPC Programming Environments Manual for loading single
  // precision floating point numbers.
  // See page 566 of http://www.freescale.com/files/product/doc/MPCFPE32B.pdf

  u64 x = value;
  u64 exp = (x >> 23) & 0xff;
  u64 frac = x & 0x007fffff;

  if (exp > 0 && exp < 255)  // Normal number
  {
    u64 y = !(exp >> 7);
    u64 z = y << 61 | y << 60 | y << 59;
    return ((x & 0xc0000000) << 32) | z | ((x & 0x3fffffff) << 29);
  }
  else if (exp == 0 && frac != 0)  // Subnormal number
  {
    exp = 1023 - 126;
    do
    {
      frac <<= 1;
      exp -= 1;
    } while ((frac & 0x00800000) == 0);

    return ((x & 0x80000000) << 32) | (exp << 52) | ((frac & 0x007fffff) << 29);
  }
  else  // QNaN, SNaN or Zero
  {
    u64 y = exp >> 7;
    u64 z = y << 61 | y << 60 | y << 59;
    return ((x & 0xc0000000) << 32) | z | ((x & 0x3fffffff) << 29);
  }
}
