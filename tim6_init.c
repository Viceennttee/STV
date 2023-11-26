/*
 * TIM6_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "TIM6_init.h"
extern volatile unsigned short joy_DAC;
unsigned short frec_DAC, DAC_total;
extern uint8_t f;
uint8_t frec_en;
int F_emergencia=1;
extern volatile unsigned short volt_B;
extern int rpm;

void TIM6_DAC_IRQHandler(void)
{
	TIM6->SR = 0; //: No update occurred, se baja la bandera
	ADC1->CR|= (1<<2); // ADC start conversion command po
	NVIC->ISER[0]|=(1<<27); //se habilita NVIC de USART
	USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y
	//la interrupción por  Transmission complete interrupt enable



	//cambiará poquito
	frec_DAC=((324*rpm+1455*100)/100)*frec_en; //sí está bien, pero calibrar



	DAC_total=(joy_DAC+frec_DAC)*f*F_emergencia;
	if(DAC_total>4090)DAC_total=4095;
	DAC->DHR12R1=DAC_total; //el valor del DAC dependiendo del freno

	//falta incorporarle lo del encoder

	//prende los leds dependiendo del nivel de batería
	if(volt_B>=39){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<10);//prender verde
		F_emergencia=1;

	}
	else if(volt_B<39&&volt_B>=36){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<11);//prender amarillo
		F_emergencia=1;
	}
	else if(volt_B<37){
		GPIOC->ODR &= (1<<8); //apagar los demás menos led de freno
		GPIOC->ODR |= (1<<12);//prender rojo
		F_emergencia=1;
	}

}

void TIM6_init(void)
{
	RCC -> APB1ENR |= (1<<4);	//Enable clocks TIM6
	TIM6 -> PSC |= 8-1;			//DIVISOR/8 8MHz/8 = 1MHz, según
	TIM6 -> ARR = 50000 - 1;	//50,000 CUENTAS DE 1us=0.05s, según

	TIM6->CR1|=1; //Counter enable
	TIM6->DIER|=1; //Module Interrupt enable
	NVIC->ISER[0]=(1<<17); //NVIC Interrupt set enable register

}