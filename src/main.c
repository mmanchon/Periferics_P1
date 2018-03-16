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



#define MAX_PERIODE 28800

/* Include my libraries here */
#include "tm_stm32f4_rng.h"


/* Variables globales */
int timer_value;
int tm4_periode;


/*
 * Function for scale a number to the desired range.
 */
int map(int rand, int min, int max)
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


void INIT_OutputPins(GPIO_InitTypeDef gpio){
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

		gpio.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_5;
		gpio.GPIO_Mode = GPIO_Mode_OUT;
		//una vez digamos que hemos indicado que "usaremos" en las linias anteriores
		//ahora se lo indicamos por asi decirlo, a la placa
		GPIO_Init(GPIOG, &gpio);
}


void INIT_Button(GPIO_InitTypeDef gpio){
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


/* Configure pins to be interrupts */
void EXTI_Pwm(void) {
	/* Set variables used */
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	/* Enable clock for GPIOD */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/* Enable clock for SYSCFG */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Set pin as input */
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* Tell system that you will use PD0 for EXTI_Line0 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource0);

	/* PD0 is connected to EXTI_Line0 */
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	/* Enable interrupt */
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	/* Interrupt mode */
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	/* Triggers on rising and falling edge */
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	/* Add to EXTI */
	EXTI_Init(&EXTI_InitStruct);

	/* Add IRQ vector to NVIC */
	/* PD0 is connected to EXTI_Line0, which has EXTI0_IRQn vector */
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	/* Set priority */
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	/* Set sub priority */
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	/* Enable interrupt */
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	/* Add to NVIC */
	NVIC_Init(&NVIC_InitStruct);
}


/* Set interrupt handlers */
/* Handle PD0 interrupt */
void EXTI0_IRQHandler(void) {
	/* Make sure that interrupt flag is set */
	if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line0);
		GPIO_SetBits(GPIOG, GPIO_Pin_5);
		if(tm4_periode > MAX_PERIODE) {
			GPIO_SetBits(GPIOG, GPIO_Pin_14);
			GPIO_ResetBits(GPIOG, GPIO_Pin_13);
		}else{
			GPIO_SetBits(GPIOG, GPIO_Pin_13);
		}

	}
}

void TIM2_INT_Init()
{
	// Nested vectored interrupt settings
	// TIM2 interrupt is most important (PreemptionPriority and
	// SubPriority = 0)
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);


	// Enable clock for TIM2
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_Period = 287;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    timer_value = 0;

    // TIM2 initialize
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    // Start TIM2
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler()
{
    // Checks whether the TIM2 interrupt has occurred or not
    if (TIM_GetITStatus(TIM2, TIM_IT_Update)!=RESET)
    {
    	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    	//GPIO_ToggleBits(GPIOG, GPIO_Pin_2);
    	timer_value++;
    }


}


void TM_PWM_Init(int dutycycle) {

	TIM_OCInitTypeDef TIM_OCStruct;


	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCStruct.TIM_Pulse = dutycycle;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
}



void TIM4_INT_Init(int periode)
{
    // Enable clock for TIM
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 9;
    TIM_TimeBaseInitStruct.TIM_Period = periode;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    // TIM2 initialize
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
    // Enable TIM2 interrupt
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
    // Start TIM4
    TIM_Cmd(TIM4, ENABLE);
    //timer_value = 0;

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    timer_value = 0;
    //TIM_ClearITPendingBit(TIM2, TIM_IT_Update);


    NVIC_Init(&NVIC_InitStruct);
    GPIO_ResetBits(GPIOG, GPIO_Pin_14);
}


int main(void)
{
	//estructura con todas la I/O de la placa (diria que mas...)
	GPIO_InitTypeDef gpio;//variable para usar la estructura
    int rand_tled = 0;
	int aux_timer;
	int dutyCycle = 0;
	int arand_tled = 0;

	tm4_periode = 0;



	INIT_OutputPins(gpio);
	INIT_Button(gpio);

	//Inicializamos el Random
	TM_RNG_Init();

	//Generamos valores random y mapeamos a los valores que cosnideramos válidos
	//para poder hacer un delay de entre 1s y 5s
	while(rand_tled<=334 || rand_tled >=1670){
		arand_tled = TM_RNG_Get();
		if(arand_tled < 0 )arand_tled *= -1;
		rand_tled = map(arand_tled,334,1670);
	}
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);
	delay(rand_tled);
	GPIO_SetBits(GPIOG, GPIO_Pin_13);

	TIM2_INT_Init();

	while(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {

	}
	timer_value= timer_value/100; //convertimos a ms

	tm4_periode = (timer_value * 288)/100;//escalamos el periodo necesario para el pwm en ms

	GPIO_ResetBits(GPIOG, GPIO_Pin_13);
	GPIO_ResetBits(GPIOG, GPIO_Pin_5);

	dutyCycle = (double)(tm4_periode*50.0)/(timer_value);

	//Rutinas de inicializacion del PWM
	TM_LEDS_PWM_Init();

	//Configuramos la interrupcion externa
	EXTI_Pwm();
	//Inicializamos la interrupcion para el PWM
	TIM4_INT_Init(tm4_periode);
	TM_PWM_Init(dutyCycle);

	while (1)
	{
		GPIO_ResetBits(GPIOG, GPIO_Pin_5);
	}
}
