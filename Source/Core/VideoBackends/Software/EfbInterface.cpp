// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "VideoBackends/Software/EfbInterface.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/Swap.h"

#include "VideoBackends/Software/CopyRegion.h"
#include "VideoCommon/BPMemory.h"
#include "VideoCommon/LookUpTables.h"
#include "VideoCommon/PerfQueryBase.h"

namespace EfbInterface
{
static std::array<u8, EFB_WIDTH * EFB_HEIGHT * 6> efb;

static std::array<u32, PQ_NUM_MEMBERS> perf_values;

static inline u32 GetColorOffset(u16 x, u16 y)
{
  return (x + y * EFB_WIDTH) * 3;
}

static inline u32 GetDepthOffset(u16 x, u16 y)
{
  constexpr u32 depth_buffer_start = EFB_WIDTH * EFB_HEIGHT * 3;

  return (x + y * EFB_WIDTH) * 3 + depth_buffer_start;
}

static void SetPixelAlphaOnly(u32 offset, u8 a)
{
  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::Z24:
  case PEControl::RGB565_Z16:
    // do nothing
    break;
  case PEControl::RGBA6_Z24:
  {
    u32 a32 = a;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xffffffc0;
    val |= (a32 >> 2) & 0x0000003f;
    *dst = val;
  }
  break;
  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
  }
}

static void SetPixelColorOnly(u32 offset, u8* rgb)
{
  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::Z24:
  {
    u32 src = *(u32*)rgb;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= src >> 8;
    *dst = val;
  }
  break;
  case PEControl::RGBA6_Z24:
  {
    u32 src = *(u32*)rgb;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff00003f;
    val |= (src >> 4) & 0x00000fc0;  // blue
    val |= (src >> 6) & 0x0003f000;  // green
    val |= (src >> 8) & 0x00fc0000;  // red
    *dst = val;
  }
  break;
  case PEControl::RGB565_Z16:
  {
    INFO_LOG(VIDEO, "RGB565_Z16 is not supported correctly yet");
    u32 src = *(u32*)rgb;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= src >> 8;
    *dst = val;
  }
  break;
  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
  }
}

static void SetPixelAlphaColor(u32 offset, u8* color)
{
  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::Z24:
  {
    u32 src = *(u32*)color;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= src >> 8;
    *dst = val;
  }
  break;
  case PEControl::RGBA6_Z24:
  {
    u32 src = *(u32*)color;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= (src >> 2) & 0x0000003f;  // alpha
    val |= (src >> 4) & 0x00000fc0;  // blue
    val |= (src >> 6) & 0x0003f000;  // green
    val |= (src >> 8) & 0x00fc0000;  // red
    *dst = val;
  }
  break;
  case PEControl::RGB565_Z16:
  {
    INFO_LOG(VIDEO, "RGB565_Z16 is not supported correctly yet");
    u32 src = *(u32*)color;
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= src >> 8;
    *dst = val;
  }
  break;
  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
  }
}

static u32 GetPixelColor(u32 offset)
{
  u32 src;
  std::memcpy(&src, &efb[offset], sizeof(u32));

  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::Z24:
    return 0xff | ((src & 0x00ffffff) << 8);

  case PEControl::RGBA6_Z24:
    return Convert6To8(src & 0x3f) |                // Alpha
           Convert6To8((src >> 6) & 0x3f) << 8 |    // Blue
           Convert6To8((src >> 12) & 0x3f) << 16 |  // Green
           Convert6To8((src >> 18) & 0x3f) << 24;   // Red

  case PEControl::RGB565_Z16:
    INFO_LOG(VIDEO, "RGB565_Z16 is not supported correctly yet");
    return 0xff | ((src & 0x00ffffff) << 8);

  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
    return 0;
  }
}

static void SetPixelDepth(u32 offset, u32 depth)
{
  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::RGBA6_Z24:
  case PEControl::Z24:
  {
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= depth & 0x00ffffff;
    *dst = val;
  }
  break;
  case PEControl::RGB565_Z16:
  {
    INFO_LOG(VIDEO, "RGB565_Z16 is not supported correctly yet");
    u32* dst = (u32*)&efb[offset];
    u32 val = *dst & 0xff000000;
    val |= depth & 0x00ffffff;
    *dst = val;
  }
  break;
  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
  }
}

static u32 GetPixelDepth(u32 offset)
{
  u32 depth = 0;

  switch (bpmem.zcontrol.pixel_format)
  {
  case PEControl::RGB8_Z24:
  case PEControl::RGBA6_Z24:
  case PEControl::Z24:
  {
    depth = (*(u32*)&efb[offset]) & 0x00ffffff;
  }
  break;
  case PEControl::RGB565_Z16:
  {
    INFO_LOG(VIDEO, "RGB565_Z16 is not supported correctly yet");
    depth = (*(u32*)&efb[offset]) & 0x00ffffff;
  }
  break;
  default:
    ERROR_LOG(VIDEO, "Unsupported pixel format: %i", static_cast<int>(bpmem.zcontrol.pixel_format));
  }

  return depth;
}

static u32 GetSourceFactor(u8* srcClr, u8* dstClr, BlendMode::BlendFactor mode)
{
  switch (mode)
  {
  case BlendMode::ZERO:
    return 0;
  case BlendMode::ONE:
    return 0xffffffff;
  case BlendMode::DSTCLR:
    return *(u32*)dstClr;
  case BlendMode::INVDSTCLR:
    return 0xffffffff - *(u32*)dstClr;
  case BlendMode::SRCALPHA:
  {
    u8 alpha = srcClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::INVSRCALPHA:
  {
    u8 alpha = 0xff - srcClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::DSTALPHA:
  {
    u8 alpha = dstClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::INVDSTALPHA:
  {
    u8 alpha = 0xff - dstClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  }

  return 0;
}

static u32 GetDestinationFactor(u8* srcClr, u8* dstClr, BlendMode::BlendFactor mode)
{
  switch (mode)
  {
  case BlendMode::ZERO:
    return 0;
  case BlendMode::ONE:
    return 0xffffffff;
  case BlendMode::SRCCLR:
    return *(u32*)srcClr;
  case BlendMode::INVSRCCLR:
    return 0xffffffff - *(u32*)srcClr;
  case BlendMode::SRCALPHA:
  {
    u8 alpha = srcClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::INVSRCALPHA:
  {
    u8 alpha = 0xff - srcClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::DSTALPHA:
  {
    u8 alpha = dstClr[ALP_C] & 0xff;
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  case BlendMode::INVDSTALPHA:
  {
    u8 alpha = 0xff - dstClr[ALP_C];
    u32 factor = alpha << 24 | alpha << 16 | alpha << 8 | alpha;
    return factor;
  }
  }

  return 0;
}

static void BlendColor(u8* srcClr, u8* dstClr)
{
  u32 srcFactor = GetSourceFactor(srcClr, dstClr, bpmem.blendmode.srcfactor);
  u32 dstFactor = GetDestinationFactor(srcClr, dstClr, bpmem.blendmode.dstfactor);

  for (int i = 0; i < 4; i++)
  {
    // add MSB of factors to make their range 0 -> 256
    u32 sf = (srcFactor & 0xff);
    sf += sf >> 7;

    u32 df = (dstFactor & 0xff);
    df += df >> 7;

    u32 color = (srcClr[i] * sf + dstClr[i] * df) >> 8;
    dstClr[i] = (color > 255) ? 255 : color;

    dstFactor >>= 8;
    srcFactor >>= 8;
  }
}

static void LogicBlend(u32 srcClr, u32* dstClr, BlendMode::LogicOp op)
{
  switch (op)
  {
  case BlendMode::CLEAR:
    *dstClr = 0;
    break;
  case BlendMode::AND:
    *dstClr = srcClr & *dstClr;
    break;
  case BlendMode::AND_REVERSE:
    *dstClr = srcClr & (~*dstClr);
    break;
  case BlendMode::COPY:
    *dstClr = srcClr;
    break;
  case BlendMode::AND_INVERTED:
    *dstClr = (~srcClr) & *dstClr;
    break;
  case BlendMode::NOOP:
    // Do nothing
    break;
  case BlendMode::XOR:
    *dstClr = srcClr ^ *dstClr;
    break;
  case BlendMode::OR:
    *dstClr = srcClr | *dstClr;
    break;
  case BlendMode::NOR:
    *dstClr = ~(srcClr | *dstClr);
    break;
  case BlendMode::EQUIV:
    *dstClr = ~(srcClr ^ *dstClr);
    break;
  case BlendMode::INVERT:
    *dstClr = ~*dstClr;
    break;
  case BlendMode::OR_REVERSE:
    *dstClr = srcClr | (~*dstClr);
    break;
  case BlendMode::COPY_INVERTED:
    *dstClr = ~srcClr;
    break;
  case BlendMode::OR_INVERTED:
    *dstClr = (~srcClr) | *dstClr;
    break;
  case BlendMode::NAND:
    *dstClr = ~(srcClr & *dstClr);
    break;
  case BlendMode::SET:
    *dstClr = 0xffffffff;
    break;
  }
}

static void SubtractBlend(u8* srcClr, u8* dstClr)
{
  for (int i = 0; i < 4; i++)
  {
    int c = (int)dstClr[i] - (int)srcClr[i];
    dstClr[i] = (c < 0) ? 0 : c;
  }
}

static void Dither(u16 x, u16 y, u8* color)
{
  // No blending for RGB8 mode
  if (!bpmem.blendmode.dither || bpmem.zcontrol.pixel_format != PEControl::PixelFormat::RGBA6_Z24)
    return;

  // Flipper uses a standard 2x2 Bayer Matrix for 6 bit dithering
  static const u8 dither[2][2] = {{0, 2}, {3, 1}};

  // Only the color channels are dithered?
  for (int i = BLU_C; i <= RED_C; i++)
    color[i] = ((color[i] - (color[i] >> 6)) + dither[y & 1][x & 1]) & 0xfc;
}

void BlendTev(u16 x, u16 y, u8* color)
{
  const u32 offset = GetColorOffset(x, y);
  u32 dstClr = GetPixelColor(offset);

  u8* dstClrPtr = (u8*)&dstClr;

  if (bpmem.blendmode.blendenable)
  {
    if (bpmem.blendmode.subtract)
      SubtractBlend(color, dstClrPtr);
    else
      BlendColor(color, dstClrPtr);
  }
  else if (bpmem.blendmode.logicopenable)
  {
    LogicBlend(*((u32*)color), &dstClr, bpmem.blendmode.logicmode);
  }
  else
  {
    dstClrPtr = color;
  }

  if (bpmem.dstalpha.enable)
    dstClrPtr[ALP_C] = bpmem.dstalpha.alpha;

  if (bpmem.blendmode.colorupdate)
  {
    Dither(x, y, dstClrPtr);
    if (bpmem.blendmode.alphaupdate)
      SetPixelAlphaColor(offset, dstClrPtr);
    else
      SetPixelColorOnly(offset, dstClrPtr);
  }
  else if (bpmem.blendmode.alphaupdate)
  {
    SetPixelAlphaOnly(offset, dstClrPtr[ALP_C]);
  }
}

void SetColor(u16 x, u16 y, u8* color)
{
  u32 offset = GetColorOffset(x, y);
  if (bpmem.blendmode.colorupdate)
  {
    if (bpmem.blendmode.alphaupdate)
      SetPixelAlphaColor(offset, color);
    else
      SetPixelColorOnly(offset, color);
  }
  else if (bpmem.blendmode.alphaupdate)
  {
    SetPixelAlphaOnly(offset, color[ALP_C]);
  }
}

void SetDepth(u16 x, u16 y, u32 depth)
{
  if (bpmem.zmode.updateenable)
    SetPixelDepth(GetDepthOffset(x, y), depth);
}

u32 GetColor(u16 x, u16 y)
{
  u32 offset = GetColorOffset(x, y);
  return GetPixelColor(offset);
}

static u32 VerticalFilter(const std::array<u32, 3>& colors,
                          const std::array<u8, 7>& filterCoefficients)
{
  u8 in_colors[3][4];
  std::memcpy(&in_colors, colors.data(), sizeof(in_colors));

  // Alpha channel is not used
  u8 out_color[4];
  out_color[ALP_C] = 0;

  // All Coefficients should sum to 64, otherwise the total brightness will change, which many games
  // do on purpose to implement a brightness filter across the whole copy.
  for (int i = BLU_C; i <= RED_C; i++)
  {
    // TODO: implement support for multisampling.
    // In non-multisampling mode:
    //   * Coefficients 2, 3 and 4 sample from the current pixel.
    //   * Coefficients 0 and 1 sample from the pixel above this one
    //   * Coefficients 5 and 6 sample from the pixel below this one
    int sum =
        in_colors[0][i] * (filterCoefficients[0] + filterCoefficients[1]) +
        in_colors[1][i] * (filterCoefficients[2] + filterCoefficients[3] + filterCoefficients[4]) +
        in_colors[2][i] * (filterCoefficients[5] + filterCoefficients[6]);

    // TODO: this clamping behavior appears to be correct, but isn't confirmed on hardware.
    out_color[i] = std::min(255, sum >> 6);  // clamp larger values to 255
  }

  u32 out_color32;
  std::memcpy(&out_color32, out_color, sizeof(out_color32));
  return out_color32;
}

static u32 GammaCorrection(u32 color, const float gamma_rcp)
{
  u8 in_colors[4];
  std::memcpy(&in_colors, &color, sizeof(in_colors));

  u8 out_color[4];
  for (int i = BLU_C; i <= RED_C; i++)
  {
    out_color[i] = static_cast<u8>(
        MathUtil::Clamp(std::pow(in_colors[i] / 255.0f, gamma_rcp) * 255.0f, 0.0f, 255.0f));
  }

  u32 out_color32;
  std::memcpy(&out_color32, out_color, sizeof(out_color32));
  return out_color32;
}

// For internal used only, return a non-normalized value, which saves work later.
static yuv444 ConvertColorToYUV(u32 color)
{
  const u8 red = static_cast<u8>(color >> 24);
  const u8 green = static_cast<u8>(color >> 16);
  const u8 blue = static_cast<u8>(color >> 8);

  // GameCube/Wii uses the BT.601 standard algorithm for converting to YCbCr; see
  // http://www.equasys.de/colorconversion.html#YCbCr-RGBColorFormatConversion
  return {static_cast<u8>(0.257f * red + 0.504f * green + 0.098f * blue),
          static_cast<s8>(-0.148f * red + -0.291f * green + 0.439f * blue),
          static_cast<s8>(0.439f * red + -0.368f * green + -0.071f * blue)};
}

u32 GetDepth(u16 x, u16 y)
{
  u32 offset = GetDepthOffset(x, y);
  return GetPixelDepth(offset);
}

u8* GetPixelPointer(u16 x, u16 y, bool depth)
{
  if (depth)
    return &efb[GetDepthOffset(x, y)];
  return &efb[GetColorOffset(x, y)];
}

void EncodeXFB(u8* xfb_in_ram, u32 memory_stride, const EFBRectangle& source_rect, float y_scale,
               float gamma)
{
  if (!xfb_in_ram)
  {
    WARN_LOG(VIDEO, "Tried to copy to invalid XFB address");
    return;
  }

  const int left = source_rect.left;
  const int right = source_rect.right;
  const bool clamp_top = bpmem.triggerEFBCopy.clamp_top;
  const bool clamp_bottom = bpmem.triggerEFBCopy.clamp_bottom;
  const float gamma_rcp = 1.0f / gamma;
  const auto filter_coefficients = bpmem.copyfilter.GetCoefficients();

  // this assumes copies will always start on an even (YU) pixel and the
  // copy always has an even width, which might not be true.
  if (left & 1 || right & 1)
  {
    WARN_LOG(VIDEO, "Trying to copy XFB to from unaligned EFB source");
    // this will show up as wrongly encoded
  }

  // Scanline buffer, leave room for borders
  yuv444 scanline[EFB_WIDTH + 2];

  static std::vector<yuv422_packed> source;
  source.resize(EFB_WIDTH * EFB_HEIGHT);
  yuv422_packed* src_ptr = &source[0];

  for (int y = source_rect.top; y < source_rect.bottom; y++)
  {
    // Clamping behavior
    //   NOTE: when the clamp bits aren't set, the hardware will happily read beyond the EFB,
    //         which returns random garbage from the empty bus (confirmed by hardware tests).
    //
    //         In our implementation, the garbage just so happens to be the top or bottom row.
    //         Statistically, that could happen.
    u16 y_prev = static_cast<u16>(std::max(clamp_top ? source_rect.top : 0, y - 1));
    u16 y_next = static_cast<u16>(std::min(clamp_bottom ? source_rect.bottom : EFB_HEIGHT, y + 1));

    // Get a scanline of YUV pixels in 4:4:4 format
    for (int i = 1, x = left; x < right; i++, x++)
    {
      // Get RGB colors
      std::array<u32, 3> colors = {{GetColor(x, y_prev), GetColor(x, y), GetColor(x, y_next)}};

      // Vertical Filter (Multisampling resolve, deflicker, brightness)
      u32 filtered = VerticalFilter(colors, filter_coefficients);

      // Gamma correction happens here.
      filtered = GammaCorrection(filtered, gamma_rcp);

      scanline[i] = ConvertColorToYUV(filtered);
    }

    // Flipper clamps the border colors
    scanline[0] = scanline[1];
    scanline[right + 1] = scanline[right];

    // And Downsample them to 4:2:2
    for (int i = 1, x = left; x < right; i += 2, x += 2)
    {
      // YU pixel
      src_ptr[x].Y = scanline[i].Y + 16;
      // we mix our color differences in 10 bit space so it will round more accurately
      // U[i] = 1/4 * U[i-1] + 1/2 * U[i] + 1/4 * U[i+1]
      src_ptr[x].UV = 128 + ((scanline[i - 1].U + (scanline[i].U << 1) + scanline[i + 1].U) >> 2);

      // YV pixel
      src_ptr[x + 1].Y = scanline[i + 1].Y + 16;
      // V[i] = 1/4 * V[i-1] + 1/2 * V[i] + 1/4 * V[i+1]
      src_ptr[x + 1].UV =
          128 + ((scanline[i - 1].V + (scanline[i].V << 1) + scanline[i + 1].V) >> 2);
    }
    src_ptr += memory_stride;
  }

  auto dest_rect = EFBRectangle{source_rect.left, source_rect.top, source_rect.right,
                                static_cast<int>(static_cast<float>(source_rect.bottom) * y_scale)};

  const std::size_t destination_size = dest_rect.GetWidth() * dest_rect.GetHeight() * 2;
  static std::vector<yuv422_packed> destination;
  destination.resize(dest_rect.GetWidth() * dest_rect.GetHeight());

  SW::CopyRegion(source.data(), source_rect, destination.data(), dest_rect);

  memcpy(xfb_in_ram, destination.data(), destination_size);
}

bool ZCompare(u16 x, u16 y, u32 z)
{
  u32 offset = GetDepthOffset(x, y);
  u32 depth = GetPixelDepth(offset);

  bool pass;

  switch (bpmem.zmode.func)
  {
  case ZMode::NEVER:
    pass = false;
    break;
  case ZMode::LESS:
    pass = z < depth;
    break;
  case ZMode::EQUAL:
    pass = z == depth;
    break;
  case ZMode::LEQUAL:
    pass = z <= depth;
    break;
  case ZMode::GREATER:
    pass = z > depth;
    break;
  case ZMode::NEQUAL:
    pass = z != depth;
    break;
  case ZMode::GEQUAL:
    pass = z >= depth;
    break;
  case ZMode::ALWAYS:
    pass = true;
    break;
  default:
    pass = false;
    ERROR_LOG(VIDEO, "Bad Z compare mode %i", (int)bpmem.zmode.func);
  }

  if (pass && bpmem.zmode.updateenable)
  {
    SetPixelDepth(offset, z);
  }

  return pass;
}

u32 GetPerfQueryResult(PerfQueryType type)
{
  return perf_values[type];
}

void ResetPerfQuery()
{
  perf_values = {};
}

void IncPerfCounterQuadCount(PerfQueryType type)
{
  // NOTE: hardware doesn't process individual pixels but quads instead.
  // Current software renderer architecture works on pixels though, so
  // we have this "quad" hack here to only increment the registers on
  // every fourth rendered pixel
  static u32 quad[PQ_NUM_MEMBERS];
  if (++quad[type] != 3)
    return;
  quad[type] = 0;
  ++perf_values[type];
}
}
