#include <stdio.h>
#include <iostream>
#include "Mmu.h"
#include "DataTypes.h"

using namespace std;

gameboy::MMU mmu;

int main(int argc, char** argv)
{
	mmu.loadProgram("C:\\Roms\\game.gb");

	BYTE value;
	WORD address;
	while(true)
	{
		printf("\nEnter address:");
		scanf("%x",&address);
		printf("\nEnter value:");
		scanf("%x",&value);
		mmu.write(address,value);
	}

	printf("Compiled\n");
	getchar();
	return 0;
}