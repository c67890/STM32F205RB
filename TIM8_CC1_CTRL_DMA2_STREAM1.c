unsigned int config_CR[5] = {0x06030441,0x06030446,0x06030442,0x06030448,0x06030444};

void hf_timer8_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = 100-1;												// 1000us *100 = 0.1s即为10 Hz		   counter的次数(多少个counter后进中断)
	TIM_TimeBaseStructure.TIM_Prescaler = 6000-1;											// 60M Hz /60000 = 1000 ,即每counter一次的时间为1ms
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;												// TIM8_CH1 PC6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;											// 复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 5;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM8,ENABLE);	

	NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;										// DMA IT CONFIG
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  

	TIM_ClearFlag(TIM8,TIM_FLAG_CC1);
	TIM_ITConfig(TIM8, TIM_FLAG_CC1, ENABLE);												// 开中断

	TIM_DMACmd(TIM8,TIM_DMA_CC1,ENABLE);
	TIM_Cmd(TIM8,ENABLE);
	DMA_Cmd(DMA2_Stream2, ENABLE); 
}

void DMA_Auto_Config_CR(void)
{    
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1|RCC_AHB1Periph_DMA2,ENABLE);	  

	DMA_DeInit(DMA2_Stream2);
	while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE){}									// 等待DMA可配置 

	DMA_InitStructure.DMA_Channel				 = DMA_Channel_7;						// 通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr	 = (uint32_t)(&DMA2_Stream3->CR);		// DMA外设地址(SOURCE ADDR)
	DMA_InitStructure.DMA_Memory0BaseAddr		 = (uint32_t)(config_CR);				// DMA 存储器0地址
	DMA_InitStructure.DMA_DIR					 = DMA_DIR_MemoryToPeripheral;			// 内存到外设

	DMA_InitStructure.DMA_BufferSize			 = 5;									// 数据传输量 
	DMA_InitStructure.DMA_PeripheralInc			 = DMA_PeripheralInc_Disable;			// 外设增量模式
	DMA_InitStructure.DMA_MemoryInc				 = DMA_MemoryInc_Enable;				// 存储器非增量模式
	DMA_InitStructure.DMA_PeripheralDataSize	 = DMA_PeripheralDataSize_Word;			// 外设数据长度:16位
	DMA_InitStructure.DMA_MemoryDataSize			 = DMA_MemoryDataSize_Word;			// 存储器数据长度:16位
	DMA_InitStructure.DMA_Mode 					 = DMA_Mode_Circular;					// 使用循环模式 
	DMA_InitStructure.DMA_Priority 				 = DMA_Priority_High;					// 优先级
	DMA_InitStructure.DMA_FIFOMode 				 = DMA_FIFOMode_Disable;	  

	DMA_InitStructure.DMA_FIFOThreshold			 = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst			 = DMA_MemoryBurst_Single;				 // 存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst		 = DMA_PeripheralBurst_Single;			 // 外设突发单次传输
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
}

int main(void) 
{	
	int i = 0;
	
	uart5_init();
	DMA_Auto_Config_CR1();						// dma2_stream2_channel7	config
	hf_timer8_init1();							// timer8 
	while(1)
	{	
		printf("DMA2_Stream3->CR 1111= 0x%x\n",DMA2_Stream3->CR);		
		delay_ms(100);	
	}
}

void TIM8_CC_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM8,TIM_IT_CC1)==SET) //溢出中断
	{
		TIM_Cmd(TIM8, DISABLE);
		TIM_ClearITPendingBit(TIM8,TIM_IT_CC1);			/*清除中断标志位*/
		

		timer8_times_count++;
		if(timer8_times_count>=100)
		{
			TIM_Cmd(TIM8, DISABLE);
			DMA2_Stream3->CR = 0;
			printf("TIMER_8 CLOSED CLOSED CLOSED CLOSED CLOSED !\n");
		}
		else
		{
			TIM_Cmd(TIM8, ENABLE);
		}		
	}
}

/*
打印结果： 
					......
			DMA2_Stream3->CR 1111= 0x6030444
			DMA2_Stream3->CR 1111= 0x6030444
			DMA2_Stream3->CR 1111= 0x6030448
			DMA2_Stream3->CR 1111= 0x6030448
			DMA2_Stream3->CR 1111= 0x6030442
			DMA2_Stream3->CR 1111= 0x6030442
			DMA2_Stream3->CR 1111= 0x6030442
			DMA2_Stream3->CR 1111= 0x6030446
			TIMER_8 CLOSED CLOSED CLOSED CLOSED CLOSED !
			DMA2_Stream3->CR 1111= 0x0
			DMA2_Stream3->CR 1111= 0x0
			DMA2_Stream3->CR 1111= 0x0
			DMA2_Stream3->CR 1111= 0x0
			DMA2_Stream3->CR 1111= 0x0
					......			
*/
