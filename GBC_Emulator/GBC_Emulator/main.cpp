#include <stdio.h>
#include "Mmu.h"

gameboy::MMU mmu;

int main(int argc, char** argv)
{
	mmu.loadProgram("C:\\Roms\\game.gb");
	printf("Compiled\n");
	getchar();
	return 0;
}