#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "misc.h"
#include "delay.h"

#define I2Cx_RCC				RCC_APB1Periph_I2C2
#define I2Cx						I2C2
#define I2C_GPIO_RCC		RCC_APB2Periph_GPIOB
#define I2C_GPIO				GPIOB
#define I2C_PIN_SDA			GPIO_Pin_11
#define I2C_PIN_SCL			GPIO_Pin_10
#define bufferlength 3

#define SLAVE_ADDRESS		0x29

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_address_direction(uint8_t address, uint8_t direction);
void i2c_transmit(uint8_t byte);
uint8_t i2c_receive_ack(void);
uint8_t i2c_receive_nack(void);
void i2c_write(uint8_t address, uint8_t* data);//, uint8_t data);
void i2c_read(uint8_t address, uint8_t* data);

///////////////////////////////////////////////////////////

int main(void){
	DelayInit();
	i2c_init();
	uint8_t data[bufferlength] = {0xAA, 0x18, 0xF7};
	while (1){
		i2c_write(SLAVE_ADDRESS, data);
		DelayMs(1);
	}
}

void i2c_init(){
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB1PeriphClockCmd(I2Cx_RCC, ENABLE);
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; // Disable
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2Cx, &I2C_InitStruct);
	I2C_Cmd(I2Cx, ENABLE);
	
	RCC_APB2PeriphClockCmd(I2C_GPIO_RCC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = I2C_PIN_SCL | I2C_PIN_SDA;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);
}

void i2c_start(){
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));  // Wait until I2Cx not busy anymore
	I2C_GenerateSTART(I2Cx, ENABLE);
	// Wait for I2C EV5, start condition has been correctly released 
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
}

void i2c_stop(){
	I2C_GenerateSTOP(I2Cx, ENABLE);
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF)); // Wait until I2C stop condition is finished
}

void i2c_address_direction(uint8_t address, uint8_t direction){
	I2C_Send7bitAddress(I2Cx, address, direction);
	// Wait for I2C EV6, slave acknowledged address
	if (direction == I2C_Direction_Transmitter){
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if (direction == I2C_Direction_Receiver){	
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

void i2c_transmit(uint8_t byte){
	I2C_SendData(I2Cx, byte);
	// Wait for I2C EV8_2, data has been output on the bus)
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t i2c_receive_ack(){
	I2C_AcknowledgeConfig(I2Cx, ENABLE);// Enable ACK of received data
	// Wait for I2C EV7, data received in I2C data register
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx);
}

uint8_t i2c_receive_nack(){
	I2C_AcknowledgeConfig(I2Cx, DISABLE);// Disable ACK of received data
	// Wait for I2C EV7: data received in I2C data register
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx);
}

void i2c_write(uint8_t address, uint8_t* data){
	uint8_t data_index = bufferlength;
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	while(data_index){
		i2c_transmit(data[data_index - 1]);
		data_index--;
	}
	i2c_stop();
}

void i2c_read(uint8_t address, uint8_t* data){
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Receiver);
	*data = i2c_receive_nack();
	i2c_stop();
}
