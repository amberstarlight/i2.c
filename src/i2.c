#include "i2c.h"

void i2cInit() {
	/*
		Set TWSR bits 0, 1 and 2 to 0.
		TWPS0 and TWPS1 are prescaler bits.
		Bit 2 is reserved, and should always be read as 0.
		By setting TWSR to 0, we clear the register and ensure these bits are set.
	*/
	TWSR = 0;
	/*
		The bit rate register, TWBR, controls the SCL period (clockspeed)
		relative to processor clockspeed. The following calculation is
		devised from rearranging the SCL Frequency equation given in the docs.
	*/
	TWBR = ((F_CPU / (8 * F_SCL)) - 2);
}

void i2cStart() {
	/*
		An I2C start condition is sent by writing 1X10 X10X to the TWCR register.
		┌───────┬──────┬───────┬───────┬──────┬──────┬─────┬──────┐
		│ TWINT │ TWEA │ TWSTA │ TWSTO │ TWWC │ TWEN │  -  │ TWIE │
		├───────┼──────┼───────┼───────┼──────┼──────┼─────┼──────┤
		│   1   │   X  │   1   │   0   │   X  │   1  │  0  │   X  │
		└───────┴──────┴───────┴───────┴──────┴──────┴─────┴──────┘
		To achieve this, we set TWCR to 0xA4. (1010 0100)
		The master device sends the start condition;
		this declares the MCU as the I2C master device.
	*/
	TWCR = 0xA4;
	/*
		In standard I2C communication, we wait for tasks to complete before sending another.
		We read the status code to discern whether a task has completed successfully or has failed.
	*/
	i2cAwaitCompletion();
	i2cCheckStatus(TW_START);
}

void i2cSendAddress(char address, int rw) {
	/*
		┌─────────────┬─────────┐
		│ I2C Address │ R/W bit │
		└───7 Bits────┴──1 Bit──┘

		I2C addresses are 7 bits in size. We must bit shift this left
		by 1 before setting the read/write bit.

		The master device requests (reads) data if this bit is set to 1,
		and writes (sends) data if this bit is set to 0.

		To send data, we AND the byte with 0xFE (1111 1110), and set TWDR
		(I2C Data Register) equal to it before sending the address.

		┌─────────────┐
		│   R/W bit   │
		├─────┬───────┤
		│  1  │ Read  │
		├─────┼───────┤
		│  0  │ Write │
		└─────┴───────┘
	*/
	if (rw) {
		TWDR = ((address << 1) & 0xFE);
	} else {
		TWDR = ((address << 1) | 0x01);
	}
	/*
		To tell the MCU to send over I2C, we set the following bits in TWCR;
		┌───────┬──────┬───────┬───────┬──────┬──────┬─────┬──────┐
		│ TWINT │ TWEA │ TWSTA │ TWSTO │ TWWC │ TWEN │  -  │ TWIE │
		├───────┼──────┼───────┼───────┼──────┼──────┼─────┼──────┤
		│   1   │   X  │   0   │   0   │   X  │   1  │  0  │   X  │
		└───────┴──────┴───────┴───────┴──────┴──────┴─────┴──────┘
		To achieve this, we set TWCR to 0x84. (1000 0100)
	*/
	TWCR = 0x84;
	i2cAwaitCompletion();
	i2cCheckStatus(TW_MT_SLA_ACK);
}

void i2cAwaitCompletion() {
	/*
		Wait for an I2C task to complete.
		We set TWINT to initiate a task, and TWINT is set to 0 during a task.
		When TWINT becomes 1 again, the task is finished.
		We must check the status code to discern success or failure.
	*/
	while (!(TWCR & 0x80));
}

int i2cCheckStatus(char expectedStatus) {
	/*
		TW_STATUS = (TWSR & TW_STATUS_MASK)
		TW_STATUS_MASK = 0xF8
		To check if an I2C task was completed successfully,
		we check if the status code is what we expect.
	*/
	return (TW_STATUS == expectedStatus);
}

void i2cSendByte(char byte) {
	/*
		To send data, we fill the data register with the data we want to send,
		and set the bits in TWCR to send data - the same way as in i2cSendAddress().
	*/
	TWDR = byte;
	TWCR = 0x84;
	i2cAwaitCompletion();
	i2cCheckStatus(TW_MT_DATA_ACK);
}

void i2cSendData(int argc, ... ) {
	/*
		To send multiple bytes over I2C.
		Some devices have multi-byte commands, where it expects values after an instruction.
	*/
	va_list valist;
	va_start(valist, argc);
	for (int i = 0; i < argc; i++) {
		i2cSendByte(va_arg(valist, int));
	}
	va_end(valist);
	/*
		TODO:
		Test how efficient this varidiac function is versus using a preprocessor macro.
	*/
}

void i2cStop() {
	/*
		An I2C stop condition is sent by writing 1X01 X10X to the TWCR register.
		┌───────┬──────┬───────┬───────┬──────┬──────┬─────┬──────┐
		│ TWINT │ TWEA │ TWSTA │ TWSTO │ TWWC │ TWEN │  -  │ TWIE │
		├───────┼──────┼───────┼───────┼──────┼──────┼─────┼──────┤
		│   1   │   X  │   0   │   1   │   X  │   1  │  0  │   X  │
		└───────┴──────┴───────┴───────┴──────┴──────┴─────┴──────┘
		To achieve this, we set TWCR to 0x94. (1001 0100)
	*/
	TWCR = 0x94;
}
