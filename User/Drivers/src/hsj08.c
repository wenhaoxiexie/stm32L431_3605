#include "hsj08.h"
#include "delay.h"
#include "global.h"


void LOCK(void)
{
	 __disable_irq();
	 HAL_GPIO_WritePin(pwm1_GPIO_Port, pwm1_Pin, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(pwm2_GPIO_Port, pwm2_Pin, GPIO_PIN_RESET);
	 sys_delay_us(150000);
	
	 HAL_GPIO_WritePin(pwm1_GPIO_Port, pwm1_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(pwm2_GPIO_Port, pwm2_Pin, GPIO_PIN_RESET);
	 __enable_irq();
}

void UNLOCK(void)
{
	 __disable_irq();
	 HAL_GPIO_WritePin(pwm1_GPIO_Port, pwm1_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(pwm2_GPIO_Port, pwm2_Pin, GPIO_PIN_SET);
	 sys_delay_us(150000);
	 HAL_GPIO_WritePin(pwm1_GPIO_Port, pwm1_Pin, GPIO_PIN_RESET);
	 HAL_GPIO_WritePin(pwm2_GPIO_Port, pwm2_Pin, GPIO_PIN_RESET);
	 __enable_irq();
}

static void fp_cap_stop(void)
{
	HAL_GPIO_WritePin(fpCap_drv1_GPIO_Port, fpCap_drv1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(fpCap_drv2_GPIO_Port, fpCap_drv2_Pin, GPIO_PIN_RESET);
	//关闭红外检测
	HAL_GPIO_WritePin(irButton_GPIO_Port, irButton_Pin, GPIO_PIN_RESET);
}

void fp_cap_drv(uint8_t director)
{
	uint16_t delayCount;
	GPIO_PinState status;
	
	// 打开红外检测
	HAL_GPIO_WritePin(irButton_GPIO_Port, irButton_Pin, GPIO_PIN_SET);
	
	// 判断齿轮转动方向
	if(director==CAP_OPEN)
	{
		// 获取当前齿轮位置
		status = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10);
		if(status)
		{
			debug_printf("GPIO_PIN_10... \r\n");
				return ;
		}
		// 转动齿轮
		HAL_GPIO_WritePin(fpCap_drv1_GPIO_Port, fpCap_drv1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(fpCap_drv2_GPIO_Port, fpCap_drv2_Pin, GPIO_PIN_RESET);
	}
	else if(director==CAP_CLOSE)
	{
		// 获取当前齿轮位置
		status = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11);
		if(status)
		{
			debug_printf("GPIO_PIN_11... \r\n");
			return ;
		}
			
		// 转动齿轮
		HAL_GPIO_WritePin(fpCap_drv1_GPIO_Port, fpCap_drv1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(fpCap_drv2_GPIO_Port, fpCap_drv2_Pin, GPIO_PIN_SET);
	}
	delayCount= 150;
	do{
		if(director==CAP_OPEN)
			status = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10);
		else if(director==CAP_CLOSE)
			status = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11);
		if(status)
		{
			fp_cap_stop();
			return ;
		}
	 sys_delay_us(1000);
	}while(delayCount--);
	fp_cap_stop();
}
