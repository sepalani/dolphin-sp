// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Core/HW/Sram.h"

#include "Common/CommonTypes.h"
#include "Common/File.h"
#include "Common/Logging/Log.h"
#include "Common/Swap.h"
#include "Core/ConfigManager.h"

// english
const SRAM sram_dump = {{0x00, 0x2C, 0xFF, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x44, 0x4F,
                         0x4C, 0x50, 0x48, 0x49, 0x4E, 0x53, 0x4C, 0x4F, 0x54, 0x41, 0x44,
                         0x4F, 0x4C, 0x50, 0x48, 0x49, 0x4E, 0x53, 0x4C, 0x4F, 0x54, 0x42,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x6E, 0x6D, 0x00, 0x00, 0x00, 0x00}};

#if 0
// german
const SRAM sram_dump_german = {{
	0x1F, 0x66,
	0xE0, 0x96,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x04, 0xEA, 0x19, 0x40,
	0x00,
	0x00,
	0x01,
	0x3C,
	0x12, 0xD5, 0xEA, 0xD3, 0x00, 0xFA, 0x2D, 0x33, 0x13, 0x41, 0x26, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00,
	0x00,
	0x84, 0xFF,
	0x00, 0x00,
	0x00, 0x00
}};
#endif

void InitSRAM()
{
  File::IOFile file(SConfig::GetInstance().m_strSRAM, "rb");
  if (file)
  {
    if (!file.ReadArray(&g_SRAM, 1))
    {
      ERROR_LOG(EXPANSIONINTERFACE, "EXI IPL-DEV: Could not read all of SRAM");
      g_SRAM = sram_dump;
    }
  }
  else
  {
    g_SRAM = sram_dump;
  }
}

void SetCardFlashID(const u8* buffer, u8 card_index)
{
  u64 rand = Common::swap64(&buffer[12]);
  u8 csum = 0;
  for (int i = 0; i < 12; i++)
  {
    rand = (((rand * (u64)0x0000000041c64e6dULL) + (u64)0x0000000000003039ULL) >> 16);
    csum += g_SRAM.flash_id[card_index][i] = buffer[i] - ((u8)rand & 0xff);
    rand = (((rand * (u64)0x0000000041c64e6dULL) + (u64)0x0000000000003039ULL) >> 16);
    rand &= (u64)0x0000000000007fffULL;
  }
  g_SRAM.flashID_chksum[card_index] = csum ^ 0xFF;
}

void FixSRAMChecksums()
{
  u16 checksum = 0;
  u16 checksum_inv = 0;
  for (int i = 0x0C; i < 0x14; i += 2)
  {
    int value = (g_SRAM.p_SRAM[i] << 8) + g_SRAM.p_SRAM[i + 1];
    checksum += value;
    checksum_inv += value ^ 0xFFFF;
  }
  g_SRAM.checksum = Common::swap16(checksum);
  g_SRAM.checksum_inv = Common::swap16(checksum_inv);
}
