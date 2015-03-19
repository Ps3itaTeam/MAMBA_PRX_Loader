#include <ppu-lv2.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "common.h"

//----------------------------------------
//FILE UTILS
//----------------------------------------

char * LoadFile(char *path, int *file_size)
{
    FILE *fp;
    char *mem = NULL;

    *file_size = 0;

    sysLv2FsChmod(path, FS_S_IFMT | 0777);

    fp = fopen(path, "rb");

    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);

        *file_size = ftell(fp);

        mem = malloc(*file_size);

        if(!mem) {fclose(fp); return NULL;}

        fseek(fp, 0, SEEK_SET);

        if(*file_size != fread((void *) mem, 1, *file_size, fp))
        {
            fclose(fp); *file_size = 0;
            free(mem); return NULL;
        }
        fclose(fp);
    }

    return mem;
}

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
