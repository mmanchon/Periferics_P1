/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

/* Include core modules */
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "tm_stm32f4_rng.h"
/* Include my libraries here */



/* Variables globales */
long timer_value;
/*
 * Function for scale a number to the desired range.
 */
long map(int rand, int min, int max)
{
  return (rand % (max + 1 - min)) + min;
}

void delay(int counter)
{
	int i;
	for (i = 0; i < counter * 10000; i++) {}
}

void TIM_ResetCounter(TIM_TypeDef* TIMx)
{
  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx));

  /* Reset the Counter Register value */
  TIMx->CNT = 0;
}
void TM_LEDS_PWM_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Clock for GPIOD */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Alternating functions for pins */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

    /* Set pins */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void initLed3(GPIO_InitTypeDef gpio){
		//RCC -> Reset and Clock Control
		// All GPIO's are on AHB1 bus.
		//GPIOG port porque los LEDs estan conectados a los pins PG13 y PG14 (supongo que estructura interna)
		//The function below enables the clock
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
		GPIO_StructInit(&gpio); //supongo inicializa la structura
		/*
		 * 	GPIO_Pin: Choose pins you will use for set settings
			GPIO_Mode: Choose mode of operation. Look down for options
			GPIO_OType: Choose output type
			GPIO_PuPd: Choose pull resistor
			GPIO_Speed: Choose pin speed
		 *
		 */
		gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_1;
		gpio.GPIO_Mode = GPIO_Mode_OUT;
		//una vez digamos que hemos indicado que "usaremos" en las linias anteriores
		//ahora se lo indicamos por asi decirlo, a la placa
		GPIO_Init(GPIOG, &gpio);
}


void initButton(GPIO_InitTypeDef gpio){
	//User button connected to PAO
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    //El pin 0
	gpio.GPIO_Pin = GPIO_Pin_0;
    //mode input
	gpio.GPIO_Mode = GPIO_Mode_IN;
    //output type push-pull (supongo que presionar y soltar para detectar que lo ha clicado)
	gpio.GPIO_OType = GPIO_OType_PP;
	//Añadimos un pull down para que
    gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
    //Pin speed 100MHz
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    //Initialize pins
    GPIO_Init(GPIOA, &gpio);

}

/**
 *
 * 500 periodes i 13 presc.
 * 64p i 10presc--> Periode de 50us
 *
 */
void TIM2_INT_Init()
{
    // Enable clock for TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // TIM2 initialization overflow every 500ms
    // TIM2 by default has clock of 84MHz
    // Here, we must set value of prescaler and period,
    // so update event is 0.5Hz or 500ms
    // Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) *
    // (TIM_Period + 1))
    // Update Event (Hz) = 84MHz / ((4199 + 1) * (9999 + 1)) = 0.5 Hz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 9;
    TIM_TimeBaseInitStruct.TIM_Period = 287;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    // TIM2 initialize
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    // Start TIM2
    TIM_Cmd(TIM2, ENABLE);
    timer_value = 0;

    // Nested vectored interrupt settings
    // TIM2 interrupt is most important (PreemptionPriority and
    // SubPriority = 0)
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void TIM2_IRQHandler()
{
    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIM2, TIM_IT_Update))
    {
    	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    	timer_value++;
    }
}

void TM_TIMER_PWM_Init(int period) {
	TIM_TimeBaseInitTypeDef TIM_BaseStruct;

	/* Enable clock for TIM4 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_BaseStruct.TIM_Prescaler = 9;
	/* Count up */
    TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_BaseStruct.TIM_Period = 287*period; /* 10kHz PWM */
    TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_BaseStruct.TIM_RepetitionCounter = 0;
	/* Initialize TIM4 */
    TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);
	/* Start count on TIM4 */
    TIM_Cmd(TIM4, ENABLE);
}

void TM_PWM_Init(int periode) {
	int pulse_length = (((periode*287)+ 1)* 50) / 100-1;
	TIM_OCInitTypeDef TIM_OCStruct;

	/* Common settings */

	/* PWM mode 2 = Clear on compare match */
	/* PWM mode 1 = Set on compare match */
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;

/*
	To get proper duty cycle, you have simple equation

	pulse_length = ((TIM_Period + 1) * DutyCycle) / 100 - 1



	where DutyCycle is in percent, between 0 and 100%

	25% duty cycle: 	pulse_length = ((8399 + 1) * 25) / 100 - 1 = 2099
	50% duty cycle: 	pulse_length = ((8399 + 1) * 50) / 100 - 1 = 4199
	75% duty cycle: 	pulse_length = ((8399 + 1) * 75) / 100 - 1 = 6299
	100% duty cycle:	pulse_length = ((8399 + 1) * 100) / 100 - 1 = 8399

	Remember: if pulse_length is larger than TIM_Period, you will have output HIGH all the time
*/

	TIM_OCStruct.TIM_Pulse = pulse_length; /* 25% duty cycle */
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

}

int main(void)
{
	//estructura con todas la I/O de la placa (diria que mas...)
	GPIO_InitTypeDef gpio;//variable para usar la estructura
    TIM_TimeBaseInitTypeDef timerInitStructure;
    TIM_TimeBaseInitTypeDef timerInitStructure_led;
	int i = 0, rand_tled = 0, max_input_value = 0x7FFFFFFF;

	int dutyCycle = 0;
	int j = 0;

	initLed3(gpio);
	initButton(gpio);


	TM_RNG_Init();
	/*
	 * Generate random, wee use the ddelay function (1 = 3ms) to wait until the random value
	 * */

	while(rand_tled<=334 || rand_tled >=1670){
		j=TM_RNG_Get();
		if(j<0 )j*=-1;
		rand_tled = map(j,334,1670);
	}
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);
	delay(rand_tled);
	GPIO_SetBits(GPIOG, GPIO_Pin_13);
	GPIO_SetBits(GPIOG, GPIO_Pin_1);

	//InitializeTimer(timerInitStructure);
	TIM2_INT_Init();
	while (1)
	{
		//GPIO_SetBits(GPIOG, GPIO_Pin_1);
		/*if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
			//GPIO_ToggleBits(GPIOG, GPIO_Pin_1);
			//timerValue+=50;
			//timerValue+=128;
			timerValue++;
		}*/

		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
			GPIO_ResetBits(GPIOG, GPIO_Pin_13);
			//dutyCycle = 5000/timerValue;

			TM_LEDS_PWM_Init();
			TM_TIMER_PWM_Init(timerValue);
			TM_PWM_Init(timerValue);
		}
	}
}
