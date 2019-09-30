/*
	����ʵ�����ݣ�ͨ����ʱ������DMA,ʵ��DMA����Ŀɿ��� 
	
	����ͨ�� ��ʱ��8 ���� DMA2_Stream1������DMA2_Stream1���ڴ浽���贫�����ݣ�ÿ�δ������ݵĸ���Ϊ5
	TIM8 ÿ����һ�Σ� DMA2_Stream1 ����һ�δ��䣬TIM8 ����100�κ󣬹ر�TIM8,��� ����Ĵ�����
	
	��������ѭ����ӡ����Ĵ�����ֵ��100�δ���֮�ڣ�����Ĵ���������һֱ�ڸ��£�
	�رն�ʱ��������Ĵ����е�����Ϊ0����ʾTI8�رպ�DMA���ٱ�������
	
	ע����������TIM8���������жϣ������۲�Ͳ���ʹ�ã�ʵ��Ӧ���п��Բ����жϣ�TIM8ͬ���ᴥ������DMA.
*/ 


unsigned int timer8_times_count = 0;    // �����ñ��� 

unsigned int config_CR[5] = {0x06030440,0x06030446,0x06030442,0x06030448,0x06030444}; // �ⲿ���� 

/*
	ͨ�� DMA2_Stream1_DMA_Channel_7 ��  DMA2_Stream3�� CR �Ĵ����а������� 
	ע������Ϊ�������ݣ���ʵ�����壬�����ڲ��Թ��� 
*/ 

void DMA_Auto_Config_CR(void)
{    
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1|RCC_AHB1Periph_DMA2,ENABLE);	  

	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}                         // �ȴ�DMA������ 

	DMA_InitStructure.DMA_Channel = DMA_Channel_7;                              // ͨ��ѡ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&DMA2_Stream3->CR);   // DMA�����ַ(SOURCE ADDR)
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(config_CR);				// DMA �洢��0��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                     // �ڴ浽����

	DMA_InitStructure.DMA_BufferSize = 5;										// ���ݴ����� 
	DMA_InitStructure.DMA_PeripheralInc	= DMA_PeripheralInc_Disable;			// ��������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // �洢��������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;     // �������ݳ���:16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;             // �洢�����ݳ���:16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                             // ʹ��ѭ��ģʽ 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                         // ���ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;	  

	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                 // �洢��ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;         // ����ͻ�����δ���
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

	NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;                    // Ϊ����۲죬����TIM8 UPDATE �ж� 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
	
	TIM_ClearFlag(TIM8,TIM_FLAG_Update);
	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);                                  // ���жϱ��ڹ۲� 
	TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);

	TIM_DMACmd(TIM8,TIM_DMA_Update,ENABLE);                                     // ���� TIM8 UPDATE ���� DMA  ,�� DMA2_Stream1
	TIM_Cmd(TIM8,ENABLE);                                                       // ���� TIM8
	DMA_Cmd(DMA2_Stream1, ENABLE);                                              // ���� DMA 
}

void TIM8_UP_TIM13_IRQHandler(void)                                             // TIM8 UP ����ж�
{
	if(TIM_GetITStatus(TIM8,TIM_IT_Update)==SET) 
	{
		TIM_Cmd(TIM8, DISABLE);
		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);			                    // ����жϱ�־λ

		printf("TIMER_8 update update update update update update update !\n");

		timer8_times_count++;
		
		if(timer8_times_count>=100)												// 100���жϺ󣬹رն�ʱ����ֹͣTIM8 ���� DMA 
		{
			TIM_Cmd(TIM8, DISABLE);
			DMA2_Stream3->CR = 0;												// �������Ĵ��� 
		}
		else																	// 100�����ڣ���������DMA 
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

��ӡ����� 
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

