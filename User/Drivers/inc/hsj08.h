#ifndef __HSJ08_H__
#define __HSJ08_H__

#include "stm32l4xx_hal.h"

#define CAP_OPEN 0x01
#define CAP_CLOSE 0x02

void hsj08_init(void);

void LOCK(void);
void UNLOCK(void);


void fp_cap_drv(uint8_t director);


#endif // __HSJ08_H__
