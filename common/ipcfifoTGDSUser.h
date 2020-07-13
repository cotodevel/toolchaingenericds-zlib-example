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

//TGDS required version: IPC Version: 1.3

//IPC FIFO Description: 
//		getsIPCSharedTGDS() 		= 	Access to TGDS internal IPC FIFO structure. 		(ipcfifoTGDS.h)
//		getsIPCSharedTGDSSpecific()	=	Access to TGDS Project (User) IPC FIFO structure	(ipcfifoTGDSUser.h)

#ifndef __ipcfifoTGDSUser_h__
#define __ipcfifoTGDSUser_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "ipcfifoTGDS.h"
#include "dswnifi.h"
#include "utilsTGDS.h"

//---------------------------------------------------------------------------------
typedef struct sIPCSharedTGDSSpecific{
//---------------------------------------------------------------------------------
	uint32 frameCounter7;	//VBLANK counter7
	uint32 frameCounter9;	//VBLANK counter9
}  IPCSharedTGDSSpecific	__attribute__((aligned (4)));

//TGDS Memory Layout ARM7/ARM9 Cores
#define TGDS_ARM7_MALLOCSTART (u32)(0x06000000)
#define TGDS_ARM7_MALLOCSIZE (int)(112*1024)
#define TGDSDLDI_ARM7_ADDRESS (u32)(TGDSDLDI_ARM7_MALLOCSTART + TGDSDLDI_ARM7_MALLOCSIZE)

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct sIPCSharedTGDSSpecific* getsIPCSharedTGDSSpecific();
//NOT weak symbols : the implementation of these is project-defined (here)
extern void HandleFifoNotEmptyWeakRef(uint32 cmd1,uint32 cmd2);
extern void HandleFifoEmptyWeakRef(uint32 cmd1,uint32 cmd2);

#ifdef __cplusplus
}
#endif