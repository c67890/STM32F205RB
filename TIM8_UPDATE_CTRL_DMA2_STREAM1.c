/*
	程序实现内容：通过定时器触发DMA,实现DMA传输的可控性 
	
	程序通过 定时器8 触发 DMA2_Stream1开启，DMA2_Stream1从内存到外设传递数据，每次传递数据的个数为5
	TIM8 每触发一次， DMA2_Stream1 开启一次传输，TIM8 触发100次后，关闭TIM8,清空 外设寄存器。
	
	主函数中循环打印外设寄存器的值，100次触发之内，外设寄存器的数据一直在更新，
	关闭定时器后，外设寄存器中的数据为0，表示TI8关闭后，DMA不再被触发。
	
	注：本测试总TIM8开启的了中断，用来观察和测试使用，实际应用中可以不开中断，TIM8同样会触发启动DMA.
*/ 


unsigned int timer8_times_count = 0;    // 计数用变量 

unsigned int config_CR[5] = {0x06030440,0x06030446,0x06030442,0x06030448,0x06030444}; // 外部数据 

/*
	通过 DMA2_Stream1_DMA_Channel_7 向  DMA2_Stream3的 CR 寄存器中搬运数据 
	注：数据为测试数据，无实际意义，仅用于测试功能 
*/ 

void DMA_Auto_Config_CR(void)
{    
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1|RCC_AHB1Periph_DMA2,ENABLE);	  

	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}                         // 等待DMA可配置 

	DMA_InitStructure.DMA_Channel = DMA_Channel_7;                              // 通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&DMA2_Stream3->CR);   // DMA外设地址(SOURCE ADDR)
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(config_CR);		    // DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                     // 内存到外设

	DMA_InitStructure.DMA_BufferSize = 5;					    // 数据传输量 
	DMA_InitStructure.DMA_PeripheralInc	= DMA_PeripheralInc_Disable;	    // 外设增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // 存储器非增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;     // 外设数据长度:16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;             // 存储器数据长度:16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // 使用循环模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         // 优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;	  

	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 // 存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         // 外设突发单次传输
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
}
void hf_timer8_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = 100-1;
	TIM_TimeBaseStructure.TIM_Prescaler = 60000-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;                    // 为方便观察，配置TIM8 UPDATE 中断 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
	
	TIM_ClearFlag(TIM8,TIM_FLAG_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);                                  // 开中断便于观察 
	TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);

	TIM_DMACmd(TIM8,TIM_DMA_Update,ENABLE);                                     // 开启 TIM8 UPDATE 触发 DMA  ,即 DMA2_Stream1
	TIM_Cmd(TIM8,ENABLE);                                                       // 开启 TIM8
	DMA_Cmd(DMA2_Stream1, ENABLE);                                              // 开启 DMA 
}

void TIM8_UP_TIM13_IRQHandler(void)                                                 // TIM8 UP 溢出中断
{
	if(TIM_GetITStatus(TIM8,TIM_IT_Update)==SET) 
	{
		TIM_Cmd(TIM8, DISABLE);
		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);			    // 清除中断标志位

		printf("TIMER_8 update update update update update update update !\n");

		timer8_times_count++;
		
		if(timer8_times_count>=100)					    // 100次中断后，关闭定时，即停止TIM8 触发 DMA 
		{
			TIM_Cmd(TIM8, DISABLE);
			DMA2_Stream3->CR = 0;					    // 清空外设寄存器 
		}
		else																	// 100次以内，继续触发DMA 
		{
			TIM_Cmd(TIM8, ENABLE);
		}		
	}
}

int main(void) 
{	
	uart5_init();
	DMA_Auto_Config_CR();						
	hf_timer8_init();							

	while(1)
	{	
		printf("DMA2_Stream3->CR = 0x%x\n",DMA2_Stream3->CR);		
		delay_ms(10);	
	}
}

/* 

打印结果： 
					......
					 
	DMA2_Stream3->CR =	0x06030442;
	TIMER_8 update update update update update update update !
	TIMER_8 update update update update update update update !
	TIMER_8 update update update update update update update !
	
	DMA2_Stream3->CR =	0x06030444;
	TIMER_8 update update update update update update update !
	TIMER_8 update update update update update update update !
	TIMER_8 update update update update update update update !

	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;		
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	DMA2_Stream3->CR = 0;
	.......	
*/

