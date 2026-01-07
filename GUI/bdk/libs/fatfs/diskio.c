/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs     (C)ChaN, 2019                */
/* SD Card Tester GUI implementation                                      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "ff.h"
#include <storage/sd.h>
#include <storage/sdmmc.h>

/* External SD storage from BDK */
extern sdmmc_storage_t sd_storage;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status(BYTE pdrv) {
  switch (pdrv) {
  case DRIVE_SD:
    if (sd_storage.initialized)
      return 0;
    return STA_NOINIT;
  default:
    break;
  }
  return STA_NOINIT | STA_NODISK;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize(BYTE pdrv) {
  switch (pdrv) {
  case DRIVE_SD:
    if (sd_storage.initialized)
      return 0;
    /* SD should already be initialized by main */
    return STA_NOINIT;
  default:
    break;
  }
  return STA_NOINIT | STA_NODISK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
  switch (pdrv) {
  case DRIVE_SD:
    if (!sd_storage.initialized)
      return RES_NOTRDY;
    if (sdmmc_storage_read(&sd_storage, sector, count, buff))
      return RES_OK;
    return RES_ERROR;
  default:
    break;
  }
  return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
  switch (pdrv) {
  case DRIVE_SD:
    if (!sd_storage.initialized)
      return RES_NOTRDY;
    if (sdmmc_storage_write(&sd_storage, sector, count, (void *)buff))
      return RES_OK;
    return RES_ERROR;
  default:
    break;
  }
  return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  switch (pdrv) {
  case DRIVE_SD:
    if (!sd_storage.initialized)
      return RES_NOTRDY;
    switch (cmd) {
    case CTRL_SYNC:
      return RES_OK;
    case GET_SECTOR_COUNT:
      *(DWORD *)buff = sd_storage.sec_cnt;
      return RES_OK;
    case GET_SECTOR_SIZE:
      *(WORD *)buff = 512;
      return RES_OK;
    case GET_BLOCK_SIZE:
      *(DWORD *)buff = 32768; /* 32KB erase block */
      return RES_OK;
    default:
      return RES_PARERR;
    }
  default:
    break;
  }
  return RES_PARERR;
}

DRESULT disk_set_info(BYTE pdrv, BYTE cmd, void *buff) {
  (void)pdrv;
  (void)cmd;
  (void)buff;
  return RES_OK;
}
