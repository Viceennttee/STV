/*
 * pin_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "pin_init.h"
uint8_t f=1;
extern int rpm;
//se tiene que llamar igual que la interrupciÃ³n
void EXTI0_1_IRQHandler(void){
	//cuando se cumpla la condiciÃ³n
	EXTI->PR |=1; // bajar la bandera, supongo
	GPIOC->ODR ^= (1<<8); 	//cambiar el led de la tarjeta
	GPIOB->ODR ^= (1<<12);
	f = ~f & 0x01; //cambiar el estado de f
	rpm^=1;

}

void pin_init(void)
{
		RCC->AHBENR |=(1<<19); //puerto C del led de la tarjeta
		RCC->AHBENR |=(1<<17); // puerto a, botÃ³n del joystick
		RCC->AHBENR |=(1<<18); // puerto b, led que había causado confusión

		GPIOA->PUPDR |=(1<<0); //pullup del PA0
		GPIOB->MODER |=(1<<24); 		// se habilita Pb12 como output
		GPIOC->MODER |=(1<<16); 		// se habilita PC8 LED tarjeta como output ya no
		GPIOC->ODR = (1<<13);
		//preguntar

		//SYSCFG->EXTICR[0] =(0<<0);		//pag 170
		//es puerto A, entonces es el [0]
		//es el pin 0 entonces es el que estÃ¡ por defecto
		//no es ncesaria la lÃ­nea de arriba

		EXTI->IMR |=(1<<0);//enmascaramiento para indicar el bit exacto a utilizarpag 222
		EXTI->RTSR |= 1;//activar con el flanco de subida
		//para que cuando se presione mande un 1
		NVIC->ISER[0]=(1<<5); //NVIC de las interrupciones en EXTI Line[1:0] interrupts
}