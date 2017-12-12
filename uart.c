#include "I2C_Header.h"

void uart_setup(void) {
    //DIGITAL I/O
    ANSELB = 0x9F; //DEFAULT TO THE ANALOG MODE AFTER RESET
    //TO USE ANY PINS AS DIGITAL GENERAL PURPOSE OR
    //PERIPHERAL INPUTS, THE CORRESPONDING ANSEL BITS MUST
    //BE INITIALIZED TO '0' BY USER SOFTWARE

    //PIN CONFIGURATION
    TRISBbits.TRISB5 = RB5TX; //RB5 OUTPUT AS TX
    TRISBbits.TRISB6 = RB6RX; //RB6 INPUT AS RX

    //TRANSMISSION
    TX1STAbits.SYNC = ASYNC; //ASYNCHRONOUS MODE
    TX1STAbits.TX9 = EIGHTBIT; //8-BIT TRANSMISSION
    TX1STAbits.BRGH = LOWBAUD; //HIGH BAUD RATE DISABLED->LOW SPEED
    TX1STAbits.TXEN = ENABLE_TRANSMIT; //TRANSMIT ENABLED

    PIE3bits.TX1IE = DISABLE_TRANSMIT_INT; //DISABLE INTERRUPTS BEFORE CHANGING
    //STATES

    //RECEPTION
    RC1STAbits.SPEN = ENABLE_SERIAL; //SERIAL PORT ENABLED
    RC1STAbits.RX9 = EIGHTBIT; //8-BIT RECEPTION
    RC1STAbits.CREN = ENABLE_CONTINOUS; //ENABLE CONTINOUS RECEIVE
    PIE3bits.RC1IE = DISABLE_USART_INT; //USART RECEIVE INTERRUPT ENABLE BIT
    PIE3bits.RC1IE = ENABLE_USART_INT; //1/0->ENABLE/DISABLE

    //BAUD RATE
    BAUD1CONbits.BRG16 = EIGHTBIT_BAUD; //8-BIT BAUD GENERATOR
    SP1BRGL = BAUD_RATE; //BAUD RATE ACCORING TO TABLE->12=0xC->FOSC=8MHz

    //TX/RX->RB5/RB6
    bool state = (uint8_t) INTCONbits.GIE; //INTERRUPT CONTROL REGISTER
    //GLOBAL INTERRUPT ENABLE BIT
    //1/0->ENABLE/DISABLE
    INTCONbits.GIE = 0;
    PPSLOCK = 0x55; //LOCK PPS
    //PPS LOCK REGISTER->BIT 0->PPSLOCKED
    //1/0->LOCKED(PPS SELECTIONS CAN NOT BE CHANGED
    ///NOT LOCKED
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; //UNLOCK PPS

    RX1DTPPSbits.RX1DTPPS = 0x0E; //RB6->EUSART1:RX1;
    RB5PPS = 0x0F; //RB5->EUSART1:TX1;

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; //LOCK PPS

    INTCONbits.GIE = state; //PPS->PERIPHERAL PIN SELECT
}

//UART RECEPTION

inline void receiveData(uint8_t* receivedData) {
    while (0 == PIR3bits.RC1IF); //RECEIVE INTERRUPT FLAG
    //1/0->BUFFER IS NOT EMPTY/EMPTY 
    if (1 == RC1STAbits.OERR) //OVERRUN ERROR BIT
        //1->OVERRUN ERROR->CLEAR BIT CREN
        //0->NO OVERRUN ERROR    
    {
        RC1STAbits.OERR = 0;
        RC1STAbits.CREN = 0; //CONTINOUS RECEIVE ENABLE BIT
        RC1STAbits.CREN = 1; //ENABLES CONTINOUS RECEIVE UNTIL ENABLE BIT
        //CREN IS CLEARED
        //0 DISABLES CONTINOUS RECEIVE
        RC1STAbits.SPEN = 0;
        RC1STAbits.SPEN = 1;
    }
    *receivedData++ = RC1REG;
}

inline void receiveData_ISR(void) {
    if (1 == RC1STAbits.OERR) //OVERRUN ERROR BIT
        //1->OVERRUN ERROR->CLEAR BIT CREN
        //0->NO OVERRUN ERROR    
    {
        RC1STAbits.OERR = 0;
        RC1STAbits.CREN = 0; //CONTINOUS RECEIVE ENABLE BIT
        RC1STAbits.CREN = 1; //ENABLES CONTINOUS RECEIVE UNTIL ENABLE BIT
        //CREN IS CLEARED
        //0 DISABLES CONTINOUS RECEIVE
        RC1STAbits.SPEN = 0;
        RC1STAbits.SPEN = 1;
    }
    receivedFlag = true;
    receivedDataISR = RC1REG;
}

inline bool validateAscii(char charInput) { //CHECK WHETHER INPUT IS ALPHANUMERIC
    bool isAlphaNumeric;
    if ((0x30 <= charInput) && (0x39 >= charInput) ||
            (0x41 <= charInput) && (0x5A >= charInput) ||
            (0x61 <= charInput) && (0x7A >= charInput) ||
            (0x20 == charInput) || (0x2C == charInput)) {
        isAlphaNumeric = true;
    } else {
        isAlphaNumeric = false;
    }
    return isAlphaNumeric;
}

inline void concatenateReceiveArray(void) {
    static uint8_t index = 0;
    static bool receiveHeader = false;
    static bool receiveBody = false;
    static bool completeHeader = false;
    static bool bodyDataReached = false;

    if (receivedFlag) {
        if (validateAscii(receivedDataISR) && receiveBody && completeHeader) {
            //STORE ALPHANUMERIC SYMBOLS
            *(receivedArray + index) = receivedDataISR;
            index++;
            inputSize++;
            if (RECEIVE_ARRAY_SIZE == index) {
                index = 0;
            }
            bodyDataReached = true;
        } else if ((5 == receivedDataISR) && completeHeader) {
            startI2CReception = true;
            completeHeader = false;
        } else if ((4 == receivedDataISR) && bodyDataReached) {
            index = 0;
            buildAddress = true;
            startI2CTransmission = true;
            receiveBody = false;
            completeHeader = false;
            bodyDataReached = false;
        } else if ((3 == receivedDataISR) && bodyDataReached) {
            index = 0;
            buildAddress = true;
            bodyDataReached = false;
        } else if ((2 == receivedDataISR) && completeHeader) {
            receiveBody = true;
        } else if ((1 == receivedDataISR) && !completeHeader && !receiveHeader) {
            receiveHeader = true;
        } else if (receiveHeader && !completeHeader) {

            addressNumber = receivedDataISR;
            receiveHeader = false;
            completeHeader = true;
        }

        receivedFlag = false;
    }
}

//UART TRANSMISSION

inline void transmitData(uint8_t *txData, uint8_t length) {
    //    while (0 == PIR3bits.TX1IF);
    for (uint8_t i = 0; i < length; i++) {
        while (0 == TX1STAbits.TRMT); //TRANSMIT SHIFT REGISTER STATUS BIT->0=TSR FULL
        if ((0xA == *txData) || ((0x20 <= *txData) && (0x7E >= *txData))) {
            // TRANSMIT ONLY LINE FEED OR ALPHANUMERIC SYMBOLS
            TX1REG = *txData++; //TRANSMIT DATA REGISTER
        } else {

            txData++;
        }
    }
}

inline void transmitRAW(uint8_t *txData, uint8_t length) {
    //    while (0 == PIR3bits.TX1IF);
    for (uint8_t i = 0; i < length; i++) {

        while (0 == TX1STAbits.TRMT); //TRANSMIT SHIFT REGISTER STATUS BIT->0=TSR FULL
        TX1REG = *txData++; //TRANSMIT DATA REGISTER
    }
}

inline void transmitAddress(ADDRESS *address) {
    char message[] = "****Address*****\n";
    char separator[] = "****************\n";
    uint8_t lineFeed = '\n';
    transmitData(message, sizeof (message));
    transmitData(address->name, EEPROM_PAGE_SIZE);
    transmitData(&lineFeed, sizeof (lineFeed));
    transmitData(address->telephone, EEPROM_PAGE_SIZE);
    transmitData(&lineFeed, sizeof (lineFeed));
    transmitData(address->street, EEPROM_PAGE_SIZE);
    transmitData(&lineFeed, sizeof (lineFeed));
    transmitData(address->city, EEPROM_PAGE_SIZE);
    transmitData(&lineFeed, sizeof (lineFeed));
    transmitData(&separator, sizeof (separator));
}