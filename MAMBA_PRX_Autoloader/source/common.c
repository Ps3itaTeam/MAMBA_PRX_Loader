#include <ppu-lv2.h>

#include "common.h"

//----------------------------------------
//COBRA/MAMBA
//----------------------------------------

#define SYSCALL8_OPCODE_GET_VERSION			0x7000ULL
#define SYSCALL8_OPCODE_GET_MAMBA			0x7FFFULL

int sys8_get_version(u32 *version)
{
    lv2syscall2(8, SYSCALL8_OPCODE_GET_VERSION, (u64)version);
    return_to_user_prog(int);
}

int sys8_get_mamba(void)
{
	lv2syscall1(8, SYSCALL8_OPCODE_GET_MAMBA);
    return_to_user_prog(int);
}

int is_cobra(void)
{
    u32 version = 0x99999999;
    if (sys8_get_version(&version) < 0)	return FAILED;
    if (version != 0x99999999 && sys8_get_mamba() != 0x666)	return SUCCESS;
    return FAILED;
}

int is_mamba(void)
{
	u32 version = 0x99999999;
    if (sys8_get_version(&version) < 0)	return FAILED;
    if (version != 0x99999999 && sys8_get_mamba() == 0x666)	return SUCCESS;
    return FAILED;
}
