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
#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <string.h>
#include <stdlib.h>

#include "zlib.h"
#include "gui_console_connector.h"
#include "dswnifi_lib.h"
#include "TGDSLogoLZSSCompressed.h"
#include "biosTGDS.h"
#include "global_settings.h"
#include "posixHandleTGDS.h"
#include "TGDSMemoryAllocator.h"
#include "ipcfifoTGDSUser.h"
#include "fatfslayerTGDS.h"
#include "utilsTGDS.h"
#include "fileBrowse.h"

char curChosenBrowseFile[MAX_TGDSFILENAME_LENGTH+1];

static const char* myVersion = ZLIB_VERSION;

static inline void menuShow(){
	clrscr();
	printf("     ");
	printf("     ");
	printf("toolchaingenericds-zlib-example ");
	printf("Button (Start): File browser ");
	printf("    Button (A) Unzip file ");
	printf("(Select): This menu ");
	printf("(R): Shutdown DS. >%d", TGDSPrintfColor_Red);
	printf("Available heap memory: %d", getMaxRam());
	printarm7DebugBuffer();
	
	//Check zlib version
	if (zlibVersion()[0] != myVersion[0]){
		printf("incompatible zlib version. Turn off NDS. ");
		while(1==1){
			handleARM9SVC();	/* Do not remove, handles TGDS services */
			IRQVBlankWait();
		}
	} 
	else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0){
		printf("warning: different zlib version. Turn off NDS. ");
		while(1==1){
			handleARM9SVC();	/* Do not remove, handles TGDS services */
			IRQVBlankWait();
		}
	}
	else{
		printf("ZLIB Version: %s", ZLIB_VERSION);
	}
}

int main(int argc, char argv[argvItems][MAX_TGDSFILENAME_LENGTH]) {
	
	/*			TGDS 1.6 Standard ARM9 Init code start	*/
	bool isTGDSCustomConsole = false;	//set default console or custom console: default console
	GUI_init(isTGDSCustomConsole);
	GUI_clear();
	bool isCustomTGDSMalloc = false;
	setTGDSMemoryAllocator(getProjectSpecificMemoryAllocatorSetup(TGDS_ARM7_MALLOCSTART, TGDS_ARM7_MALLOCSIZE, isCustomTGDSMalloc));
	sint32 fwlanguage = (sint32)getLanguage();
	#ifdef ARM7_DLDI
	setDLDIARM7Address((u32 *)TGDSDLDI_ARM7_ADDRESS);	//Required by ARM7DLDI!
	#endif
	int ret=FS_init();
	if (ret == 0)
	{
		printf("FS Init ok.");
	}
	else if(ret == -1)
	{
		printf("FS Init error.");
	}
	switch_dswnifi_mode(dswifi_idlemode);
	asm("mcr	p15, 0, r0, c7, c10, 4");
	flush_icache_all();
	flush_dcache_all();
	/*			TGDS 1.6 Standard ARM9 Init code end	*/
	
	//Show logo
	RenderTGDSLogoMainEngine((uint8*)&TGDSLogoLZSSCompressed[0], TGDSLogoLZSSCompressed_size);
	
	//Remove logo and restore Main Engine registers
	//restoreFBModeMainEngine();
	
	menuShow();
	
	while (1)
	{
		scanKeys();
		if (keysPressed() & KEY_START){
			char startPath[MAX_TGDSFILENAME_LENGTH+1];
			strcpy(startPath,"/");
			while( ShowBrowser((char *)startPath, (char *)&curChosenBrowseFile[0]) == true ){	//as long you keep using directories ShowBrowser will be true
				
			}
			
			clrscr();
			printf(" -- ");
			printf("Unzip start.");
			char unzippedFile[MAX_TGDSFILENAME_LENGTH+1];
			load_gz((char*)curChosenBrowseFile, (char*)unzippedFile);
			printf("File:%s unzipped into %s", curChosenBrowseFile, unzippedFile);
			printf("Press A to exit");
			
			while(1==1){
				scanKeys();
				if(keysPressed()&KEY_A){
					break;
				}
			}
			
			scanKeys();
			while((keysPressed() & KEY_A) || (keysPressed() & KEY_START)){
				scanKeys();
			}
			menuShow();
		}
		
		if (keysPressed() & KEY_SELECT){
			menuShow();
			while(keysPressed() & KEY_SELECT){
				scanKeys();
			}
		}
		
		if (keysPressed() & KEY_R){
			printf("NDS should shutdown now. Otherwise unimplemented.>%d", TGDSPrintfColor_Red);
			shutdownNDSHardware();
			while(keysPressed() & KEY_R){
				scanKeys();
			}
		}
		
		handleARM9SVC();	/* Do not remove, handles TGDS services */
		IRQVBlankWait();
	}

}
