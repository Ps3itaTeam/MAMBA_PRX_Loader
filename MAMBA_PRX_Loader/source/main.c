/*
	==============================================================
	
	MAMBA/PRX loader By NzV
	
	Load of MAMBA and/or VSH plugins (with MAMBA or PRX Loader) after system boot.

	==============================================================
	
	==============================================================
	
	MAMBA/PRX Autoloader By NzV
	
	Load of MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot.

	==============================================================
*/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <ppu-lv2.h>
#include <io/pad.h>

#include "common.h"

#define VERSION_NAME 	"MAMBA/PRX Loader v1.5.0 by NzV"

#define SC_SYS_POWER 		(379)
#define SYS_REBOOT			0x8201
#define SC_FS_MOUNT  		(837)

#define BUTTON_SQUARE     	128
#define BUTTON_CROSS      	64
#define BUTTON_R1         	8
#define BUTTON_L1         	4

//----------------------------------------
//FLAG
//----------------------------------------

int verbose = 1;

//----------------------------------------
//LOG
//----------------------------------------

#ifdef ENABLE_LOG

#define	LOG_PATH	 	"/dev_hdd0/tmp/MAMBA_PRX_Loader.log"

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
//MAMBA/PRX LOADER
//----------------------------------------

#ifndef DISABLE_MAMBA
#include "mamba.h"
#endif

#ifndef DISABLE_VSH_PLUG
#include "vsh_plugins_loader.h"
#endif

//#define DISABLE_MAMBA
//#define DISABLE_VSH_PLUG

#ifndef DISABLE_MAMBA
int mamba_off = 0;
#endif

#ifndef DISABLE_VSH_PLUG
int noplugins = 0;
#endif

int run_mamba_prx_loader()
{	
	//COBRA
	if (is_cobra() == SUCCESS) 
	{
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog("Error: Running in COBRA mode!\r\n");
		#endif
		goto err_exit_loader;
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
			goto err_exit_loader;
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
	return SUCCESS;
	//END ERROR
err_exit_loader:
	return FAILED;
}

//----------------------------------------
//MAMBA/PRX AUTOLOADER
//----------------------------------------

#include "lv2_utils.h"

#define PATH_SYS_INI_OSD 		"/dev_blind/sys/internal/sys_init_osd.self" 
#define PATH_SYS_INI_OSD_ORIG 	"/dev_blind/sys/internal/sys_init_osd_orig.self"
#define PATH_DIR_AUTOLOADER 	"/dev_hdd0/game/MAMBAPRXL/USRDIR/Autoloader/"
#define	VSH_PLUGINS_PATH_MAMBA	"/dev_hdd0/mamba_plugins.txt"
#define	VSH_PLUGINS_PATH_PRX	"/dev_hdd0/prx_plugins.txt"

int install_autoloader = 0;
int uninstall_autoloader = 0;

int CopyFile(char* path, char* path2)
{
	int ret = 0;
	s32 fd = -1;
	s32 fd2 = -1;
	u64 lenght = 0LL;

	u64 pos = 0ULL;
	u64 readed = 0, writed = 0;

	char *mem = NULL;

	sysFSStat stat;

	ret= sysLv2FsStat(path, &stat);
	if(ret) goto skip;

	if(!memcmp(path, "/dev_hdd0/", 10) && !memcmp(path2, "/dev_hdd0/", 10))
	{
		if(strcmp(path, path2)==0) return ret;

		sysLv2FsUnlink(path2);
		return sysLv2FsLink(path, path2);
	}

    lenght = stat.st_size;

	ret = sysLv2FsOpen(path, 0, &fd, S_IRWXU | S_IRWXG | S_IRWXO, NULL, 0);
	if(ret) goto skip;

	ret = sysLv2FsOpen(path2, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd2, 0777, NULL, 0);
	if(ret) {sysLv2FsClose(fd);goto skip;}

	mem = malloc(0x100000);
	if (mem == NULL) return FAILED;

	while(pos < lenght)
	{
		readed = lenght - pos; if(readed > 0x100000ULL) readed = 0x100000ULL;
		ret=sysLv2FsRead(fd, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		ret=sysLv2FsWrite(fd2, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		pos += readed;
	}

skip:

	if(mem) free(mem);
	if(fd >=0) sysLv2FsClose(fd);
	if(fd2>=0) sysLv2FsClose(fd2);
	if(ret>0) ret = SUCCESS;

	return ret;
}

int get_firmware()
{
	uint64_t toc = lv2peek(0x8000000000003000ULL);
	switch(toc)
	{
		case 0x8000000000346390ULL:
			return 0x421;//C;
		break;
		case 0x8000000000363E80ULL:
			return 0x421;//D;
		break;
		case 0x8000000000348200ULL:
			return 0x430;//C;
		break;
		case 0x8000000000348210ULL:
			return 0x431;//C;
		break;
		case 0x80000000003487D0ULL:
			return 0x440;//C;
		break;
		case 0x80000000003487E0ULL:
			return 0x441;//C;
		break;
		case 0x80000000003665C0ULL:
			return 0x441;//D;
		break;
		case 0x8000000000366BD0ULL:
			return 0x446;//D;
		break;
		case 0x8000000000348DF0ULL:
			return 0x446;//C;
		break;
		case 0x800000000034B160ULL:
			return 0x450;//C;
		break;
		case 0x800000000036EC40ULL:
			return 0x450;//D;
		break;
		case 0x800000000034B2E0ULL:
			return 0x453;//C;
		break;
		case 0x8000000000370620ULL:
			return 0x453;//D;
		break;
		case 0x800000000034E620ULL:
			return 0x455;//C;
		break;
		case 0x80000000003738E0ULL:
			return 0x455;//D;
		break;
		case 0x800000000034F950ULL:
			return 0x460;//C;
		break;
		case 0x800000000034F960ULL:
			if(lv2peek(0x800000000031EBA8ULL)==0x323031342F31312FULL) return 0x466;//C;
			else return 0x465;//C;
		break;
		case 0x8000000000375510ULL:
			if(lv2peek(0x800000000031EBA8ULL)==0x323031342F31312FULL) return 0x466;//D;
			else return 0x465;//D;
		break;
		case 0x800000000034FB10ULL:
			return 0x470;//C;
		break;
		default:
			return 0;
		break;
	}
	return 0;
}

int run_uninstall_autoloader()
{	
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog("[UNINSTALLER]");
		#endif 
		//Check if installed
		if(file_exists("/dev_flash/sys/internal/sys_init_osd_orig.self") != SUCCESS)  
		{
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog("Error: sys_init_osd_orig.self not found\r\n");
			#endif 
			return FAILED;
		}
		//Enable dev_blind
		if(dir_exists("/dev_blind") != SUCCESS)
		{
			{lv2syscall8(SC_FS_MOUNT, (u64)(char*)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)(char*)"CELL_FS_FAT", (u64)(char*)"/dev_blind", 0, 0, 0, 0, 0); }
		}	
		//Restore original sys_init_osd.self
		if(file_exists(PATH_SYS_INI_OSD_ORIG) == SUCCESS)
		{
			if(file_exists(PATH_SYS_INI_OSD) == SUCCESS)
			{
				sysLv2FsChmod(PATH_SYS_INI_OSD, 0777);
				sysLv2FsUnlink(PATH_SYS_INI_OSD);
				sysLv2FsRename(PATH_SYS_INI_OSD_ORIG, PATH_SYS_INI_OSD);
				#ifdef ENABLE_LOG
				if (verbose) WriteToLog("Success: MAMBA/PRX Autoloader uninstalled\r\n");
				#endif
				return SUCCESS;
			}
			else  
			{ 
				#ifdef ENABLE_LOG
				if (verbose) WriteToLog("Error: sys_init_osd.self not found\r\n");
				#endif
				return FAILED;
			}
		}
		else  
		{ 
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog("Error: /dev_blind not mounted\r\n");
			#endif
			return FAILED;
		}
}

int run_install_autoloader()
{	
		//Uninstall New_Core
		run_uninstall_autoloader();
		#ifdef ENABLE_LOG
		if (verbose) WriteToLog("[INSTALLER]");
		#endif 
		//Init FW
		char filename[256];
		sprintf (filename, "%ssys_init_osd_%X.self",PATH_DIR_AUTOLOADER , get_firmware());
		#ifdef ENABLE_LOG
		if (verbose) {WriteToLog(filename); WriteToLog("\r\n");}
		#endif 
		if(file_exists(filename) != SUCCESS)
		{ 
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog("Error: Unable to find file\r\n");
			#endif 
			return FAILED;
		}
		//Enable dev_blind
		if(file_exists(PATH_SYS_INI_OSD) != SUCCESS)
			{{lv2syscall8(SC_FS_MOUNT, (u64)(char*)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)(char*)"CELL_FS_FAT", (u64)(char*)"/dev_blind", 0, 0, 0, 0, 0); }}
		//Install New_Core
		if(file_exists(PATH_SYS_INI_OSD_ORIG) != SUCCESS)
		{
			if((file_exists(PATH_SYS_INI_OSD) == SUCCESS))
			{
				sysLv2FsChmod(PATH_SYS_INI_OSD, 0777);
				sysLv2FsRename(PATH_SYS_INI_OSD, PATH_SYS_INI_OSD_ORIG);
				CopyFile(filename, PATH_SYS_INI_OSD);
				if ((file_exists("/dev_hdd0/game/MAMBAINST/USRDIR/mamba_plugins.txt") == SUCCESS) && (file_exists(VSH_PLUGINS_PATH_MAMBA) != SUCCESS))
				{
					CopyFile("/dev_hdd0/game/MAMBAINST/USRDIR/plugins.txt", VSH_PLUGINS_PATH_MAMBA);
				}
				if ((file_exists("/dev_hdd0/game/MAMBAINST/USRDIR/prx_plugins.txt") == SUCCESS)  && (file_exists(VSH_PLUGINS_PATH_PRX) != SUCCESS))
				{
					CopyFile("/dev_hdd0/game/MAMBAINST/USRDIR/prx_plugins.txt", VSH_PLUGINS_PATH_PRX);
				}	
			}
			else  
			{ 
				#ifdef ENABLE_LOG
				if (verbose) WriteToLog("Error: sys_init_osd.self not found (/dev_blind not mounted?)\r\n");
				#endif
				return FAILED;
			}
			return SUCCESS;
		}
		else
		{ 
			#ifdef ENABLE_LOG
			if (verbose) WriteToLog("Error: sys_init_osd_orig.self already exist\r\n");
			#endif
			return FAILED;
		}
}

//----------------------------------------
//MAIN
//----------------------------------------

int main()
{	
	#ifdef ENABLE_LOG
	verbose = 1;
	Open_Log(LOG_PATH);
	#endif
	//Detect startup mode
    unsigned button = 0;
    padInfo padinfo;
    padData paddata;
    ioPadInit(7);
    int n, r;
    for(r=0; r<10; r++)
    {
        ioPadGetInfo(&padinfo);
        for(n = 0; n < 7; n++)
        {
            if(padinfo.status[n])
            {
                ioPadGetData(n, &paddata);
                button = (paddata.button[2] << 8) | (paddata.button[3] & 0xff);
                break;
            }
        }
        if(button) break;
		else usleep(20000);
    }
    ioPadEnd();
	switch(button)
	{
		case BUTTON_CROSS:
			install_autoloader=1;
		break;
		
		case BUTTON_SQUARE:
			uninstall_autoloader=1;
		break;
		
		case BUTTON_R1:
			noplugins=1;
		break;
		
		case BUTTON_L1:
			mamba_off=1;
		break;
	}
	//Install MAMBA/PRX Autoloader if cross was hold
	if(install_autoloader)
	{
		if (run_install_autoloader() ==  SUCCESS)
		{	
			#ifdef ENABLE_LOG
			CloseLog();
			#endif	
			{lv2syscall3(392, 0x1004, 0x4, 0x6); } //1 Beep
			sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");
			{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0);} // Reboot
			return SUCCESS;
		}
		else
		{
			goto err_back_to_xmb;
		}
	}
	//Uninstall MAMBA/PRX Autoloader if square was hold
	else if(uninstall_autoloader)
	{
		if (run_uninstall_autoloader() ==  SUCCESS)
		{	
			#ifdef ENABLE_LOG
			CloseLog();
			#endif	
			{lv2syscall3(392, 0x1004, 0x4, 0x6); } //1 Beep
			sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");
			{lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0);} // Reboot
			return SUCCESS;
		}
		else
		{
			goto err_back_to_xmb;
		}
	}
	//Run MAMBA/PRX Loader
	else if (run_mamba_prx_loader() ==  SUCCESS)
	{	
		#ifdef ENABLE_LOG
		CloseLog();
		#endif	
		{lv2syscall3(392, 0x1004, 0x4, 0x6); } //1 Beep
		return SUCCESS; //Get back to xmb
	}
	//Error get back to xmb
err_back_to_xmb:	
    #ifdef ENABLE_LOG
	CloseLog();
	#endif	
	{lv2syscall3(392, 0x1004, 0xa, 0x1b6); } //3 Beep
	return FAILED; //Get back to xmb
}

