#ifndef __APT8L16_H__
#define __APT8L16_H__

#include "stm32l4xx_hal.h"
#include "stdio.h"

#define WRITE_ADDR 0xae
#define READ_ADDR 0xaf

#define APT_SDA_HIGH  HAL_GPIO_WritePin(GPIOA, SDA1_Pin, GPIO_PIN_SET)
#define APT_SDA_LOW   HAL_GPIO_WritePin(GPIOA, SDA1_Pin, GPIO_PIN_RESET)

#define APT_SCL_HIGH 	HAL_GPIO_WritePin(GPIOA, SCL1_Pin, GPIO_PIN_SET)
#define APT_SCL_LOW   HAL_GPIO_WritePin(GPIOA, SCL1_Pin, GPIO_PIN_RESET)

#define APT_SDA_STATUS HAL_GPIO_ReadPin(GPIOA,SDA1_Pin)


#define KEY0_DISA 0x80  //k00
#define KEY1_DISA 0x02  //k00
#define KEY2_DISA 0x01  //k00
#define KEY3_DISA 0x08  //k01
#define KEY4_DISA 0x08  //k00
#define KEY5_DISA 0x04  //k00
#define KEY6_DISA 0x04  //k01
#define KEY7_DISA 0x10  //k00
#define KEY8_DISA 0x20  //k00
#define KEY9_DISA 0x02  //k01
#define KEY10_DISA 0x40 //k00
#define KEY11_DISA 0x01 //01

#define KEYTOUCH_DISA 0x10

#define KDR0_CH_10_EN      (uint8_t)~(KEY10_DISA)
#define KDR1_CH_10_EN      (uint8_t)~(KEYTOUCH_DISA)

#define KDR0_CH_10_11_EN   (uint8_t)~(KEY10_DISA)
#define KDR1_CH_10_11_EN    (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_1_10_11_EN   (uint8_t)~(KEY1_DISA|KEY10_DISA)
#define KDR1_CH_1_10_11_EN   (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_1_2_10_11_EN   (uint8_t)~(KEY1_DISA|KEY2_DISA|KEY10_DISA)
#define KDR1_CH_1_2_10_11_EN   (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_1_2_3_10_11_EN  (uint8_t)~(KEY1_DISA|KEY2_DISA|KEY10_DISA)
#define KDR1_CH_1_2_3_10_11_EN   (uint8_t)~(KEY3_DISA|KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_1_2_3_4_10_11_EN   (uint8_t)~(KEY1_DISA|KEY2_DISA|KEY4_DISA|KEY10_DISA)
#define KDR1_CH_1_2_3_4_10_11_EN   (uint8_t)~(KEY3_DISA|KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_0_1_10_11_EN       (uint8_t)~(KEY0_DISA|KEY1_DISA|KEY10_DISA)
#define KDR1_CH_0_1_10_11_EN       (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_0_1_2_10_11_EN      (uint8_t)~(KEY0_DISA|KEY1_DISA|KEY2_DISA|KEY10_DISA)
#define KDR1_CH_0_1_2_10_11_EN       (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_0_1_2_3_10_11_EN     (uint8_t)~(KEY0_DISA|KEY1_DISA|KEY2_DISA|KEY10_DISA)
#define KDR1_CH_0_1_2_3_10_11_EN     (uint8_t)~(KEY3_DISA|KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_0_1_2_3_4_5_10_11_EN  (uint8_t)~(KEY0_DISA|KEY1_DISA|KEY2_DISA|KEY4_DISA|KEY5_DISA|KEY10_DISA)
#define KDR1_CH_0_1_2_3_4_5_10_11_EN   (uint8_t)~(KEY3_DISA|KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_2_8_10_11_EN       (uint8_t)~(KEY2_DISA|KEY8_DISA|KEY10_DISA)
#define KDR1_CH_2_8_10_11_EN      (uint8_t)~(KEY11_DISA|KEYTOUCH_DISA)

#define KDR0_CH_2_8_10_EN          (uint8_t)~(KEY2_DISA|KEY8_DISA|KEY10_DISA)
#define KDR1_CH_2_8_10_EN     (uint8_t)~(KEYTOUCH_DISA)

#define KDR0_ALL_EN 0x00
#define KDR1_ALL_EN 0x00

#define KDR0_CH_ALL_DISA 0xff
#define KDR1_CH_ALL_DISA (uint8_t)~(KEYTOUCH_DISA)
 
void	APT_IIC_Start(void);
void  APT_IIC_Stop(void);
void  APT_IIC_WaitAck(void);
void  APT_IIC_SendACK(uint8_t Ack);
void  APT_IIC_Write8bit(uint8_t WrData);
uint8_t  APT_IIC_Read8bit(void);
void  APT_WriteByte(uint8_t APTWrAdd,uint8_t APTWrData);

void  APT_WriteChipReg(uint8_t ChipRegAdd,uint8_t ChipRegValueA);
uint8_t  APT_ReadChipReg(uint8_t APTRdAdd);

void APT_ChipRegInit(void);
void APT_ChipRegSet(void);
void APT_Init(void);

void  APT_ScanTouchKey(void);

void APT_KeyEnableSel(uint8_t kdr0,uint8_t kdr1);

void APT_WORK_125HZ_VAL(void);

#endif  //__APT8L16_H__
