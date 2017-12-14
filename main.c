#include "I2C_Header.h"

void main(void) {
    //0x00 <= byteAddress <= 0x3FF
    //0x3FF = 1023
    ADDRESS myAddress;
    ADDRESS receiveAddress;

    uart_setup(); //INITIALIZE UART BEFORE I2C
    i2c_setup();
    //    setupLED();

    while (1) {
        concatenateReceiveArray(); //UART->WAITS FOR VALID INPUT->INTERRUPT TRIGGERED
        if (buildAddress) {
            switch (currentSection++) {
                case NAME: //ADD NAME TO ADDRESS
                    setName(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case TELEPHONE: //ADD TELEPHONE TO ADDRESS
                    setTelephone(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case STREET: //ADD STREET TO ADDRESS
                    setStreet(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    break;
                case CITY: //ADD CITY TO ADDRESS
                    setCity(&myAddress, receivedArray, inputSize);
                    inputSize = 0;
                    currentSection = NAME;
                    break;
            }
            buildAddress = false;

            if (startI2CTransmission) { //WRITING ADDRESS ON THE EEPROM
                writeAddress(&myAddress, DEVICE_ADDRESS, userToByteAddress(addressNumber));
                startI2CTransmission = false;
            }
        } else if (startI2CReception) {
            readAddress(&receiveAddress, DEVICE_ADDRESS, userToByteAddress(addressNumber)); //READ ADDRESS FROM EEPROM->I2C
            transmitAddress(&receiveAddress); //PRINT ADDRESS ON TERMINAL->UART
            startI2CReception = false;
        }
    }
    return;
}