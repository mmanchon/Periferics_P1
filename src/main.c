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
#include "stm32f4xx_gpio.h"

/* Include my libraries here */


void delay(int counter)
{
	int i;
	for (i = 0; i < counter * 10000; i++) {}
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
		gpio.GPIO_Pin = GPIO_Pin_13;
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


int main(void)
{
	//estructura con todas la I/O de la placa (diria que mas...)
	GPIO_InitTypeDef gpio;//variable para usar la estructura
	initLed3(gpio);
	initButton(gpio);
	int i = 0;
	//GPIO_SetBits(GPIOG, GPIO_Pin_13);
	GPIO_ResetBits(GPIOG, GPIO_Pin_13); // LED OFF

	while (1)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
			while(i<3){
				GPIO_SetBits(GPIOG, GPIO_Pin_13); // LED ON
				delay(400);
				GPIO_ResetBits(GPIOG, GPIO_Pin_13); // LED OFF
				delay(400);
				i++;
			}
		}
	}
}
