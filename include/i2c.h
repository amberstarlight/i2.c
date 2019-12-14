#ifndef I2C_H
#define I2C_H

#include <stdarg.h>
#include <util/twi.h>

#define F_SCL 400000UL

void i2cInit();
void i2cStart();
void i2cSendAddress(char address, int rw);
void i2cAwaitCompletion();
int  i2cCheckStatus(char expectedStatus);
void i2cSendByte(char byte);
void i2cSendData(int argc, ...);
void i2cStop();

#endif
