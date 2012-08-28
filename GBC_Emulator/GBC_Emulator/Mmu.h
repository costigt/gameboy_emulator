#include "DataTypes.h"
namespace gameboy
{
#define CGB_BIOS 256
#define CGB_VRAM 16384
#define CGB_WRAM 32768
#define CGB_OAM 160
#define CGB_ZPAGE 127
#define CGB_IO 128
#define CGB_ROM_BANK 16384
#define CGB_VRAM_BANK 8192
#define CGB_WRAM_BANK 4096
#define CGB_ERAM_BANK 8192
	enum BinaryType {BIOS,ROM};
	class MMU
	{
	public:
		MMU();
		~MMU();
		void loadProgram(const char *filename);
		void write(WORD address, BYTE value);
		BYTE read(WORD address);
		void reset();
		void setBiosEnabled(bool value);
		bool getBiosEnabled();
	private:
		void loadBinary(const char *filename,BinaryType type);
		void initialiseAfterBios();
		bool m_CGBFlag,m_BiosEnable,m_RomMode,m_RamEnable;
		BYTE m_Title[16],m_CartType,m_RomSize,m_RamSize,m_RTC,m_RomBank,m_ERamBank,m_WRamBank;
		BYTE *m_Rom,*m_Bios;
		BYTE m_Vram[CGB_VRAM],m_Wram[CGB_WRAM],m_Oam[CGB_OAM],m_IO[CGB_IO],m_ZeroPage[CGB_ZPAGE],m_IEF;
	};
}