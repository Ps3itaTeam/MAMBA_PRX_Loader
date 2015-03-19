#ifndef _COMMON_H_
#define _COMMON_H_

#define SUCCESS 	0
#define FAILED	 	-1

#define FS_S_IFMT 0170000

int file_exists(const char *path);int dir_exists(const char *path);
char * LoadFile(char *path, int *file_size);

//----------------------------------------
//COBRA/MAMBA
//----------------------------------------

int is_cobra(void);
int is_mamba(void);

#endif