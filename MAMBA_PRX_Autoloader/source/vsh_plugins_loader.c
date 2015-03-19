/*
	==============================================================
	
	MAMBA/PRX Autoloader by NzV
	
	Load MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot using New Core as loader.
	
	==============================================================
*/

#include <malloc.h>
#include <ppu-lv2.h>
#include <stdio.h>
#include <sys/file.h>

#include "common.h"
#include "vsh_plugins_loader.h"

//#define DISABLE_PRXLOADER

#ifndef DISABLE_PRXLOADER
#include "prx_loader.h"
#endif


//----------------------------------------
//VSH PLUGINS LOG
//----------------------------------------

#ifdef ENABLE_LOG

static char logVSHPlug[1024];
int EnableLogVSHPlug = 0;

static void InitVSHPlugLog()
{
	logVSHPlug[0]=0;
}

static void WriteVSHPlugLog(char *msg)
{
	sprintf(logVSHPlug,"%s%s\r\n", logVSHPlug, msg);
}

char *GetVSHPlugLog()
{
	return logVSHPlug;
}

#endif

//----------------------------------------
//VSH PLUGINS LOADER
//----------------------------------------

#define PAYLOAD_PRX_SYSCALL_NUM					1022
#define SYSCALL8_OPCODE_GET_VERSION				0x7000
#define SYSCALL8_OPCODE_LOAD_VSH_PLUGIN			0x1EE7
#define MAX_VSH_PLUGINS							7
#define VSH_PLUGINS_FIRST_SLOT					1
#define	VSH_PLUGINS_PATH_MAMBA					"/dev_hdd0/mamba_plugins.txt"
#define	VSH_PLUGINS_PATH_PRX					"/dev_hdd0/prx_plugins.txt"

int syscall_load_prx_module(uint64_t syscall_num, uint32_t slot, char * path, void * arg, uint32_t arg_size)
{
    lv2syscall5(syscall_num, SYSCALL8_OPCODE_LOAD_VSH_PLUGIN, (uint64_t) slot, (uint64_t) path, (uint64_t) arg, (uint64_t) arg_size);
	return_to_user_prog(int);
}

static int read_text_line(int fd, char *line, unsigned int size, int *eof)
{
	int i = 0, j;
	int line_started = 0;

	if (size == 0) return -1;

	*eof = 0;

	while (i < (size-1))
	{
		uint8_t ch;
		uint64_t r;

		if (sysLv2FsRead(fd, &ch, 1, &r) != 0 || r != 1)
		{
			*eof = 1;
			break;
		}

		if (!line_started)
		{
			if (ch > ' ')
			{
				line[i++] = (char)ch;
				line_started = 1;
			}
		}
		else
		{
			if (ch == '\n' || ch == '\r') break;
			line[i++] = (char)ch;
		}
	}

	line[i] = 0;

	// Remove space chars at end
	for (j = i-1; j >= 0; j--)
	{
		if (line[j] <= ' ')
		{
			line[j] = 0;
			i = j;
		}
		else break;
	}

	return i;
}

uint32_t load_all_prx(char *plugins_path, uint64_t syscall_num)
{
	#ifdef ENABLE_LOG
	if (EnableLogVSHPlug)  WriteVSHPlugLog(plugins_path);
	#endif
	int fd;
	unsigned int slot = VSH_PLUGINS_FIRST_SLOT;
	if (sysLv2FsOpen(plugins_path, SYS_O_RDONLY, &fd, 0, NULL, 0) != SUCCESS) return FAILED;
	while (slot < MAX_VSH_PLUGINS)
	{
		char path[128];
		int eof;
		if (read_text_line(fd, path, sizeof(path), &eof) > 0)
		{
			syscall_load_prx_module(syscall_num, slot, path, 0, 0);
			#ifdef ENABLE_LOG
			if (EnableLogVSHPlug)  WriteVSHPlugLog(path);
			#endif
			slot++;
		}
		if (eof) break;
	}	
	sysLv2FsClose(fd);
	return (slot-VSH_PLUGINS_FIRST_SLOT);
}

int load_vsh_plugins(int verbose)
{
	#ifdef ENABLE_LOG
	EnableLogVSHPlug = verbose;
	if (EnableLogVSHPlug) 
	{
		InitVSHPlugLog();
	}
	#endif
	if(is_mamba() == SUCCESS)
	{
		#ifdef ENABLE_LOG
		if (EnableLogVSHPlug)  WriteVSHPlugLog("[VSH PLUGINS]");
		if (EnableLogVSHPlug) WriteVSHPlugLog("Success: MAMBA detected");
		#endif
		return load_all_prx(VSH_PLUGINS_PATH_MAMBA, 8);
	}
	#ifndef DISABLE_PRXLOADER
	else if (load_prx_loader_payload(EnableLogVSHPlug) == SUCCESS)
	{
		#ifdef ENABLE_LOG
		if (EnableLogVSHPlug)  WriteVSHPlugLog(GetPrxLog());
		if (EnableLogVSHPlug)  WriteVSHPlugLog("[VSH PLUGINS]");
		if (EnableLogVSHPlug)  WriteVSHPlugLog("Success: PRX Loader detected");
		#endif
		return load_all_prx(VSH_PLUGINS_PATH_PRX, PAYLOAD_PRX_SYSCALL_NUM);
	}
	#endif
	#ifdef ENABLE_LOG
	if (EnableLogVSHPlug)  WriteVSHPlugLog("[VSH plugins]");
	#ifndef DISABLE_PRXLOADER
	if (EnableLogVSHPlug) WriteVSHPlugLog("Error: MAMBA not detected, PRX Loader too");
	#else
	if (EnableLogVSHPlug) WriteVSHPlugLog("Error: MAMBA not detected");
	#endif
	#endif
	return FAILED;
}
