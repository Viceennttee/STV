/*
 * UART_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "UART_init.h"
unsigned char mensaje[] = {"m/s 0.000 A 0#.### V ##.##0 T 0#.### \r\n"};
unsigned char i = 0;
void USART1_IRQHandler (void)
{
	USART1->TDR = mensaje[i++]; //Contains the data character to be transmitted
	if (mensaje[i]==0) {
		i = 0;//si si la cadena esta vacÃ­a comenzar de nuevo, creo
	}
}

void UART_init (void)
{
	RCC->AHBENR|=(1<<17);	//CLK PORTA
	RCC->APB2ENR|=(1<<14);	//USART 1

	GPIOA->MODER|=(2<<20)+(2<<18);	//PORTA9 & PORTA10 FunciÃ³n alterna
	GPIOA->AFR[1]|=(1<<4)+(1<<8);	//PORTA9 & PORTA10 USART1_TX, USART1_RX

	USART1->BRR|=833;				//CLK UART / Baud Rate -> 8MHz / 9600, se supone
	//para que funcione en la terminal del celular debe estar a 5000

	USART1->CR1|=(1<<0)+(1<<3)+(1<<2)+(1<<6);	/*TE=RE=USART_Enable=1,
	 Bit 6 hab interrupciÃ³n transmisiÃ³n
	 */
}

