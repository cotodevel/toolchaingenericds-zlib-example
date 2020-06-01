/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA
*/

#include "main.h"
#include "biosTGDS.h"
#include "spifwTGDS.h"
#include "posixHandleTGDS.h"

//These buffers are project specific for ARM7 WAV SoundStream
u16 strpcmL0Buf[WAV_READ_SIZE];
u16 strpcmL1Buf[WAV_READ_SIZE];
u16 strpcmR0Buf[WAV_READ_SIZE];
u16 strpcmR1Buf[WAV_READ_SIZE];

//---------------------------------------------------------------------------------
int main(int _argc, sint8 **_argv) {
//---------------------------------------------------------------------------------
	/*			TGDS 1.5 Standard ARM7 Init code start	*/
	installWifiFIFO();		
	
	//wait for VRAM D to be assigned from ARM9->ARM7 (ARM7 has load/store on byte/half/words on VRAM)
	while (!(*((vuint8*)0x04000240) & 0x2));
	
	int argBuffer[MAXPRINT7ARGVCOUNT];
	memset((unsigned char *)&argBuffer[0], 0, sizeof(argBuffer));
	writeDebugBuffer7("TGDS ARM7.bin Boot OK!", 0, (int*)&argBuffer[0]);
	
	/*			TGDS 1.5 Standard ARM7 Init code end	*/
	
    while (1) {
		handleARM7SVC();	/* Do not remove, handles TGDS services */
		IRQVBlankWait();
	}
	return 0;
}

//Custom Button Mapping Handler implementation: IRQ Driven
void CustomInputMappingHandler(uint32 readKeys){
	
}

//Project specific: ARM7 Setup for TGDS sound stream
void initSoundStreamUser(u32 srcFmt){
	if(srcFmt == SRC_WAV){
		//Buffers must be provided here. 
		//Format: s16 buffer[WAV_READ_SIZE];
		strpcmL0 = (s16*)&strpcmL0Buf[0];
		strpcmL1 = (s16*)&strpcmL1Buf[0];
		strpcmR0 = (s16*)&strpcmR0Buf[0];
		strpcmR1 = (s16*)&strpcmR1Buf[0];
		
		// clear vram d bank to not have sound leftover
		int i = 0;
		
		for(i=0;i<(WAV_READ_SIZE);++i)
		{
			strpcmL0[i] = 0;
		}
		
		for(i=0;i<(WAV_READ_SIZE);++i)
		{
			strpcmR0[i] = 0;
		}
	}
}