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
#include "syscall8.h"
#include "mamba.h"

#if defined(FIRMWARE_355)

#define MAMBA_CEX
#include "payload/Iris_sky/355/payload_355.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/355/payload_355dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_421)

#define MAMBA_CEX
#include "payload/Iris_sky/421/payload_421.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/421/payload_421dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_430)

#define MAMBA_CEX
#include "payload/Iris_sky/430/payload_430.h"
#include "mamba_C_bin.h"

//#define MAMBA_DEX
//#include "payload/Iris_sky/430/payload_430dex.h"
//#include "mamba_D_bin.h"

#elif defined(FIRMWARE_431)

#define MAMBA_CEX
#include "payload/Iris_sky/431/payload_431.h"
#include "mamba_C_bin.h"

#elif defined(FIRMWARE_440)

#define MAMBA_CEX
#include "payload/Iris_sky/440/payload_440.h"
#include "mamba_C_bin.h"

#elif defined(FIRMWARE_441)

#define MAMBA_CEX
#include "payload/Iris_sky/441/payload_441.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/441/payload_441dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_446)

#define MAMBA_CEX
#include "payload/Iris_sky/446/payload_446.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/446/payload_446dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_450)

#define MAMBA_CEX
#include "payload/Iris_sky/450/payload_450.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/450/payload_450dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_453)

#define MAMBA_CEX
#include "payload/Iris_sky/453/payload_453.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/453/payload_453dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_455)

#define MAMBA_CEX
#include "payload/Iris_sky/455/payload_455.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/455/payload_455dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_460)

#define MAMBA_CEX
#include "payload/Iris_sky/460/payload_460.h"
#include "mamba_C_bin.h"


#elif defined(FIRMWARE_465)

#define MAMBA_CEX
#include "payload/Iris_sky/465/payload_465.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/465/payload_465dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_466)

#define MAMBA_CEX
#include "payload/Iris_sky/466/payload_466.h"
#include "mamba_C_bin.h"

#define MAMBA_DEX
#include "payload/Iris_sky/466/payload_466dex.h"
#include "mamba_D_bin.h"

#elif defined(FIRMWARE_470)

#define MAMBA_CEX
#include "payload/Iris_sky/470/payload_470.h"
#include "mamba_C_bin.h"

#endif

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

	
	#if defined(FIRMWARE_355)

	#ifdef MAMBA_CEX
	if (is_firm_355() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_355dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 355 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_421)

	#ifdef MAMBA_CEX
	if (is_firm_421() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_421dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 421 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_430)

	#ifdef MAMBA_CEX
	if (is_firm_430() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_430dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	}
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 430 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_431)

	#ifdef MAMBA_CEX
	if (is_firm_431() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_431dex() ==  1)
	{
		switch(is_payload_loaded_431dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_431dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 431 DEX installed");
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
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 431 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_440)

	#ifdef MAMBA_CEX
	if (is_firm_440() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_440dex() ==  1)
	{
		switch(is_payload_loaded_440dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_440dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 440 DEX installed");
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
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 440 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_441)

	#ifdef MAMBA_CEX
	if (is_firm_441() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_441dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 441 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_446)

	#ifdef MAMBA_CEX
	if (is_firm_446() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_446dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 446 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_450)

	#ifdef MAMBA_CEX
	if (is_firm_450() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_450dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 450 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_453)

	#ifdef MAMBA_CEX
	if (is_firm_453() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_453dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 453 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_455)

	#ifdef MAMBA_CEX
	if (is_firm_455() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_455dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 455 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_460)

	#ifdef MAMBA_CEX
	if (is_firm_460() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_460dex() ==  1)
	{
		switch(is_payload_loaded_460dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_460dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 460 DEX installed");
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
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 460 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_465)

	#ifdef MAMBA_CEX
	if (is_firm_465() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_465dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 465 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_466)

	#ifdef MAMBA_CEX
	if (is_firm_466() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	else
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_466dex() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
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
	else
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 466 CEX/DEX");
		#endif
		return FAILED;
	}

	#elif defined(FIRMWARE_470)

	#ifdef MAMBA_CEX
	if (is_firm_470() ==  1)
	{
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
					PAYLOAD_MAMBA = (u64)mamba_C_bin;
					PAYLOAD_MAMBA_SIZE = mamba_C_bin_size;
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
	#endif
	
	#ifdef MAMBA_DEX
	if (is_firm_470dex() ==  1)
	{
		switch(is_payload_loaded_470dex())
            {
				case SKY10_PAYLOAD:
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Error: Iris (sky) payload already loaded");
					#endif
					return FAILED;
				break;
				
				case ZERO_PAYLOAD:
					load_payload_470dex(ZERO_PAYLOAD);
					__asm__("sync");
					sleep(1); //maybe need it, maybe not
					PAYLOAD_MAMBA = (u64)mamba_D_bin;
					PAYLOAD_MAMBA_SIZE = mamba_D_bin_size;
					#ifdef ENABLE_LOG
					if (EnableLogMamba)  WriteMambaLog("Success: Iris (sky) payload 470 DEX installed");
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
	#endif
	
	{
		#ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: Not firmware 470 CEX/DEX");
		#endif
		return FAILED;
	}

	#endif
	
	// Load Mamba payload
	#ifdef ENABLE_LOG
	if (EnableLogMamba)  WriteMambaLog("[MAMBA]");
	#endif
	
	sys8_write_htab();
	
	uint32_t file_size = (PAYLOAD_MAMBA_SIZE + 0x4000) & ~127;
    u64 lv2_mem = sys8_alloc(file_size, 0x27ULL); // alloc LV2 memory
	if(!lv2_mem)
	{
        #ifdef ENABLE_LOG
		if (EnableLogMamba)  WriteMambaLog("Error: LV2 memory is full");
		#endif
        return FAILED;
    }
	
    lv2poke(lv2_mem, lv2_mem + 0x8ULL);
    sys8_memcpy(lv2_mem + 8, (u64) PAYLOAD_MAMBA, PAYLOAD_MAMBA_SIZE);

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

