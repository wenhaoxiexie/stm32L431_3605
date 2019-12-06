#include "fpcst90x.h"
#include "usart.h"
#include "global.h"
#include "flash.h"
#include "user_operator.h"
#include "wifi_8266.h"
#include "win6170.h"
#include "apt8l16.h"
#include "hsj08.h"

extern uint8_t finger_press_flag;
extern KEY_STATE get_cur_number;
extern uint8_t usart2_rx[1];

int32_t recvDataOffSet;
uint8_t usart2TxBuff[1024];

void fp_power_on(void)
{
	uint16_t delay_count;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// 1.0屏蔽中断响应
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	
	// 2.0配置串口为GPIO口，并输出低电平
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);
	// 3.0延时5ms
	sys_delay_us(5000);
	// 4.0上电
	HAL_GPIO_WritePin(GPIOB, FPPower_Pin, GPIO_PIN_SET);
	// 5.0延时5ms
	sys_delay_us(5000);
	// 6.0配置串口
	__HAL_RCC_USART2_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	
	MX_USART2_UART_Init();
	
	// 启动中断接收
	HAL_UART_Receive_IT(&huart2, usart2_rx, 1);
	
	// 等待接收到0X55
	delay_count=0;
	while(usart2_rx[0]!=0x55&&delay_count<100)
	{
		delay_count++;
		sys_delay_us(1000);
	}
	debug_printf(" usart2_rx[0]:%02x : %d : %d\r\n",usart2_rx[0],delay_count,usart2_rx_len);
}

void fp_power_off(void)
{

	// 1.0主控配置串口为GPIO，并输出低电平
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);
	
	// 2.0延时5ms
	sys_delay_us(5000);
	
	// 3.0关闭电源
	HAL_GPIO_WritePin(GPIOB, FPPower_Pin, GPIO_PIN_RESET);
}

void fp_sleep(void)
{
	// 清除中断位
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
	
	// 开启TOUCH 中断
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
		// 断电
	fp_power_off();
}

// 指纹注册
uint8_t fp_add(uint8_t permision)
{
	uint8_t Revalue;
	uint8_t flash_index;
	char lock_time[32];
	uint8_t delay;
	uint8_t regCount=0;
	uint8_t locCount=0;
	uint8_t pPakege[128];
	uint16_t pLen;
	uint8_t pResult;
	FP_StatusTypeDef reValue;
	uint8_t PS_RegMode[3]={0x01,0x02,0x03};
	FP_STATUS_T fpStatus;
	
	send_bytes(KEY10_ON);
	APT_KeyEnableSel(KDR0_CH_10_EN,KDR1_CH_10_EN);
	
	stm_flash_read(ADDR_FLASH_PAGE_58,(uint8_t*)finger_flash,FINGER_SIZE);
	stm_flash_read(ADDR_FLASH_PAGE_100,(uint8_t*)get_cur_info,CUR_INDEX_SIZE);
	
	//打开指纹盖
	fp_cap_drv(CAP_OPEN);
	
	// 清除中断位
	//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);
	// 开启TOUCH 中断
	//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	fp_power_on();
	
	debug_printf(" get_cur_info->cur_fingerprint_index:%d \r\n",get_cur_info->cur_fingerprint_index);

	// 验证是否超过最大数100
	if(get_cur_info->cur_fingerprint_index>=FINGER_NUM)
	{
			fpStatus = LOCK_MAX;
			goto _END_;
	}
	
	// 保存指纹到空闲位置 指纹从下标0开始。
	for(flash_index=0;flash_index<get_cur_info->cur_fingerprint_index;flash_index++)
	{
		if(finger_flash[flash_index].del_flag==1)
			break;
	}
	
	debug_printf(" the %d finger user is coming \r\n",flash_index);
	
	regCount=3;
	locCount=0;
	while(locCount<regCount)
	{	
		// 请按手指
		display_finger_press();
		set_win6170(18);
		
		delay=30;
		do{
			if(get_cur_number.key_prees)
			{
					get_cur_number.key_prees=0;
					if(get_cur_number.number==10)
					{
							fpStatus = LOCK_OK;
							goto _END_;
					}
			}
			// 检测手指
			pPakege[0]=0;
			pPakege[1]=0;
			pPakege[2]=0;
			pPakege[3]=0;
			pLen = 4;
			reValue = fp_tarns_recv_command(PS_Detect, pPakege, &pLen, pPakege, &pResult,500);
			debug_printf("PS_Detect : %d:%d \r\n",reValue,pResult);
			if(reValue==FP_OK&&pResult==0x0)
			{
				if(pPakege[3]==0x0)
						break;
			}
			sys_delay_us(100000);
		}while(--delay);
		if(delay==0)
		{
			fpStatus = LOCK_OUTTIME;
			goto _END_;
		}
	
		//注册指纹
		pPakege[0]= 0;
		pPakege[1]= flash_index+1;
		pPakege[2]= 1;
		pPakege[3]= 0;
		pLen=4;
		reValue = fp_tarns_recv_command(PS_RegMode[locCount], pPakege,&pLen, pPakege,&pResult,2000);
		debug_printf("PS_RegMode : %d:%d \r\n",reValue,pResult);
		if(reValue!=FP_OK||pResult!=0x00)
		{
			if(reValue==FP_OK&&pResult==0x07)
			{
					fpStatus = LOCK_EXIT;
					goto _END_;
			}
			fpStatus = LOCK_ERROR;
			goto _END_;
		}
		// 请抬起手指
		display_finger_realse();
		set_win6170(20);
		delay=8;
		do{
			if(get_cur_number.key_prees)
			{
					get_cur_number.key_prees=0;
					if(get_cur_number.number==10)
					{
							fpStatus = LOCK_OK;
							goto _END_;
					}
			}
			// 检测手指
			pPakege[0]=0;
			pPakege[1]=0;
			pPakege[2]=0;
			pPakege[3]=0;
			pLen = 4;
			reValue = fp_tarns_recv_command(PS_Detect, pPakege, &pLen, pPakege, &pResult,500);
			debug_printf("PS_Detect : %d:%d:%d \r\n",reValue,pLen,pResult);
			if(reValue==FP_OK&&pResult==0x0)
			{
					if(pPakege[3]==0x01)
						break;
			}
			sys_delay_us(500000);
		}while(--delay);
		if(delay==0)
		{
				fpStatus = LOCK_OUTTIME;
				goto _END_;
		}
		
		locCount++;
		debug_printf("locCount: %d \r\n",locCount);
	}	
	
		set_win6170(42);
		display_check_sucessful(flash_index+1,permision,FINGER_TYPE);
		
		finger_flash[flash_index].index=flash_index+1;
		finger_flash[flash_index].user_type=FINGER_TYPE;
		finger_flash[flash_index].user_permision=permision;
		finger_flash[flash_index].del_flag=0;
		
		stm_flash_erase_write(ADDR_FLASH_PAGE_58,(uint64_t*)finger_flash,FINGER_SIZE);
		
		debug_printf(" flash_index:%d get_cur_info->cur_fingerprint_index:%d\r\n",flash_index+1,get_cur_info->cur_fingerprint_index);
		
		get_cur_info->cur_fingerprint_index++;
		get_cur_info->cur_index_number++;
		
		stm_flash_erase_write(ADDR_FLASH_PAGE_100,(uint64_t*)get_cur_info,CUR_INDEX_SIZE);
		if(get_cur_info->cur_log_number>=LOG_NUM)
			get_cur_info->cur_log_number=0;
	
		HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
		sys_log_record(flash_index+1,ADD_USER_OPR,permision,FINGER_TYPE,get_cur_info->cur_log_number);
		get_cur_info->cur_log_number++;
		
		stm_flash_erase_write(ADDR_FLASH_PAGE_100,(uint64_t*)get_cur_info,CUR_INDEX_SIZE);
		// 新用户上报信息
		stm_flash_read(ADDR_FLASH_PAGE_70,(uint8_t*)wifi_flash,WIFI_SIZE);				
//		wifi_power_on();			
		Revalue = wifi_flash->index;
		debug_printf("Revalue: %d \r\n",Revalue);
		
		sprintf(lock_time,"20%02d-%02d-%02d %02d:%02d:%02d",sdatestructure.Year,sdatestructure.Month,sdatestructure.Date,
																																						 stimestructure.Hours,stimestructure.Minutes,stimestructure.Seconds);
		wifi_flash[Revalue].exist=1;
		sprintf(wifi_flash[Revalue].upload_info,"%d%%%d%%%d%%%d%%%d%%%s%%",ADD_USER_UPLOAD,flash_index+1,FINGER_TYPE,permision,
																																				 ADD_USER_OPR,lock_time);
		if(wifi_flash->index<99)
			wifi_flash->index++;
		
		// 保存上报信息到flash
		stm_flash_erase_write(ADDR_FLASH_PAGE_70,(uint64_t*)wifi_flash,WIFI_SIZE);
		
		sys_delay_us(1000000);
		
		fpStatus = LOCK_OK;
		goto _END_;
	
_END_:	
		// 1.0屏蔽中断响应
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		
		if(finger_press_flag)
			finger_press_flag=0;
		return fpStatus;
		
}

uint8_t fp_delete(uint8_t index,uint8_t permision)
{
	uint8_t i;

	uint8_t wifi_index;
	char lock_time[32];
	uint8_t pPakege[128];
	uint16_t pLen;
	uint8_t pResult;
	FP_StatusTypeDef reValue;

	fp_power_on();
	
	stm_flash_read(ADDR_FLASH_PAGE_58,(uint8_t*)finger_flash,FINGER_SIZE);
	stm_flash_read(ADDR_FLASH_PAGE_100,(uint8_t*)get_cur_info,CUR_INDEX_SIZE);
	
	pPakege[0]=0;
	pPakege[1]=index;
	pPakege[2]=0;
	pPakege[3]=0;
	pLen = 4;
	reValue = fp_tarns_recv_command(PS_Delete, pPakege, &pLen, pPakege, &pResult,500);

	if(reValue!=FP_OK||pResult!=0x00)
	{
		sys_delay_us(1000000);
		return LOCK_ERROR;
	}

	// 更新FLASH 
	finger_flash[index-1].del_flag=1;
	finger_flash[index-1].index=0;
	
	stm_flash_erase_write(ADDR_FLASH_PAGE_58,(uint64_t*)finger_flash,FINGER_SIZE);
	
	get_cur_info->cur_fingerprint_index--;
	get_cur_info->cur_index_number--;
	
	stm_flash_erase_write(ADDR_FLASH_PAGE_100,(uint64_t*)get_cur_info,CUR_INDEX_SIZE);
	
	if(get_cur_info->cur_log_number>=LOG_NUM)
			get_cur_info->cur_log_number=0;
	
	HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
	sys_log_record(index,DEL_USER_OPR,permision,FINGER_TYPE,get_cur_info->cur_log_number);
	get_cur_info->cur_log_number++;
	
	stm_flash_erase_write(ADDR_FLASH_PAGE_100,(uint64_t*)get_cur_info,CUR_INDEX_SIZE);
	// 删除上报信息 
	stm_flash_read(ADDR_FLASH_PAGE_70,(uint8_t*)wifi_flash,WIFI_SIZE);
//	wifi_power_on();
	wifi_index = wifi_flash->index;
	wifi_flash[wifi_index].exist=1;
	sprintf(lock_time,"20%02d-%02d-%02d %02d:%02d:%02d",sdatestructure.Year,sdatestructure.Month,sdatestructure.Date,
																																						 stimestructure.Hours,stimestructure.Minutes,stimestructure.Seconds);
	sprintf(wifi_flash[wifi_index].upload_info,"%d%%%d%%%d%%%d%%%d%%",DELETE_USER_UPLOAD,index,
																																										FINGER_TYPE,permision,DEL_USER_OPR);
	debug_printf(" upload_info:%s \r\n",wifi_flash[i].upload_info);
	
	if(wifi_flash->index<99)
		wifi_flash->index++;
	
	// 保存上报信息到flash
	stm_flash_erase_write(ADDR_FLASH_PAGE_70,(uint64_t*)wifi_flash,WIFI_SIZE);
	
	fp_power_off();
	display_finger_del_sucess();
	set_win6170(42);
	sys_delay_us(1000000);
	return LOCK_OK;
}

static void fp_str_copy(uint8_t *Src, uint8_t *pDes, uint32_t len)
{
	uint32_t i;
	for(i=0;i<len;i++)
		Src[i]=pDes[i];
}

static void fp_trans_buff_clear(void)
{
	uint16_t i;
	for(i=0;i<USART2_REC_LEN;i++)
	{
		usart2TxBuff[i]=0;
	}
}

static void fp_recev_buff_clear(void)
{
	uint16_t i;
   
	for (i=0;i<USART2_REC_LEN;i++)
	   usart2_rx_buf[i]=0;
	usart2_rx_len=0;
	recvDataOffSet=0;
}


static uint8_t caculator_hex_sum(uint8_t*pBuff,uint8_t len)
{
	uint8_t i;
	uint8_t nwCHK = 0;
	
	debug_printf("len:%d \r\n",len);
	
	for(i = 1; i <len; i++)
		nwCHK ^= (pBuff[i]&0xff);
		
	return(nwCHK);
}

static void MakePackageHead(uint8_t* pHead,uint8_t Type)
{
	pHead[0] = PKG_HEAD;
	pHead[1] = Type;
}

static FP_StatusTypeDef fp_usart2_send(uint8_t Type, uint8_t *pData, uint16_t Datalen)
{
	uint16_t len;
	
	MakePackageHead(usart2TxBuff,Type);
	
	len = PAKEGE_HEAD_LEN;

	fp_str_copy(&usart2TxBuff[PAKEGE_HEAD_LEN],pData,Datalen);
	len += Datalen;
	
	usart2TxBuff[len] = caculator_hex_sum(usart2TxBuff,len);
	len+=1;
	usart2TxBuff[len] = PKG_TAIL;
	len+=1;
	
	// 测试打印信息
	for(uint16_t i=0;i<len;i++)
		printf("%02x ",usart2TxBuff[i]);
	printf("\r\n");
	if(HAL_UART_Transmit(&huart2, usart2TxBuff, len,100)==HAL_OK)
		return FP_OK;
	else
		return FP_ERROR;
}

static FP_StatusTypeDef checkout_sum_tool(uint8_t* pBuff,uint8_t len)
{
	uint8_t reSum,caSum;
	
	reSum = pBuff[len-2];
	
	caSum = caculator_hex_sum(pBuff,len-2);
	debug_printf("%x:%x\r\n",reSum,caSum);
	if(caSum!=reSum)
		return FP_ERROR;
	else
		return FP_OK;
}

static FP_StatusTypeDef usart2_recv_data(uint8_t* pBuf,uint8_t dataLen,uint16_t timeOut)
{
	uint16_t i;
	uint16_t delayCount;
	
	delayCount=timeOut;
	do{
		if(usart2_rx_len-recvDataOffSet>=dataLen)
		{
			for(i=0;i<dataLen;i++)
			{
				pBuf[i]=usart2_rx_buf[i+recvDataOffSet];
			}
			recvDataOffSet+=dataLen;
			
			return FP_OK;
		}
		sys_delay_us(1000);
	}while(--delayCount);
	return FP_TIMEOUT;
}

static FP_StatusTypeDef fp_pakge_recv_parse(uint8_t* pPakege,uint16_t timeOut,uint8_t* pResult)
{
	uint8_t reBuff[16];
	FP_StatusTypeDef reValue;
	
	
	if(pPakege==NULL||pResult==NULL)
		return FP_ERROR;

	do{
		reValue = usart2_recv_data(reBuff,1,timeOut);
		if(reValue!=FP_OK)
			return FP_TIMEOUT;
		if(reBuff[0]==PKG_HEAD)
		{
			reValue = usart2_recv_data(&reBuff[1],1,timeOut);
			if(reValue!=FP_OK)
				return FP_TIMEOUT;
			if(reBuff[1]==usart2TxBuff[1])
			{
				pPakege[0]=PKG_HEAD;
        pPakege[1]=reBuff[1];
				break;
			}
		}
	}while(1);
	
	// 接收包数据
	reValue = usart2_recv_data(&pPakege[2],PAKEGE_DATA_LEN, timeOut);
	if (reValue != FP_OK)
			return FP_TIMEOUT;
	
	for(uint8_t i=0;i<8;i++)
	{
		printf("%02x ",pPakege[i]);
	}
	printf("\r\n");
	//验证校验和	
	reValue = checkout_sum_tool(pPakege,PAKEGE_DATA_LEN+2);	
	if(reValue!=FP_OK)
		return FP_ERROR;

	*pResult = pPakege[4];
	
	return FP_OK;
}

FP_StatusTypeDef fp_tarns_recv_command(uint8_t Type, uint8_t *pDataIn, uint16_t* Datalen, uint8_t *pDataOut, uint8_t *pErr, uint16_t TimeOutMs)
{
	
	uint8_t reResult;
	
	FP_StatusTypeDef reValue;
	
	fp_trans_buff_clear();
	fp_recev_buff_clear();
	
	reValue = fp_usart2_send(Type,pDataIn,*Datalen);
	if(reValue!=FP_OK)
		return FP_ERROR;
	
	reValue = fp_pakge_recv_parse(pDataOut,TimeOutMs,&reResult);
	if(reValue!=FP_OK)
	{
		return FP_ERROR;
	}
	
	*pErr = reResult;
	
	return FP_OK;
}

