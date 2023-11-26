/*
 * DAC_init.c
 *
 *  Created on: Nov 12, 2023
 *      Author: vicen
 */
#include <stdint.h>
#include "stm32f051x8.h"
#include "DAC_init.h"

//el DAC no tiene interrupciones, se cambia su valor en tim6
void DAC_init(void){
	RCC->APB1ENR|=(1<<29); //reloj del DAC
	GPIOA->MODER|=(3<<8); //PA4: Analog mode
	DAC->CR=(1<<0);  //DAC enable
}

