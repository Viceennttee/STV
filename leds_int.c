/*
 * leds_int.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */

#include <stdint.h>
#include "stm32f051x8.h"
#include "leds_init.h"

void leds_int(){
	//leds de la batería y ya
	RCC->AHBENR |= (1<<19);			//Bit 19 (1): I/O port C clock enabled
	GPIOC->MODER |= (1<<20)|(1<<22)|(1<<24);		//PORTC9 : Output mode
}
