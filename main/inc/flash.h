#ifndef __flash__h__
#define __flash__h__
#include <stdio.h>
int32_t nor_flash_read(uint32_t offset, void *buf, uint32_t nbytes);
int32_t nor_flash_write(uint32_t offset, void *buf, uint32_t nbytes);
int32_t nor_flash_erase(uint32_t offset, uint32_t size);
uint32_t flash_init();
char* flash_read(u8 addr);
int flash_write(char *buff,u8 addr);
#define FLASH_TYPE			"fota_param"      //ѡ���д��flash����
//#define FLASH_TYPE "nvm"
#endif