/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f051x8.h"
volatile unsigned short ADC_result, adc_voltaje_mV;
volatile unsigned short joy_DAC, amp_A, volt_B, temp_C;
short canal=1;


unsigned char mensaje[] = {"m/s 0.000 A 0#.### V ##.##0 T 0#.### \r\n"};
unsigned char i = 0;

unsigned short frec_DAC, DAC_total;
uint8_t frec_en;
int F_emergencia=1;

uint8_t f=1;

volatile unsigned long counter0, counter1, Counter, tiempo_ms;
int factor, rpm, speed_cm7s;
unsigned char gap=0;
unsigned short nV=0;

void ADC_COMP_IRQHandler(void)
{
	switch(canal){
		case 1:
			joy_DAC=ADC1->DR; //variable a enviar al DAC, CALIBRAR YA CON MOTOR
			if(joy_DAC<=2250) {
				frec_en=1;
				joy_DAC=0;
			}
			if(joy_DAC>2250) frec_en=0;
			rpm=60*1000/(tiempo_ms); //revoluciones por 60,000ms
			speed_cm7s=53*rpm/60;//53 de circunferencia del círcullo cm/s
			mensaje[7] = (speed_cm7s%10)+0x30;
			speed_cm7s /= 10;
			mensaje[6] = (speed_cm7s%10)+0x30;
			mensaje[4] = (speed_cm7s/10)+0x30;
			canal=2;
			ADC1->CHSELR = (1<<2);
			break;
		case 2:
            // Canal 2 amperimetro
			ADC_result= ADC1->DR; //lectura del ADC en PA2
			adc_voltaje_mV = ADC_result*3300/4095; //pasar el ADC a mV
			amp_A=(adc_voltaje_mV-1600)*1000/66; //pasar el ADC aAmperaje, falta Axel

			//pasar el valor en mV a la cadena de UART
			mensaje[17]= (amp_A%10)+0x30;
			amp_A /=10;
			mensaje[16] = (amp_A%10)+0x30;
			amp_A /= 10;
			mensaje[15] = (amp_A%10)+0x30;
			mensaje[13] = (amp_A/10)+0x30;
			canal=3; // Habilitar el canal 3 (PA3) voltaje de batería
			ADC1->CHSELR = (1<<3);
			break;
		case 3:
	        ADC_result= ADC1->DR; //lectura del ADC en PA3
	    	adc_voltaje_mV = ADC_result*3300/4095;
			volt_B=adc_voltaje_mV*(1200+15000)/12000; //Voltaje de la batería en V

			//pasar el valor en mV a la cadena de UART
			mensaje[25] = (volt_B%10)+0x30;
			volt_B /= 10;
			mensaje[24] = (volt_B%10)+0x30;
			volt_B /= 10;
			mensaje[22] = (volt_B%10)+0x30;
			mensaje[21] = (volt_B/10)+0x30;
	        canal=10; // Habilitar el canal 10 (PC0) temperatura
			ADC1->CHSELR = (1<<10);
	        break;
		case 10:
			ADC_result= ADC1->DR; //lectura del ADC en PC0 (temperatura)
			temp_C=10*ADC_result*3330/4095; //Temperatura falta Fercho

			mensaje[35] = (DAC_total%10)+0x30;
			DAC_total /= 10;
			mensaje[34] = (DAC_total%10)+0x30;
			DAC_total /=10;
			mensaje[33] = (DAC_total%10)+0x30;
			mensaje[31] = (DAC_total/10)+0x30;


			canal = 1; // Habilitar el canal 1 de nuevo (PA1), joystick
			ADC1->CHSELR = (1<<1);
			break;

	}
	NVIC->ISER[0]|=(1<<27); //se habilita NVIC de USART
	USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y
	//la interrupción por  Transmission complete interrupt enable
}
void ADC_init()
{
	RCC->AHBENR|= (1<<17); // PORTA Clock joystick, sensor de corriente y voltaje
	RCC->AHBENR|= (1<<19); // PORTC Clock temperatura
	RCC->APB2ENR |= (1<<9); // enable clocks for ADC1 clock

	GPIOA->MODER|=(3<<2); // PA1 joystick
	GPIOA->MODER|=(3<<4);  ///PA2 amp
	GPIOA->MODER|=(3<<6); // PA3 voltaje
	GPIOC->MODER|=(3<<0); //PC0 temperatura

	ADC1->CFGR1|=(1<<26)+(1<<27); // ADC_IN1 MUX selected, single conv, SW trigger//Right alignment, 12 bits
	ADC1->CHSELR|=(1<<1);// se empieza ADC en el canal 1 (PA1)

	ADC1->IER|=(1<<2); //habilitación de interrupción en EOC (End of conversion;
	NVIC->ISER[0]=(1<<12); //NVIC del ADC

	ADC1->CR|=1; //habilitación del ADC;
	do{} while ((ADC1->ISR & 1)==0); //while ADC not ready
	/*ADC1->ISR|=1;
	 * Creo que esta línea es inncesesaria en la inicialización
	 */
}

void USART1_IRQHandler (void)
{
	USART1->TDR = mensaje[i++]; //Contains the data character to be transmitted
	if (mensaje[i]==0) {
		i = 0;//si si la cadena esta vacía comenzar de nuevo, creo
	}
}
void UART_init (void)
{
	RCC->AHBENR|=(1<<17);	//CLK PORTA
	RCC->APB2ENR|=(1<<14);	//USART 1

	GPIOA->MODER|=(2<<20)+(2<<18);	//PORTA9 & PORTA10 Función alterna
	GPIOA->AFR[1]|=(1<<4)+(1<<8);	//PORTA9 & PORTA10 USART1_TX, USART1_RX

	USART1->BRR|=833;				//CLK UART / Baud Rate -> 8MHz / 9600, se supone
	//para que funcione en la terminal del celular debe estar a 5000

	USART1->CR1|=(1<<0)+(1<<3)+(1<<2)+(1<<6);	/*TE=RE=USART_Enable=1,
	 Bit 6 hab interrupción transmisión
	 */
}

void TIM6_DAC_IRQHandler(void)
{
	TIM6->SR = 0; //: No update occurred, se baja la bandera
	ADC1->CR|= (1<<2); // ADC start conversion command po
	NVIC->ISER[0]|=(1<<27); //se habilita NVIC de USART
	USART1 -> CR1 |= (1<<3)+(1<<2)+(1<<0)+(1<<6);	//HABILITAMOS EL Tx, Rx, EL UART Y
	//la interrupción por  Transmission complete interrupt enable

	frec_DAC=(324*rpm+1455*100)/100*frec_en;
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

void leds_int(){
	//leds de la batería y ya
	RCC->AHBENR |= (1<<19);			//Bit 19 (1): I/O port C clock enabled
	GPIOC->MODER |= (1<<20)|(1<<22)|(1<<24);		//PORTC9 : Output mode
}

void DAC_init(void){
	RCC->APB1ENR|=(1<<29); //reloj del DAC
	GPIOA->MODER|=(3<<8); //PA4: Analog mode
	DAC->CR=(1<<0);  //DAC enable
}

void EXTI0_1_IRQHandler(void){
	//cuando se cumpla la condición
	EXTI->PR |=1; // bajar la bandera, supongo
	GPIOC->ODR ^= (1<<8); 	//cambiar el led de la tarjeta
	GPIOB->ODR ^= (1<<12);
	f = ~f & 0x01; //cambiar el estado de f

	rpm=0;
}
void pin_init(void)
{
		RCC->AHBENR |=(1<<19); //puerto C del led de la tarjeta
		RCC->AHBENR |=(1<<17); // puerto a, botón del joystick
		RCC->AHBENR |=(1<<18); // puerto b, led por el que casi me navajean

		GPIOA->PUPDR |=(1<<0); //pullup del PA0
		GPIOB->MODER |=(1<<24); 		// se habilita Pb12 como output
		GPIOC->MODER |=(1<<16); 		// se habilita PC8 LED tarjeta como output ya no
		GPIOC->ODR = (1<<13);
		//preguntar

		//SYSCFG->EXTICR[0] =(0<<0);		//pag 170
		//es puerto A, entonces es el [0]
		//es el pin 0 entonces es el que está por defecto
		//no es ncesaria la línea de arriba

		EXTI->IMR |=(1<<0);//enmascaramiento para indicar el bit exacto a utilizarpag 222
		EXTI->RTSR |= 1;//activar con el flanco de subida
		//para que cuando se presione mande un 1
		NVIC->ISER[0]=(1<<5); //NVIC de las interrupciones en EXTI Line[1:0] interrupts
}

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

				if (counter1>counter0) Counter=counter1-counter0+nV*(((2)^(32)) - 1);
				else Counter=counter1-counter0+(nV-1)*(((2)^(32)) - 1);

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

int main(void)
{
    /* Loop forever */
	ADC_init(); //ya comentado
	UART_init(); //ya comentado
	TIM6_init(); //ya comentado
	leds_int(); //ya comentado
	DAC_init(); //ya comentado
	pin_init(); //sí sirve acá
	frec_init(); //ya
	for(;;);
}