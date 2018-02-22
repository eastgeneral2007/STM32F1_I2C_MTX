#include "stm32f10x.h"
#include "i2c_MTX.h"
#include "delay.h"
///////////////////////////////////////////////////////////

int main(void){
	DelayInit();
	i2c_init();
	uint8_t data[bufferlength] = {0xAA, 0xC2, 0xF7};
	while (1){
		i2c_write(SLAVE_ADDRESS, data);
		DelayMs(1);
	}
}
