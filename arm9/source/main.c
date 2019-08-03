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

#include "socket.h"
#include "in.h"
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "InterruptsARMCores_h.h"
#include "ipcfifoTGDSUser.h"
#include "ff.h"
#include "memoryHandleTGDS.h"
#include "reent.h"
#include "sys/types.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"
#include "consoleTGDS.h"
#include "utilsTGDS.h"

#include "devoptab_devices.h"
#include "fatfslayerTGDS.h"
#include "usrsettingsTGDS.h"

#include "videoTGDS.h"
#include "keypadTGDS.h"
#include "dswnifi_lib.h"

//zlib
const char hello[] = "if you see this zlib is working nicely!! ";
uLong len = (uLong)strlen(hello)+1;
static const char* myVersion = ZLIB_VERSION;

//Coto, Note: this example requires that you know the compressed buffer size and the uncompressed buffer size previous any zlib operation.
// for anything else check the zlib.h file/documentation.

int main(int _argc, sint8 **_argv) {
	
	/*			TGDS 1.5 Standard ARM9 Init code start	*/
	bool project_specific_console = false;	//set default console or custom console: default console
	GUI_init(project_specific_console);
	GUI_clear();
	
	sint32 fwlanguage = (sint32)getLanguage();
	
	printf("              ");
	printf("              ");
	
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
	/*			TGDS 1.5 Standard ARM9 Init code end	*/
	
	//Check zlib version
	if (zlibVersion()[0] != myVersion[0]){
		printf("incompatible zlib version\n");
	} 
	else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0){
		printf("warning: different zlib version\n");
	}
	
	while (1)
	{
		scanKeys();
		//compress / decompress trigger
		if(keysPressed() & KEY_LEFT){
			int res;
			static Byte *compr, *uncompr;

			uLong comprLen = (1024*20); //20KB
			uLong uncomprLen = comprLen;

			//create storage buffer that holds compressed data
			compr    = (Byte*)malloc((uInt)comprLen);
			uncompr  = (Byte*)malloc((uInt)uncomprLen);

			//compress
			res = compress(compr,&comprLen,(const Bytef*)hello,len);
			switch(res){
				case(Z_OK):
					printf("stream compress status: OK! "); //(%x) BUFFER-> (add:(%x) sz:(%x))",Z_OK,(unsigned int)&compr,(unsigned int)comprLen);
				break;
				case(Z_STREAM_END):
					printf("stream compress status: error, unexpected end! (%x)",Z_STREAM_END);
				break;
				case(Z_NEED_DICT):
					printf("stream compress status: error, need dictionary! (%x)",Z_NEED_DICT);
				break;
				case(Z_ERRNO):
					printf("stream compress status: error, cant open file! (%x)",Z_ERRNO);
				break;
				case(Z_STREAM_ERROR):
					printf("stream compress status: error, corrupted source stream! (%x)",Z_STREAM_ERROR);
				break;
				case(Z_DATA_ERROR):
					printf("stream compress status: error, data not zlib ! (%x)",Z_DATA_ERROR);
				break;
				case(Z_MEM_ERROR):
					printf("stream compress status: error, memory source corrupted! (%x)",Z_MEM_ERROR);
				break;
				case(Z_BUF_ERROR):
					printf("stream compress status: error, not enough room in the output buffer! (%x)",Z_BUF_ERROR);
				break;
			}

			//decompress
			res=uncompress(uncompr,&uncomprLen,compr,comprLen);

			switch(res){
				case(Z_OK):
					printf("stream decomp status: OK! (%x) ",Z_OK);
				break;
				case(Z_STREAM_END):
					printf("stream decomp status: error, unexpected end! (%x)",Z_STREAM_END);
				break;
				case(Z_NEED_DICT):
					printf("stream decomp status: error, need dictionary! (%x)",Z_NEED_DICT);
				break;
				case(Z_ERRNO):
					printf("stream decomp status: error, cant open file! (%x)",Z_ERRNO);
				break;
				case(Z_STREAM_ERROR):
					printf("stream decomp status: error, corrupted source stream! (%x)",Z_STREAM_ERROR);
				break;
				case(Z_DATA_ERROR):
					printf("stream decomp status: error, data not zlib ! (%x)",Z_DATA_ERROR);
				break;
				case(Z_MEM_ERROR):
					printf("stream decomp status: error, memory source corrupted! (%x)",Z_MEM_ERROR);
				break;
				case(Z_BUF_ERROR):
					printf("stream decomp status: error, not enough room in the output buffer! (%x)",Z_BUF_ERROR);
				break;
			}

			//printf("uncompressed BUFFER: (addr: (%x) sz: (%x)) ",(int)&uncompr,(unsigned int)uncomprLen);
			volatile uint8 bufuncomspafe[512];
			memcpy((uint8*)&bufuncomspafe[0],(uint8*)uncompr,uncomprLen);
			printf("%s",(char*)&bufuncomspafe[0]);

			free(compr);
			free(uncompr);

		}
			
		if (keysPressed() & KEY_B){
			GUI_clear();
		}
		
		IRQVBlankWait();
	}

}
