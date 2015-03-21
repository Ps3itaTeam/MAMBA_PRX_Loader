/*
	==============================================================
	
	MAMBA/PRX Autoloader (c) 2015 NzV
	
	Load MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot using New Core.

	The generated "sys_init_osd.self" should replace sys_init_osd.self in /dev_flash/sys/internal/
	who need to be previously renamed as sys_init_osd_orig.self
	 
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
	
	If flag "mamba_off" is not set VSH Plugins will be loaded from file /dev_hdd0/mamba_plugins.txt with MAMBA
	else they will be loaded from file /dev_hdd0/prx_plugins.txt with prx loader
	
	==============================================================
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

#include "common.h"
#include "lv2_utils.h"
#include "mamba_prx_loader.h"

//----------------------------------------
//UTILS
//----------------------------------------

extern int _sys_process_atexitspawn(u32 a, const char *file, u32 c, u32 d, u32 e, u32 f);

int launchself(const char*file)
{
	if(file_exists(file)==SUCCESS)
	{
        return _sys_process_atexitspawn(0, file, 0, 0, 0x3d9, 0x20);
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

int failsafe = 0;

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
	//Log
	#ifdef ENABLE_LOG
	if (failsafe && verbose) WriteToLog("Success: Flag failsafe detected");	
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
				if (verbose) WriteToLog("Error: sys_init_osd_orig.self and vsh.self not found or corrupt");
				#endif
				
				if(sys_fs_mount_ext("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_rewrite", 0, NULL, 0)==SUCCESS)
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Success: /dev_rewrite mounted");
					#endif
				}
				#ifdef ENABLE_LOG
				else
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Error: /dev_rewrite not mounted");
					#endif
				}
				#endif
				if(launchself("/dev_usb000/emergency.self")!=SUCCESS)
				{
					#ifdef ENABLE_LOG
					if (verbose) WriteToLog("Error: Launch /dev_usb000/emergency.self failed");
					CloseLog();
					#endif
					unlink_secure("/dev_hdd0/tmp/turnoff");
					{lv2syscall4(379,0x1100,0,0,0);} //Power off PS3
					goto exit_new_core;
				}
				#ifdef ENABLE_LOG
				if (verbose) WriteToLog("Error: /dev_usb000/emergency.self launched");
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

	//COBRA ENABLED ?
	if (is_cobra() == SUCCESS) 
	{
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog("Error: Running in COBRA mode!");
		#endif
		goto exit_new_core;
	}
	
	//CLEAR STAGE1 IF COBRA CFW
	if ((dir_exists("/dev_flash/rebug/cobra") == SUCCESS) || (dir_exists("/dev_flash/habib/cobra") == SUCCESS) ||
		(file_exists("/dev_flash/sys/stage2_disabled.bin") == SUCCESS) || (file_exists("/dev_flash/sys/stage2.bak") == SUCCESS))
		{
			int i;
			for(i=0;i<(512/8);i++)
					lv2poke(0x80000000007F0000ULL + (i * 8), 0ULL);
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog("Success: Cleared COBRA stage1");
			#endif
		}
	
	//MAMBA/PRX LOADER
	mamba_prx_loader(0,0);
	
	//END
exit_new_core:	
    #ifdef ENABLE_LOG
	CloseLog();
	#endif	
	return SUCCESS;
}

