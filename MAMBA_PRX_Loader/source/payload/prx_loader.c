/*
	==============================================================
	
	MAMBA/PRX loader By NzV
	
	Load of MAMBA and/or VSH plugins (with MAMBA or PRX Loader) after system boot.

	==============================================================
*/
#include <malloc.h>
#include <ppu-lv2.h>
#include <stdio.h>

#include "common.h"
#include "lv2_utils.h"
#include "prx_loader.h"

//----------------------------------------
//PRX LOADER LOG
//----------------------------------------

#ifdef ENABLE_LOG

static char logPrx[1024];
int EnableLogPrx = 0;

static void InitPrxLog()
{
	logPrx[0]=0;
}

static void WritePrxLog(char *msg)
{
	sprintf(logPrx,"%s%s\r\n", logPrx, msg);
}

char *GetPrxLog()
{
	return logPrx;
}

#endif

//----------------------------------------
//PRX LOADER PAYLOAD
//----------------------------------------

#define PAYLOAD_PRX_INSTALL_OFFSET				0x80000000007F0000ULL
#define PAYLOAD_PRX_SYSCALL_NUM					1022
#define PAYLOAD_DIR_PRXLOADER 					"/dev_hdd0/game/MAMBAPRXL/USRDIR/PRXLoader/"

void lv1poke( u64 addr, u64 val)//needed for patch lv2 protection (rebug 446 only)	
{
	if(addr==0) return;
	lv2syscall2(9, addr, val);
}

void write_htab(void)
{
    uint64_t cont = 0;
    uint64_t reg5, reg6;
    uint32_t val;
    while(cont < 0x80) 
	{
        val = (cont << 7);
        reg5 = lv2peek(0x800000000f000000ULL | ((uint64_t) val));
        reg6 = lv2peek(0x800000000f000008ULL | ((uint64_t) val));
        reg6 = (reg6  & 0xff0000ULL) | 0x190ULL;
		lv2syscall8(10, 0, (cont << 3ULL), reg5, reg6, 0, 0, 0, 1);
        cont++;
    }
}

int get_lv2_version()
{
	uint64_t toc = lv2peek(0x8000000000003000ULL);
	switch(toc)
	{
		case 0x8000000000330540ULL:
			return 0x355C;
		break;
		case 0x800000000034AC80ULL:
			return 0x355D;
		break;
		case 0x8000000000346390ULL:
			return 0x421C;
		break;
		case 0x8000000000363E80ULL:
			return 0x421D;
		break;
		case 0x8000000000348200ULL:
			return 0x430C;
		break;
		case 0x8000000000365CA0ULL:
			return 0x430D;
		break;
		case 0x8000000000348210ULL:
			return 0x431C;
		break;
		case 0x80000000003487D0ULL:
			return 0x440C;
		break;
		case 0x80000000003487E0ULL:
			return 0x441C;
		break;
		case 0x80000000003665C0ULL:
			return 0x441D;
		break;
		case 0x8000000000366BD0ULL:
			return 0x446D;
		break;
		case 0x8000000000348DF0ULL:
			return 0x446C;
		break;
		case 0x800000000034B160ULL:
			return 0x450C;
		break;
		case 0x800000000036EC40ULL:
			return 0x450D;
		break;
		case 0x800000000034B2E0ULL:
			return 0x453C;
		break;
		case 0x8000000000370620ULL:
			return 0x453D;
		break;
		case 0x800000000034E620ULL:
			return 0x455C;
		break;
		case 0x80000000003738E0ULL:
			return 0x455D;
		break;
		case 0x800000000034F950ULL:
			return 0x460C;
		break;
		case 0x800000000034F960ULL:
			if(lv2peek(0x800000000031EBA8ULL)==0x323031342F31312FULL) return 0x466C;
			else return 0x465C;
		break;
		case 0x8000000000375510ULL:
			if(lv2peek(0x800000000031EBA8ULL)==0x323031342F31312FULL) return 0x466D;
			else return 0x465D;
		break;
		case 0x800000000034FB10ULL:
			return 0x470C;
		break;
		default:
			return 0;
		break;
	}
	return 0;
}

uint64_t get_syscall_table(int lv2_version)
{
	switch(get_lv2_version())
	{
		case 0x355C:
			return 0x8000000000346570ULL;
		break;
		case 0x355D:
			return 0x8000000000361578ULL;
		break;
		case 0x421C:
			return 0x800000000035BCA8ULL;
		break;
		case 0x421D:
			return 0x800000000037A1B0ULL;
		break;
		case 0x430C:
			return 0x800000000035DBE0ULL;
		break;
		case 0x430D:
			return 0x800000000037C068ULL;
		break;
		case 0x431C:
			return 0x800000000035DBE0ULL;
		break;
		case 0x440C:
			return 0x800000000035E260ULL;
		break;
		case 0x441C:
			return 0x800000000035E260ULL;
		break;
		case 0x441D:
			return 0x800000000037C9E8ULL;
		break;
		case 0x446C:
			return 0x800000000035E860ULL;
		break;
		case 0x446D:
			return 0x800000000037CFE8ULL;
		break;
		case 0x450C:
			return 0x800000000035F0D0ULL;
		break;
		case 0x450D:
			return 0x8000000000383658ULL;
		break;
		case 0x453C:
			return 0x800000000035F300ULL;
		break;
		case 0x453D:
			return 0x8000000000385108ULL;
		break;
		case 0x455C:
			return 0x8000000000362680ULL;
		break;
		case 0x455D:
			return 0x8000000000388488ULL;
		break;
		case 0x460C:
			return 0x8000000000363A18ULL;
		break;
		case 0x465C:
			return 0x8000000000363A18ULL;
		break;
		case 0x465D:
			return 0x800000000038A120ULL;
		break;
		case 0x466C:
			return 0x8000000000363A18ULL;
		break;
		case 0x466D:
			return 0x800000000038A120ULL;
		break;
		case 0x470C:
			return 0x8000000000363B60ULL;
		break;
		default:
			return 0;
		break;
	}
	return 0;
}

uint8_t * read_file(char *path, uint32_t * file_size, uint16_t round)
{
	uint8_t * buf;
	uint32_t size = 0;
	uint16_t rest;
	FILE * f = fopen(path, "rb");
	if(f)
	{
		uint32_t size = fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if(round)
		{
			rest = size % round;
			if(rest)
				size = size - rest + round;
		}
		buf = malloc(size);
		fread(buf, size, 1, f);
		fclose(f);
		*(file_size) = size;
		return buf;
	}else{
		*(file_size) = 0;
		return NULL;
	}
}

int load_prx_loader_payload(int verbose)
{
	#ifdef ENABLE_LOG
	EnableLogPrx = verbose;
	if (EnableLogPrx) 
	{
		InitPrxLog();
	}
	if (EnableLogPrx)  WritePrxLog("[PRX LOADER]");
	#endif
	
	int lv2_version = get_lv2_version();
	if(!lv2_version)
	{
		#ifdef ENABLE_LOG
		if (EnableLogPrx) WritePrxLog("Error: Unsupported firmware");
		#endif
		return FAILED;
	}
	
	if((lv2_version>>4) == 0x446)//patch lv2 protection (rebug only, doesn't affect others)
	{	
		lv1poke(0x370AA8, 0x0000000000000001ULL);
		lv1poke(0x370AA8 + 8, 0xE0D251B556C59F05ULL);
		lv1poke(0x370AA8 + 16, 0xC232FCAD552C80D7ULL);
		lv1poke(0x370AA8 + 24, 0x65140CD200000000ULL);
	}
	
	write_htab();
	
	char payload_path[256];
	sprintf(payload_path, "%spayload_%X.bin", PAYLOAD_DIR_PRXLOADER, lv2_version);
	
	uint64_t * payload;
	uint32_t size;
	payload = (uint64_t *) read_file(payload_path, &size, 8);
	
	if(!payload)
	{
		#ifdef ENABLE_LOG
		if (EnableLogPrx) WritePrxLog("Error: Unable to find MAMBA payload file");
		#endif
		return FAILED; 
	}
	
	uint64_t syscall_table = get_syscall_table(lv2_version);
	uint64_t payload_opd = PAYLOAD_PRX_INSTALL_OFFSET + size + 0x10;
	int i;	
	for(i=0;i<(size/8);i++) lv2poke(PAYLOAD_PRX_INSTALL_OFFSET+(i*8), payload[i]);
	lv2poke(payload_opd, PAYLOAD_PRX_INSTALL_OFFSET);
	lv2poke(syscall_table + (8*PAYLOAD_PRX_SYSCALL_NUM), payload_opd);
	free(payload);
	
	lv2poke(0x8000000000003D90ULL, 0x386000014E800020ULL); // /patch permission 4.xx, usually "fixed" by warez payload
	
	#ifdef ENABLE_LOG
	if (EnableLogPrx) WritePrxLog("Success: PRX Loader payload installed");
	#endif
	return SUCCESS; 			
}

