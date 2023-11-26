/*
 * frec_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "frec_init.h"
volatile unsigned long counter0, counter1, Counter, tiempo_ms;
int factor, rpm, speed_cm7s;
unsigned char gap=0;
unsigned short nV=0;
extern unsigned char mensaje[];

void TIM2_IRQHandler(void)
{
	if ((TIM2->SR & TIM_SR_CC2IF) != 0)
	{
		//GPIOC->ODR^=(1<<7);
		TIM2->SR &= ~(TIM_SR_CC2IF);
		if ((TIM2->SR & TIM_SR_CC2OF) != 0) // Check the overflow
			{
				// Overflow error management
				//gap = 0; // Reinitialize the laps computing
				nV++;
				TIM2->SR &= ~(TIM_SR_CC2OF | TIM_SR_CC2IF); // Clear the flags
				return;
			}

		if (gap == 0) /* Test if it is the first rising edge */
			{
				counter0 = TIM2->CCR2; /* Read the capture counter which clears the CC1ICF */
				gap = 1; /* Indicate that the first rising edge has yet been detected */
			}
		else
			{   gap=0;
				counter1 = TIM2->CCR2; /* Read the capture counter which clears the	CC1ICF */

				if (counter1>counter0) Counter=counter1-counter0+nV*(2^32 - 1);
				else Counter=counter1-counter0+(nV-1)*(2^32 - 1);

				nV=0;
				counter0 = counter1;
				tiempo_ms=(Counter*factor)/1000; /*se multiplican los conteos
				por el tiempo de conteo y se divide entre 1 millón para tener el tiempo
				por vuelta en milisegundos
				 */


			}
	}
	else
	{
		/* Unexpected Interrupt */
		/* Manage an error for robust application */
	}
}
void frec_init (void)
{
	factor=(15); //factor, es el tiempo que toma cada conteo del encoder
	RCC->AHBENR |= (1<<18);			//Bit 17 (1): I/O port B clock enabled
	GPIOB->MODER |= (2<<6);			//PORTB3 : Alternate mode
	GPIOB->AFR[0]|= (2<<12);		//Alternate Function 02 que es canal del TIM2

	RCC->APB1ENR |= (1<<0);			//Bit 0 (1): TIM2 clock enabled
	TIM2->PSC = 8-1; 					//Set prescaler to 7, so APBCLK/8 i.e 1MHz

	TIM2->CCMR1 |= TIM_CCMR1_CC2S_0	| TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1;
			//Select the active input TI1 (CC1S = 01),	program the input filter for 8 clock cycles (IC1F = 0011),
			//select the rising edge on CC1 (CC1P = 0, reset value)	and prescaler at each valid transition (IC1PS = 00, reset value)
	TIM2->CCER |= TIM_CCER_CC2E; 		// Enable capture by setting CC1E (00 Rising edge, default)
	TIM2->DIER |= TIM_DIER_CC2IE; 		// Enable interrupt on Capture/Compare
	TIM2->CR1 |= TIM_CR1_CEN; 			// Enable TIM2 counter

	TIM2->EGR |= TIM_EGR_UG; 		//Force update generation (UG = 1)

	NVIC->ISER[0]=(1<<15);
	}
