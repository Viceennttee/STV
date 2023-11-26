#include <stdint.h>
#include "stm32f051x8.h"
#include "ADC_init.h"
#include "UART_init.h"
#include "TIM6_init.h"
#include "leds_init.h"
#include "DAC_init.h"
#include "pin_init.h"
#include "frec_init.h"

int main(void)
{
	ADC_init(); //ya comentado
	UART_init(); //ya comentado
	TIM6_init(); //ya comentado
	leds_int(); //ya comentado
	DAC_init(); //ya comentado
	pin_init(); //sí sirve acá
	frec_init(); //ya
    while(1);
}
