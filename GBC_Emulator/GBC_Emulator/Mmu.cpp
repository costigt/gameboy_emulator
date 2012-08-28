#include "Mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

namespace gameboy
{
	MMU::MMU()
	{
		loadBinary("C:\\Roms\\bios.bin",BIOS);
	}

	MMU::~MMU()
	{
		if(m_Rom != NULL)
		{
			delete[] m_Rom;
		}
		delete[] m_Bios;
	}

	void MMU::loadProgram(const char *filename)
	{
		loadBinary(filename,ROM);
		//establish cart details
		//cart type?
		m_CartType = m_Rom[0x0147];
		printf("Cart is 0x%X\n",m_CartType);
		//rom size?
		m_RomSize = m_Rom[0x0148];
		printf("Rom is 0x%X\n",m_RomSize);
		//ram size?
		m_RamSize = m_Rom[0x0149];
		printf("Ram is 0x%X\n",m_RamSize);
		//is CGB?
		if(m_Rom[0x0143] == 0x80)
		{
			m_CGBFlag = true;
			printf("Color\n");
		}
		else
		{
			m_CGBFlag = false;
			printf("Original\n");
		}

		printf("Title is ");
		for(int i = 0 ; i < 16 ; i++)
		{
			printf("%c",m_Rom[i+0x0134]);
		}
		printf("\n");
	}

	void MMU::loadBinary(const char *filename,BinaryType type)
	{
		FILE *program;
		char * buffer;
		long lSize;
		size_t result;

		program = fopen(filename,"rb");
		if(program == NULL)
		{
			fputs("File error",stderr);
			exit(1);
		}

		//obtain file size
		fseek(program,0,SEEK_END);
		lSize = ftell(program);
		rewind(program);

		//allocate memory to contain the whole file
		buffer = (char*) malloc(sizeof(char)*lSize);
		if(buffer == NULL)
		{
			fputs("Memory error",stderr);
			exit(2);
		}

		//copy the file into the buffer
		result = fread(buffer,1,lSize,program);
		if(result != lSize)
		{
			fputs("Reading error",stderr);
			exit(3);
		}

		fclose(program);

		if(type == BIOS)
		{
			//load bios into memory
			m_Bios = new BYTE[lSize];
			for(int i = 0 ; i < lSize ; ++i)
			{
				m_Bios[i] = buffer[i];
			}
		}
		else
		{
			//load program into memory
			m_Rom = new BYTE[lSize];
			for(int i = 0 ; i < lSize ; ++i)
			{
				m_Rom[i] = buffer[i];
			}
		}

		free(buffer);
	}

	void MMU::write(WORD address,BYTE value)
	{
		switch(address & 0xF000)
		{
			//RAM bank enable
		case 0x0000:case 0x1000:
			m_RamEnable = (value & 0x000F == 0xA)?true:false;
			break;
			//ROM bank number
		case 0x2000:case 0x3000:
			//MBC automatically turns 0x00 to 0x01
			if(value == 0x00)
			{
				m_RomBank = 0x01;
			}
			//set lower 5 bits of rom bank number
			else
			{
				//clear lower 5 bits
				m_RomBank &= 0xE0;
				//set lower 5 bits
				m_RomBank |= value & 0x1F;
			}
			break;
			//RAM bank number or Upper bits of ROM bank number
		case 0x4000:case 0x5000:
			//set bits 5-6
			if(m_RomMode)
			{
				//clear bit 5 and 6
				m_RomBank &= 0x9F;
				//set bit 5 and 6
				m_RomBank |= (value && 0x03) << 5;
			}
			else
			{
				m_ERamBank = value;
			}
			break;
			//ROM/RAM Mode Select
		case 0x6000:case 0x7000:
			m_RomMode = (value==00)? true:false;
			break;
			//VRAM
		case 0x8000:case 0x9000:
			m_Vram[(address-0x8000) + CGB_VRAM_BANK*m_IO[0x4F]] = value;
			break;
			//ERAM
		case 0xA000:case 0xB000:
			if(m_RomMode)
			{

			}
			else
			{

			}
			break;
			//WRAM bank 0
		case 0xC000:
			m_Wram[address-0xC000] = value;
			break;
			//WRAM bank 1-7
		case 0xD000:
			m_Wram[(address-0xC000) + CGB_WRAM_BANK*m_WRamBank] = value;
			break;
			//ECHO Ram
		case 0xE000:
			m_Wram[address-0xE000] = value;
			break;
			//Upper ECHO ram,OAM,Hardware Registers,Zero page,IEF
		case 0xF000:
			switch(address & 0x0F00)
			{
				//Upper ECHO ram
			case 0x000:case 0x100:case 0x200:case 0x300:case 0x400:
			case 0x500:case 0x600:case 0x700:case 0x800:case 0x900:
			case 0xA00:case 0xB00:case 0xC00:case 0xD00:
				m_Wram[address-0xE000] = value;
				break;
				//OAM
			case 0xE00:
				m_Oam[address-0xFE00] = value;
				break;
				//Hardware Registers,Zero page, IEF
			case 0xF00:
				switch(address & 0x00F0)
				{
					//IO registers
				case 0x00:case 0x10:case 0x20:case 0x30:case 0x40:case 0x50:
				case 0x60:case 0x70:
					m_IO[address-0xFF00] = value;
					break;
					//Zero Page
				case 0x80:case 0x90:case 0xA0:case 0xB0:case 0xC0:case 0xD0:
				case 0xE0:
					m_ZeroPage[address-0xFF80] = value;
					break;
				//Zero Page, IEF
				case 0xF0:
					switch(address & 0x000F)
					{
					case 0x0:case 0x1:case 0x2:case 0x3:case 0x4:case 0x5:
					case 0x6:case 0x7:case 0x8:case 0x9:case 0xA:case 0xB:
					case 0xC:case 0xD:case 0xE:
						m_ZeroPage[address-0xFF80] = value;
						break;
					case 0xF:
						m_IEF = value;
						break;
					}
				}
			}
		}
	}

	BYTE MMU::read(WORD address)
	{
		switch(address & 0xF000)
		{
			//BIOS
		case 0x0000:
			if (m_BiosEnable && address <= 0x00FF)
			{
				return m_Bios[address];
			}
			//ROM bank 0
		case 0x1000:case 0x2000:case 0x3000:
			return m_Rom[address];
			//ROM bank 1-7
		case 0x4000:case 0x5000:case 0x6000:case 0x7000:
			if(m_RomBank)
			{
				return m_Rom[address + CGB_ROM_BANK*(1-m_RomBank)];
			}
			else
			{
				//if Rom mode is disabled we cannot acces the upper rom banks
				return m_Rom[address + CGB_ROM_BANK*(1-(m_RomBank&0x1F))];
			}
			//VRAM
		case 0x8000:case 0x9000:
			return m_Vram[(address-0x8000) + CGB_VRAM_BANK*m_IO[0x4F]];
			//ERAM
		case 0xA000:case 0xB000:
			if(m_RomMode)
			{

			}
			else
			{

			}
			return 0x0000;
			//WRAM bank 0
		case 0xC000:
			return m_Wram[address-0xC000];
			//WRAM bank 1-7
		case 0xD000:
			return m_Wram[(address-0xC000) + CGB_WRAM_BANK*(1-m_WRamBank)];
			//ECHO Ram
		case 0xE000:
			return m_Wram[address-0xE000];
			//Upper ECHO ram,OAM,Hardware Registers,Zero page,IEF
		case 0xF000:
			switch(address & 0x0F00)
			{
				//Upper ECHO ram
			case 0x000:case 0x100:case 0x200:case 0x300:case 0x400:
			case 0x500:case 0x600:case 0x700:case 0x800:case 0x900:
			case 0xA00:case 0xB00:case 0xC00:case 0xD00:
				return m_Wram[address-0xE000];
				//OAM
			case 0xE00:
				return m_Oam[address-0xFE00];
				//Hardware Registers,Zero page, IEF
			case 0xF00:
				switch(address & 0x00F0)
				{
					//IO registers
				case 0x00:case 0x10:case 0x20:case 0x30:case 0x40:case 0x50:
				case 0x60:case 0x70:
					return m_IO[address-0xFF00];
					//Zero Page
				case 0x80:case 0x90:case 0xA0:case 0xB0:case 0xC0:case 0xD0:
				case 0xE0:
					return m_ZeroPage[address-0xFF80];
				//Zero Page, IEF
				case 0xF0:
					switch(address & 0x000F)
					{
					case 0x0:case 0x1:case 0x2:case 0x3:case 0x4:case 0x5:
					case 0x6:case 0x7:case 0x8:case 0x9:case 0xA:case 0xB:
					case 0xC:case 0xD:case 0xE:
						return m_ZeroPage[address-0xFF80];
					case 0xF:
						return m_IEF;
					}
				}
			}
			default:
				return 0x0000;
		}
	}

	void MMU::reset()
	{

	}

	void MMU::setBiosEnabled(bool value)
	{

	}

	bool MMU::getBiosEnabled()
	{
		return m_BiosEnable;
	}

	void MMU::initialiseAfterBios()
	{

	}
}