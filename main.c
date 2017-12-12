#include "I2C_Header.h"

void main(void) {
    //    uint8_t toTransmit[EEPROM_PAGE_SIZE], result[0x10];
    //0x00 <= byteAddress <= 0x3FF
    //0x3FF = 1023
    ADDRESS myAddress;
    ADDRESS receiveAddress;

    uart_setup(); //INITIALIZE UART BEFORE I2C
    i2c_setup();
    //    setupLED();

    while (1) {
        concatenateReceiveArray();
        if (buildAddress) {
            switch (currentSection++) {
                case NAME:
                    setName(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case TELEPHONE:
                    setTelephone(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case STREET:
                    setStreet(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case CITY:
                    setCity(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    currentSection = NAME;
                    break;
            }
            buildAddress = false;

            if (startI2CTransmission) {
                writeAddress(&myAddress, DEVICE_ADDRESS, userToByteAddress(addressNumber));
                startI2CTransmission = false;
            }
        } else if (startI2CReception) {
            readAddress(&receiveAddress, DEVICE_ADDRESS, userToByteAddress(addressNumber)); //I2C
            transmitAddress(&receiveAddress); //UART
            startI2CReception = false;
        }
    }
    return;
}