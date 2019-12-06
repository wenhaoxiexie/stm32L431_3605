#ifndef __FPCST90X_H__
#define __FPCST90X_H__

#include "stm32l4xx_hal.h"
#include "stdio.h"
#include "global.h"

#define PAKEGE_HEAD_LEN 2
#define PAKEGE_DATA_LEN 6
// 包头和包尾
#define PKG_HEAD          0xf5       
#define PKG_TAIL          0xf5

//检测手指
#define PS_Detect         0x11

//比对指纹
#define PS_Compare        0x0c

//删除和清空
#define PS_Delete         0x04
#define PS_Empty          0x05    

typedef enum
{
  FP_OK       = 0x00,
  FP_ERROR    = 0x01,
  FP_BUSY     = 0x02,
  FP_TIMEOUT  = 0x03
}FP_StatusTypeDef;

void fp_power_on(void);
void fp_power_off(void);
void fp_sleep(void);

uint8_t fp_add(uint8_t permision);
uint8_t fp_delete(uint8_t index,uint8_t permision);

FP_StatusTypeDef fp_tarns_recv_command(uint8_t Type, uint8_t *pDataIn, uint16_t *Datalen, uint8_t *pDataOut, uint8_t *pErr, uint16_t TimeOutMs);

#endif  // __FPCST90X_H__
