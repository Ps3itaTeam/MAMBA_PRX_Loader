/*
	==============================================================
	
	MAMBA/PRX Autoloader by NzV
	
	Load MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot using New Core as loader.

	-The generated "sys_init_osd.self" is fw dependent (CEX an DEX version include in the same self)!
	-The generated "sys_init_osd.self" should replace sys_init_osd.self in /dev_flash/sys/internal/
	 who need to be previously renamed sys_init_osd_orig.self
	 
	===============================
				[FLAGS]
	===============================
	
	Flags can be placed in /dev_usb000/core_flags/ or /dev_usb001/core_flags/ or /dev_hdd0/tmp/core_flags/

	"failsafe"  	Start in normal mode (MAMBA and VSH plugins not loaded)
	"mamba_off"   	Don't load  MAMBA (PRX Loader will be used instead of MAMBA to load VSH plugins)
	"noplugins"   	Don't load  VSH plugins at boot
	"verbose"   	Enable log and write it in /dev_usb000 or /dev_usb001 or /dev_hdd0
	
	===============================
			[VSH PLUGINS]
	===============================
	
	-If flag "mamba_off" is not set VSH Plugins will be loaded from file /dev_hdd0/mamba_plugins.txt with MAMBA
	-Else they will be loaded from file /dev_hdd0/prx_plugins.txt with prx loader
	
	==============================================================

	This program is based in:
	
	- New Core by MiralaTijera and changes by Estwald 
	
	- MAMBA by Estwald and unofficial updates by _NZV_ (GetVSHProcess, PS3M_API)
	  Some part of code for load MAMBA payload come from Iris Manager and his fork (IRISMAN, GAMESONIC MANAGER, MANAGUNZ)
	
	- PRX Loader by User and unofficial updates by _NZV_ (GetVSHProcess)
	  Some part of code for load PRX Loader payload come from "payload autoloader" by KW
	
	==============================================================
	
	(c) 2013 MiralaTijera <www.elotrolado.net> (Original Core)
	(c) 2013 Estwald <www.elotrolado.net> (New Core)

	"New Core" is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	"New Core" is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with "New Core". If not, see <http://www.gnu.org/licenses/>.
	
	==============================================================
*/

#include <stdio.h>
#include <string.h>
#include <ppu-lv2.h>
#include <sys/systime.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "common.h"

//#define DISABLE_MAMBA
//#define DISABLE_VSH_PLUG

#ifndef DISABLE_MAMBA
#include "mamba.h"
#endif

#ifndef DISABLE_VSH_PLUG
#include "vsh_plugins_loader.h"
#endif

#define VERSION_NAME "MAMBA/PRX Autoloader v1.5.0 by NzV"

#define FS_S_IFMT 0170000

//----------------------------------------
//FLAG
//----------------------------------------

int verbose = 0;
int failsafe = 0;
#ifndef DISABLE_MAMBA
int mamba_off = 0;
#endif
#ifndef DISABLE_VSH_PLUG
int noplugins = 0;
#endif

//----------------------------------------
//LOG
//----------------------------------------

#ifdef ENABLE_LOG

int fd_log = -1;

int WriteToLog(char *str)
{
	if(fd_log < 0 ) return FAILED;
	if(!str) return SUCCESS;
    u64 size = strlen(str);
    if(size == 0) return SUCCESS;
    u64 ret_size = 0;
    if(sysLv2FsWrite(fd_log, str, size, &ret_size) || ret_size!=size)
	{
		return FAILED;
	}
    return SUCCESS;
}

void CloseLog()
{
	if (verbose) WriteToLog("-----[END]-----\r\n");
    if (verbose) WriteToLog("---[By NzV]---\r\n");
	verbose = 0;
	if(fd_log >= 0) sysLv2FsClose(fd_log);
    fd_log = -1;
}

int Open_Log(char *file)
{
    if(fd_log >= 0) return -666;
    if(!sysLv2FsOpen(file, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd_log, 0777, NULL, 0))
	{
        sysLv2FsChmod(file, FS_S_IFMT | 0777);
        if(WriteToLog(VERSION_NAME)!=SUCCESS) {CloseLog(); return FAILED;}
        WriteToLog("\r\n-----[LOG]-----\r\n");
        return SUCCESS;
    } 
    fd_log = -1;
	verbose = 0;
    return FAILED;
    
}

#endif

//----------------------------------------
//UTILS
//----------------------------------------


int file_exists(const char *path)
{
    sysFSStat stat;
    int ret = sysLv2FsStat(path, &stat);
    if(ret == SUCCESS && S_ISDIR(stat.st_mode)) return FAILED;
    return ret;
}

int dir_exists(const char *path)
{
    sysFSStat stat;
    int ret = sysLv2FsStat(path, &stat);
    if(ret == SUCCESS && S_ISDIR(stat.st_mode)) return SUCCESS;
    return FAILED;
}

extern int _sys_process_atexitspawn(u32 a, const char *file, u32 c, u32 d, u32 e, u32 f);

int launchself(const char*file)
{
	if(file_exists(file)==SUCCESS)
	{
        return _sys_process_atexitspawn(0, file, 0, 0, 0x3d9, 0x20);
    }
    return FAILED;
		
}

int unlink_secure(void *path)
{	
    if(file_exists(path)==SUCCESS)
	{
        sysLv2FsChmod(path, FS_S_IFMT | 0777);
        return sysLv2FsUnlink(path);
    }
    return FAILED;
}

int sys_fs_mount_ext(char const* deviceName, char const* deviceFileSystem, char const* devicePath, int writeProt, u32* buffer, u32 count) 
{
    lv2syscall8(837, (u64) deviceName, (u64) deviceFileSystem, (u64) devicePath, 0ULL, (u64) writeProt, 0ULL, (u64) buffer, (u64) count);
    return_to_user_prog(int);
}

int try_mount_usb0()
{   
    int ret;
    u32 usb_buf[64]; // 0xA is not sufficient!
    char dev[64];
    int n;
    strncpy(dev, "CELL_FS_IOS:USB_MASS_STORAGE000", 64);  

    for(n = 0; n < 5; n++) //5*3 seconds
	{
        memset(usb_buf, 0, sizeof(usb_buf));
        ret = sys_fs_mount_ext(dev, "CELL_FS_FAT", "/dev_usb000", 0, usb_buf, 0);
        if(ret == (int) 0x8001002B) sysSleep(3);
        else break;
    }
    return ret;
}

//----------------------------------------
//INIT FLAG
//----------------------------------------

void InitFlag()
{
	failsafe = verbose = 0;
	#ifdef ENABLE_LOG
	//Flag Verbose
    if(file_exists("/dev_usb000/core_flags/verbose")==0)
	{
        verbose = 1;
		Open_Log("/dev_usb000/new_core.log");
    }
	else if(file_exists("/dev_usb001/core_flags/verbose")==0)
	{
        verbose = 1;
		Open_Log("/dev_usb001/new_core.log");
    }
	else if(file_exists("/dev_usb001/core_flags/verbose")==0)
	{
        verbose = 1;
		Open_Log("/dev_hdd0/tmp/new_core.log");
    }
	#endif
	//Flag Failsafe
    if(file_exists("/dev_usb000/core_flags/failsafe")==0) failsafe = 1;
	else if(file_exists("/dev_usb001/core_flags/failsafe")==0) failsafe = 1;
	else if(file_exists("/dev_hdd0/tmp/core_flags/failsafe")==0) failsafe = 1;
	#ifndef DISABLE_MAMBA
	//Flag MAMBA
	mamba_off = 0;
    if(file_exists("/dev_usb000/core_flags/mamba_off")==0) mamba_off = 1;
	else if(file_exists("/dev_usb001/core_flags/mamba_off")==0) mamba_off = 1;
	else if(file_exists("/dev_hdd0/tmp/core_flags/mamba_off")==0) mamba_off = 1;
	#endif
	#ifndef DISABLE_VSH_PLUG
	//Flag VSH Plugins
	noplugins = 0;
	if(file_exists("/dev_usb000/core_flags/noplugins")==0) noplugins = 1;
	else if(file_exists("/dev_usb001/core_flags/noplugins")==0) noplugins = 1;
	else if(file_exists("/dev_hdd0/tmp/core_flags/noplugins")==0) noplugins = 1;
	#endif
	//Log
	#ifdef ENABLE_LOG
	
	if (verbose && (failsafe || mamba_off || noplugins )) WriteToLog("[FLAGS]\r\n");
	if (failsafe)
	{
		if (verbose) WriteToLog("Success: Flag failsafe detected\r\n");	
	}
	#ifndef DISABLE_MAMBA
	if (mamba_off)
	{
		if (verbose) WriteToLog("Success: Flag mamba_off detected\r\n");	
	}
	#endif
	#ifndef DISABLE_VSH_PLUG
	if (noplugins)
	{
		if (verbose) WriteToLog("Success: Flag noplugins detected\r\n");	
	}
	#endif
	#endif
}

//----------------------------------------
//MAIN
//----------------------------------------

s32 main(s32 argc, const char* argv[]) 
{
    //Launch VSH at first to avoid problems with the PAD
    if(launchself("/dev_flash/sys/internal/sys_init_osd_orig.self")!=SUCCESS)
	{
        if(launchself("/dev_flash/vsh/module/vsh.self")!=SUCCESS)
		{
            //EMERGENCY MODE
			//Try to mount /dev_usb000 (wait 15sec)
            try_mount_usb0();
            // Try to launch /dev_usb000/emergency.self
			if (dir_exists("/dev_usb000"))
			{
				#ifdef ENABLE_LOG
				verbose = 1;
				Open_Log("/dev_usb000/new_core.log");
				if (verbose) WriteToLog("Error: sys_init_osd_orig.self and vsh.self not found or corrupt\r\n");
				#endif
				
				if(sys_fs_mount_ext("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_rewrite", 0, NULL, 0)==SUCCESS)
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Success: /dev_rewrite mounted\r\n");
					#endif
				}
				#ifdef ENABLE_LOG
				else
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Error: /dev_rewrite not mounted\r\n");
					#endif
				}
				#endif
				if(launchself("/dev_usb000/emergency.self")!=SUCCESS)
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Error: Launch /dev_usb000/emergency.self failed\r\n");
					CloseLog();
					#endif
					unlink_secure("/dev_hdd0/tmp/turnoff");
					{lv2syscall4(379,0x1100,0,0,0);} //Power off PS3
					goto exit_new_core;
				}
				#ifdef ENABLE_LOG
				if (verbose) WriteToLog("Error: /dev_usb000/emergency.self launched\r\n");
				#endif
			}
			goto exit_new_core;
		}
		else  goto exit_new_core;
    }
    
    //Wait to VSH.SELF mount /dev_usb000 if is connected (10sec)
    int n = 0;
	for( n = 0; n < 5; n++)
	{
        sysSleep(2);
        if (dir_exists("/dev_usb000") == SUCCESS) break;
        if (dir_exists("/dev_usb001") == SUCCESS) break;
    }
	
	//INIT FLAG
	InitFlag();
	
	//FAILSAFE
	if (failsafe) goto exit_new_core;

	//COBRA
	if (is_cobra() == SUCCESS) 
	{
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog("Error: Running in COBRA mode!\r\n");
		#endif
		goto exit_new_core;
	}
	
	#ifndef DISABLE_MAMBA
	//MAMBA
	if (!mamba_off)
	{
		if (load_mamba(verbose) == SUCCESS)
		{
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog(GetMambaLog());
			#endif
		}
		else
		{
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog(GetMambaLog());
			#endif
			goto exit_new_core;
		}
	}	
	#endif
	
	#ifndef DISABLE_VSH_PLUG
	//VSH PLUGINS
	if (!noplugins)
	{
		load_vsh_plugins(verbose);
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog(GetVSHPlugLog());
		#endif
	}
	#endif
	
	//END
exit_new_core:	
    #ifdef ENABLE_LOG
	CloseLog();
	#endif	
	return SUCCESS;
}

