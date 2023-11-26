/*
 * ADC_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "ADC_init.h"



extern unsigned char mensaje[];
extern long tiempo_ms;
extern int speed_cm7s, rpm;
volatile unsigned short ADC_result, adc_voltaje_mV;
volatile unsigned short joy_DAC, amp_A, volt_B, temp_C;
extern unsigned short frec_DAC;
extern uint8_t frec_en;
short canal=1;
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
			amp_A=0;//(adc_voltaje_mV-1600)*1000/66; //pasar el ADC aAmperaje, falta Axel

			//pasar el valor en mV a la cadena de UART
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

			mensaje[35] = (temp_C%10)+0x30;
			adc_voltaje_mV /= 10;
			mensaje[34] = (temp_C%10)+0x30;
			adc_voltaje_mV /= 10;
			mensaje[33] = (temp_C%10)+0x30;
			mensaje[31] = (temp_C/10)+0x30;

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
