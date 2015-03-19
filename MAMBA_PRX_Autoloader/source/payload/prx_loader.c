/*
	==============================================================
	
	MAMBA/PRX Autoloader by NzV
	
	Load MAMBA and/or VSH plugins (with MAMBA or PRX Loader) at system boot using New Core as loader.
	
	==============================================================
*/

#include <malloc.h>
#include <ppu-lv2.h>
#include <stdio.h>

#include "common.h"
#include "lv2_utils.h"

#include "prx_loader.h"

//----------------------------------------
//FW DEF
//----------------------------------------

#define TOC_OFFSET 			0x8000000000003000ULL

#if defined(FIRMWARE_355)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x8000000000330540ULL
#define SYSCALL_TABLE_C 	0x8000000000346570ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x800000000034AC80ULL
#define SYSCALL_TABLE_D 	0x8000000000361578ULL

#elif defined(FIRMWARE_421)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x8000000000346390ULL
#define SYSCALL_TABLE_C 	0x800000000035BCA8ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000363E80ULL
#define SYSCALL_TABLE_D 	0x800000000037A1B0ULL

#elif defined(FIRMWARE_430)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x8000000000348200ULL
#define SYSCALL_TABLE_C 	0x800000000035DBE0ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000365CA0ULL
#define SYSCALL_TABLE_D 	0x800000000037C068ULL

#elif defined(FIRMWARE_431)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x8000000000348210ULL
#define SYSCALL_TABLE_C 	0x800000000035DBE0ULL

#elif defined(FIRMWARE_440)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x80000000003487D0ULL
#define SYSCALL_TABLE_C 	0x800000000035E260ULL

#elif defined(FIRMWARE_441)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x80000000003487E0ULL
#define SYSCALL_TABLE_C 	0x800000000035E260ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x80000000003665C0ULL
#define SYSCALL_TABLE_D 	0x800000000037C9E8ULL

#elif defined(FIRMWARE_446)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x8000000000348DF0ULL
#define SYSCALL_TABLE_C 	0x800000000035E860ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000366BD0ULL
#define SYSCALL_TABLE_D 	0x800000000037CFE8ULL

#elif defined(FIRMWARE_450)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034B160ULL
#define SYSCALL_TABLE_C 	0x800000000035F0D0ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x800000000036EC40ULL
#define SYSCALL_TABLE_D 	0x8000000000383658ULL

#elif defined(FIRMWARE_453)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034B2E0ULL
#define SYSCALL_TABLE_C 	0x800000000035F300ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000370620ULL
#define SYSCALL_TABLE_D 	0x8000000000385108ULL

#elif defined(FIRMWARE_455)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034E620ULL
#define SYSCALL_TABLE_C 	0x8000000000362680ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x80000000003738E0ULL
#define SYSCALL_TABLE_D 	0x8000000000388488ULL

#elif defined(FIRMWARE_460)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034F950ULL
#define SYSCALL_TABLE_C 	0x8000000000363A18ULL

#elif defined(FIRMWARE_465)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034F960ULL
#define SYSCALL_TABLE_C 	0x8000000000363A18ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000375510ULL
#define SYSCALL_TABLE_D 	0x800000000038A120ULL

#elif defined(FIRMWARE_466)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034F960ULL
#define SYSCALL_TABLE_C 	0x8000000000363A18ULL

#define PRX_LOADER_DEX
#define TOC_DEX 			0x8000000000375510ULL
#define SYSCALL_TABLE_D 	0x800000000038A120ULL

#elif defined(FIRMWARE_470)

#define PRX_LOADER_CEX
#define TOC_CEX 			0x800000000034FB10ULL
#define SYSCALL_TABLE_C 	0x8000000000363B60ULL

#endif
	
#ifdef PRX_LOADER_CEX
#include "payload_prx_C_bin.h"
#endif

#ifdef PRX_LOADER_DEX
#include "payload_prx_D_bin.h"
#endif

#define PAYLOAD_PRX_INSTALL_OFFSET				0x80000000007F0000ULL
#define PAYLOAD_PRX_SYSCALL_NUM					1022

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

#ifdef FIRMWARE_446  //needed for patch lv2 protection (rebug 446 only)	
void lv1poke( u64 addr, u64 val)
{
	if(addr==0) return;
	lv2syscall2(9, addr, val);
}
#endif

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
	uint32_t payload_size = 0;
	uint64_t *payload; 
	uint64_t syscall_table;
	
	uint64_t toc = lv2peek(TOC_OFFSET);
	switch(toc)
	{
	
		#if defined(FIRMWARE_355)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_421)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_430)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_431)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_440)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_441)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_446)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_450)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_453)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_455)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_460)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_465)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_466)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#elif defined(FIRMWARE_470)

		#ifdef PRX_LOADER_CEX
		case TOC_CEX:
			syscall_table = SYSCALL_TABLE_C;
			payload_size = payload_prx_C_bin_size;
			payload = (uint64_t *)payload_prx_C_bin;
		break;
		#endif
		
		#ifdef PRX_LOADER_DEX
		case TOC_DEX:
			syscall_table = SYSCALL_TABLE_D;
			payload_size = payload_prx_D_bin_size;
			payload = (uint64_t *)payload_prx_D_bin;
		break;
		#endif
		
		#endif

		default:
			#ifdef ENABLE_LOG
			if (EnableLogPrx)  WritePrxLog("Error: Unknown firmware");
			#endif
			return FAILED;
		break;
		
	}
	
	#ifdef FIRMWARE_446  //patch lv2 protection (rebug only, doesn't affect others)	
	lv1poke(0x370AA8, 0x0000000000000001ULL);
	lv1poke(0x370AA8 + 8, 0xE0D251B556C59F05ULL);
	lv1poke(0x370AA8 + 16, 0xC232FCAD552C80D7ULL);
	lv1poke(0x370AA8 + 24, 0x65140CD200000000ULL);
	#endif
	
	write_htab();
				
	uint64_t payload_opd = PAYLOAD_PRX_INSTALL_OFFSET + payload_size + 0x10;

	int i;
	for(i=0;i<(payload_size/8);i++)
		lv2poke(PAYLOAD_PRX_INSTALL_OFFSET +(i*8), payload[i]);			// displaces 8 bytes (64 bit) to reserve space to opd

	lv2poke(payload_opd, PAYLOAD_PRX_INSTALL_OFFSET);
	lv2poke(syscall_table + (8*PAYLOAD_PRX_SYSCALL_NUM), payload_opd);

	lv2poke(0x8000000000003D90ULL, 0x386000014E800020ULL); //patch permission 4.xx, usually "fixed" by warez payload
	
	#ifdef ENABLE_LOG
	if (EnableLogPrx) WritePrxLog("Success: PRX Loader payload installed");
	#endif
	
	return SUCCESS; 			
}

