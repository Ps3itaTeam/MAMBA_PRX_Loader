/*
	==============================================================
	
	MAMBA/PRX loader By NzV
	
	Load of MAMBA and/or VSH plugins (with MAMBA or PRX Loader) after system boot.

	==============================================================
*/

#include <malloc.h>
#include <ppu-lv2.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lv2_utils.h"
#include "syscall8.h"
#include "zlib.h"
#include "mamba.h"

#include "payload/Iris_sky/355/payload_355.h"
#include "payload/Iris_sky/355/payload_355dex.h"
#include "payload/Iris_sky/421/payload_421.h"
#include "payload/Iris_sky/421/payload_421dex.h"
#include "payload/Iris_sky/430/payload_430.h"
//#include "payload/Iris_sky/430/payload_430dex.h"
#include "payload/Iris_sky/431/payload_431.h"
#include "payload/Iris_sky/440/payload_440.h"
#include "payload/Iris_sky/441/payload_441.h"
#include "payload/Iris_sky/441/payload_441dex.h"
#include "payload/Iris_sky/446/payload_446.h"
#include "payload/Iris_sky/446/payload_446dex.h"
#include "payload/Iris_sky/450/payload_450.h"
#include "payload/Iris_sky/450/payload_450dex.h"
#include "payload/Iris_sky/453/payload_453.h"
#include "payload/Iris_sky/453/payload_453dex.h"
#include "payload/Iris_sky/455/payload_455.h"
#include "payload/Iris_sky/455/payload_455dex.h"
#include "payload/Iris_sky/460/payload_460.h"
#include "payload/Iris_sky/465/payload_465.h"
#include "payload/Iris_sky/465/payload_465dex.h"
#include "payload/Iris_sky/466/payload_466.h"
#include "payload/Iris_sky/466/payload_466dex.h"
#include "payload/Iris_sky/470/payload_470.h"

//----------------------------------------
//Logs
//----------------------------------------

#ifdef ENABLE_LOG

static char logMamba[1024];
int EnableLogMamba = 0;

static void InitMambaLog()
{
	logMamba[0]=0;
}

static void WriteMambaLog(char *msg)
{
	sprintf(logMamba,"%s%s\r\n", logMamba, msg);
}

char *GetMambaLog()
{
	return logMamba;
}

#endif

//----------------------------------------
// Mamba
//----------------------------------------

#define PAYLOAD_DIR_MAMBA "/dev_hdd0/game/MAMBAPRXL/USRDIR/MAMBA/"

int firmware = 0;

u64 restore_syscall8[2] = {0,0};
u64 syscall_base;

int noBDVD;

char path_name[MAXPATHLEN];
char temp_buffer[8192];

int (*lv2_unpatch_bdvdemu)(void) = NULL;
int (*lv2_patch_bdvdemu)(uint32_t flags) = NULL;
int (*lv2_patch_storage)(void) = NULL;
int (*lv2_unpatch_storage)(void) = NULL;

u64 PAYLOAD_MAMBA;
size_t PAYLOAD_MAMBA_SIZE;

static void sys8_write_htab(void)
{
    u64 cont = 0;
    lv1_reg regs_i, regs_o;
    u32 val;

    while(cont < 0x80)
	{
        val = (cont << 7);

        regs_i.reg3 = 0;
        regs_i.reg4 = (cont << 3ULL);
        regs_i.reg5 = lv2peek(0x800000000f000000ULL | ((u64) val));
        regs_i.reg6 = lv2peek(0x800000000f000008ULL | ((u64) val));
        regs_i.reg6 = (regs_i.reg6  & 0xff0000ULL) | 0x190ULL;
        regs_i.reg11= 1;

        sys8_lv1_syscall(&regs_i, &regs_o);

        cont++;
    }
}

u64 syscall_40(u64 cmd, u64 arg)
{
    lv2syscall2(40, cmd, arg);
    return_to_user_prog(u64);
}

int load_mamba(int verbose)
{
	#ifdef ENABLE_LOG
		EnableLogMamba = verbose;
		if (EnableLogMamba) InitMambaLog();
	#endif
	
	if (is_cobra() ==  SUCCESS)
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: COBRA is enabled");
		#endif
        return FAILED;
	}
	
	if (is_mamba() ==  SUCCESS) 
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: MAMBA already loaded");
		#endif
        return FAILED;
	}
	
	// Load Iris (Sky10) payload
	#ifdef ENABLE_LOG
	if (EnableLogMamba)  WriteMambaLog("[IRIS (SKY)]");
	#endif

	if (is_firm_355() ==  1)
	{
		firmware = 0x355C;		
		switch(is_payload_loaded_355())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					patch_lv2_protection();	
					remove_new_poke(); //restore poke
					unmap_lv1();  //3.55 need unmap?	
					__asm__("sync");
					sleep(1); // dont touch! nein!	
					load_payload_355(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); // maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 355 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_355dex() ==  1)
	{
		firmware = 0x355D;		
		switch(is_payload_loaded_355dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					install_new_poke_355dex();
					if (!map_lv1_355dex())
					{
						remove_new_poke_355dex();
						#ifdef ENABLE_LOG
						if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded (map_lv1 failed?)");
						#endif
						return FAILED;
					}
					patch_lv2_protection_355dex();
					remove_new_poke_355dex(); // restore pokes
					unmap_lv1_355dex();  // 3.55 need unmap
					__asm__("sync");
					sleep(1); // dont touch! nein!		
					load_payload_355dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); // maybe need it, maybe not 
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 355 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_421() ==  1)
	{
		firmware = 0x421C;		
		switch(is_payload_loaded_421())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_421(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 421 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_421dex() ==  1)
	{
		firmware = 0x421D;		
		switch(is_payload_loaded_421dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_421dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 421 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_430() ==  1)
	{
		firmware = 0x430C;		
		switch(is_payload_loaded_430())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_430(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 430 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
/* 	else if (is_firm_430dex() ==  1)
	{
		firmware = 0x430D;	
		switch(is_payload_loaded_430dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_430dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 430 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	} */
	else if (is_firm_431() ==  1)
	{
		firmware = 0x431C;		
		switch(is_payload_loaded_431())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_431(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 431 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_440() ==  1)
	{
		firmware = 0x440C;		
		switch(is_payload_loaded_440())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_440(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 440 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_441() ==  1)
	{
		firmware = 0x441C;		
		switch(is_payload_loaded_441())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_441(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 441 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_441dex() ==  1)
	{
		firmware = 0x441D;	
		switch(is_payload_loaded_441dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_441dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 441 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_446() ==  1)
	{
		firmware = 0x446C;		
		switch(is_payload_loaded_446())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_446(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 446 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_446dex() ==  1)
	{
		firmware = 0x446D;	
		switch(is_payload_loaded_446dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_446dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 446 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_450() ==  1)
	{
		firmware = 0x450C;		
		switch(is_payload_loaded_450())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_450(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 450 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_450dex() ==  1)
	{
		firmware = 0x450D;	
		switch(is_payload_loaded_450dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_450dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 450 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_453() ==  1)
	{
		firmware = 0x453C;		
		switch(is_payload_loaded_453())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_453(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 453 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_453dex() ==  1)
	{
		firmware = 0x453D;	
		switch(is_payload_loaded_453dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_453dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 453 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_455() ==  1)
	{
		firmware = 0x455C;		
		switch(is_payload_loaded_455())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_455(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 455 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_455dex() ==  1)
	{
		firmware = 0x465D;	
		switch(is_payload_loaded_455dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_455dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 455 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_460() ==  1)
	{
		firmware = 0x460C;		
		switch(is_payload_loaded_460())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_460(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 460 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_465() ==  1)
	{
		firmware = 0x465C;		
		switch(is_payload_loaded_465())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_465(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 465 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_465dex() ==  1)
	{
		firmware = 0x465D;	
		switch(is_payload_loaded_465dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_465dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 465 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_466() ==  1)
	{
		firmware = 0x466C;		
		switch(is_payload_loaded_466())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_466(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 466 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_466dex() ==  1)
	{
		firmware = 0x466D;	
		switch(is_payload_loaded_466dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_466dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 466 DEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else if (is_firm_470() ==  1)
	{
		firmware = 0x470C;		
		switch(is_payload_loaded_470())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_470(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 470 CEX installed");
					#endif
				break;
				
				default:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload not loaded");
					#endif
					return FAILED;
				break;
			}
	}
	else
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Unknown firmware");
		#endif
		return FAILED;
	}
	
	// Load Mamba payload
	#ifdef ENABLE_LOG
	if (EnableLogMamba)  WriteMambaLog("[MAMBA]");
	#endif
	
	u64 *addr= (u64 *) memalign(128, 0x20000);
    if(!addr) 
	{
        #ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: memalign(128, 0x20000) failed");
		#endif
		return FAILED;
	}
	
	memset((char *) addr, 0, 0x20000);
	
	char payload_file[128];
    sprintf(payload_file, "%smamba_%X.lz.bin", PAYLOAD_DIR_MAMBA, firmware);
	#ifdef ENABLE_LOG
	if (EnableLogMamba)  WriteMambaLog(payload_file);
	#endif
	if(file_exists(payload_file) == FAILED)
	{
		free(addr);
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Unable to load MAMBA payload");
		#endif
		return FAILED;
	}
	
    int out_size;
	int  file_size = 0;
    char *mamba_payload = LoadFile((char *) payload_file, &file_size);

    if(file_size < 20000)
    {
        if(mamba_payload) free(mamba_payload);
        free(addr);
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: MAMBA payload file size  < 20000");
		#endif
		return FAILED;
    }
	
	zlib_decompress((char *) mamba_payload, (char *) addr, file_size, &out_size);
	free(mamba_payload);
		
	sys8_write_htab();
		
	out_size = (out_size + 0x4000) & ~127;
    u64 lv2_mem = sys8_alloc(out_size, 0x27ULL); // alloc LV2 memory
	if(!lv2_mem)
	{
        free(addr);
        #ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: LV2 memory is full");
		#endif
        return FAILED;
    }
	
    lv2poke(lv2_mem, lv2_mem + 0x8ULL);
    sys8_memcpy(lv2_mem + 8, (u64) addr, out_size);
	free(addr);
	
    lv2poke(syscall_base + (u64) (40 * 8), lv2_mem);  // syscall management
    lv2poke(0x80000000000004E8ULL, 0);
    usleep(1000);
	
	syscall_40(1, 0);
    sys8_perm_mode(0);
	
	if(restore_syscall8[0]) sys8_pokeinstr(restore_syscall8[0], restore_syscall8[1]);
    restore_syscall8[1]= lv2peek(restore_syscall8[0]); // use mamba vector
	
	#ifdef ENABLE_LOG
	if (EnableLogMamba)  WriteMambaLog("Success: MAMBA payload installed");
	#endif
	
    return SUCCESS;
}

