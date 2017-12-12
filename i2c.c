#include "I2C_Header.h"

void i2c_setup(void) {
    LATC = 0x00; //PORTC DATA LATCH REGISTER
    LATBbits.LATB7 = HIGH;
    TRISC = 0xFF; //PORTC TRI-STATE REGISTER: 1->INPUT (TRI-STATED)
    //0->OUTPUT
    TRISBbits.TRISB7 = OUTPUT;
    ANSELC = 0x3C; //PORTC ANALOG SELECT REGISTER: 1->ANALOG INPUT
    //0->DIGITAL I/O
    //RC0,RC1->DIGITAL I/0

    //PERIPHERAL PIN SELECT (PSS) - REMAPPING
    bool state = (unsigned char) GIE; //GLOBAL INTERRUPT ENABLE BIT
    GIE = 0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; //UNLOCK PPS
    //PPSLOCK->PPS LOCK REGISTER->PPSLOCKED: 1->PPS IS LOCKED
    //0->PPS IS NOT LOCKED

    RC0PPS = 0x15; //RC0->MSSP1:SCL1; OUTPUT
    SSP1CLKPPSbits.SSP1CLKPPS = 0x10; //RC0->MSSP1:SCL1; INPUT
    SSP1DATPPSbits.SSP1DATPPS = 0x11; //RC1->MSSP1:SDA1; INPUT
    RC1PPS = 0x16; //RC1->MSSP1:SDA1; OUTPUT

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; //LOCK PPS

    GIE = state;

    //OSCILLATOR
    OSCCON1 = HFINTOSC_NO_DIVIDER; //110->HFINTOSC 000->1 CLOCK DIVIDER
    OSCENbits.HFOEN = ENABLE_HFOSC; //HFINTOSC ENABLED
    OSCFRQbits.HFFRQ = EIGHT_MHZ; //HFINTOSC FREQUENCY SELECTION REGISTER->8MHz

    //MASTER SYNCHRONOUS SERIAL PORT (MSSP1)
    //I2C
    SSP1STAT = 0x80; //SSP1 STATUS REGISTER->SLEWRATE CONTROL DISABLED FOR
    //STANDARD SPEED MODE (100kHz AND 1MHz)
    SSP1CON1 = 0x28; //SSP1 CONTROL REGISTER 1;SSPEN->SYNCHRONOUS SERIAL
    //PORT ENABLE BIT->ENABLES SERIAL PORT (SDA AND SCL)
    //I2C MASTER MODE;CLOCK=FOSC/(4*(SSP1ADD+1))
    SSP1CON3 = 0x00; //SSP1 CONTROL REGISTER 3
    SSP1ADD = 0x13; //MSSP1 ADDRESS AND BAUD RATE REGISTER (I2C MODE)
    //BAUD RATE CLOCK DIVIDER BITS: 0x13->FOSC=8MHz->CLOCK=400kHz

    INTCONbits.GIE = 1; //INTERRUPT CONTROL REGISTER->GLOBAL INTERRUPT
    //ENABLE BIT: 1->ENABLES ALL ACTIVE INTERRUPTS
    INTCONbits.PEIE = 1; //PERIPHERAL INTERRUPT ENABLE BIT: 1->ENABLES ALL
    //ACTIVE PERIPHERAL INTERRUPTS

    PIR3bits.SSP1IF = 0; //PERIPHERAL INTERRUPT REQUEST REGISTER 3
    //SYNCHRONOUS SERIAL PORT (MSSP1) INTERRUPT FLAG BIT: 1->TRANSMISSION/
    //RECEPTION/BUS CONDITION IS COMPLETE: 0->WAITING FOR TRANSMISSION/
    //RECEPTION/BUS CONDITION IN PROGRESS
    PIE3bits.SSP1IE = 1; //PERIPHERAL INTERRUPT ENABLE REGISTER 3
    //SYNCHRONOUS SERIAL PORT (MSSP1) INTERRUPT ENABLE BIT: 1->ENABLES THE
    //MSSP INTERRUPT
}

void isACK(void) {
    if (!(SSP1CON2 & 0x40)) { //ACKSTAT->ACKNOWLEDGE WAS (NOT) RECEIVED
        LATAbits.LATA2 = LOW;
    } else {
        LATAbits.LATA2 = HIGH;
    }
}

void sendACK(void) {
    SSP1CON2 &= 0xDF; //SSP1CON2 = SSP1CON2 & 1101 1111->ACKDT->ACKNOWLEDGE
    SSP1CON2 |= 0x10; //SSP1CON2 = SSP1CON2 | 0001 0000->ACKEN->INITIATE ACK SEQ
    while (SSP1CON2 & 0x10);
}

void sendNACK(void) {
    SSP1CON2 |= 0x20; //SSP1CON2 = SSP1CON2 | 0010 000->ACKDT->NOT ACKNOWLEDGE
    SSP1CON2 |= 0x10;
    while (SSP1CON2 & 0x10);
}

uint8_t calcBlockAddress(uint16_t byteAddress) { //0 <= BYTEADDRESS <= 1023
    if ((EEPROM_CAPACITY - 1) < byteAddress) {
        return (EEPROM_CAPACITY / EEPROM_BLOCK_SIZE); //RETURN INVALID VALUE
        //->FORCE NACK
    }
    return (uint8_t) (byteAddress / EEPROM_BLOCK_SIZE);
}

uint8_t calcByteAddress(uint16_t byteAddress) {
    return (uint8_t) (byteAddress % EEPROM_BLOCK_SIZE);
}

uint16_t userToByteAddress(uint8_t user) { //USER IS SYNONYMOUS WITH ADDRESS
    return (0x40 * user); //NOT SCALABLE->ALWAYS CONSIDER SIZE OF AN ADDRESS
}

void readRandomByte(uint8_t *result, uint8_t deviceAddress, uint16_t byteAddress) {
    readRandomSEQ(result, deviceAddress, byteAddress, 1);
}

void readCurrentByte(uint8_t *result,
        uint8_t deviceAddress,
        uint16_t byteAddress) {
    readCurrentSEQ(result, deviceAddress, byteAddress, 1);
}

void readCurrentSEQ(uint8_t *result,
        uint8_t deviceAddress,
        uint16_t byteAddress,
        uint8_t numberOfBytes) {
    isIdle();
    startI2C();

    uint8_t block = calcBlockAddress(byteAddress);
    block = (uint8_t) ((block << 1) & 0x06); //0b00000011 -> 0b00000110
    deviceAddress = (uint8_t) ((deviceAddress << 3) | 0x1); //0b10100 -> 0b10100001
    deviceAddress = (uint8_t) (deviceAddress | block); //0b10100001 | 0b00000110
    //->0b10100111
    writeByte(deviceAddress);
    isIdle();

    for (uint8_t i = 1; i < numberOfBytes; i++) {
        SSP1CON2 |= 0x08; //RCEN->RECEIVE ENABLE BIT: 1->ENABLES RECEIVE MODE
        while (!(SSP1STAT & 0x01));
        sendACK();
        *result++ = SSP1BUF;
    }

    SSP1CON2 |= 0x08; //RCEN->RECEIVE ENABLE BIT: 1->ENABLES RECEIVE MODE
    while (!(SSP1STAT & 0x01));
    sendNACK();
    *result++ = SSP1BUF;

    isIdle();
    stopI2C();
}

void readRandomSEQ(uint8_t *result,
        uint8_t deviceAddress,
        uint16_t byteAddress,
        uint8_t numberOfBytes) {
    isIdle();
    startI2C();

    uint8_t block = calcBlockAddress(byteAddress);
    block = (uint8_t) ((block << 1) & 0x06); //0b00000011 -> 0b00000110
    deviceAddress = (uint8_t) ((deviceAddress << 3) & 0xFE); //0b10100 -> 0b10100000
    deviceAddress = (uint8_t) (deviceAddress | block); //0b10100000 | 0b00000110
    //->0b10100110
    writeByte(deviceAddress);

    byteAddress = calcByteAddress(byteAddress);
    isIdle();
    writeByte(byteAddress);
    isIdle();
    restartI2C();

    deviceAddress = (uint8_t) (deviceAddress | 0x01);
    writeByte(deviceAddress);
    isIdle();

    for (uint8_t i = 1; i < numberOfBytes; i++) {
        SSP1CON2 |= 0x08; //RCEN->RECEIVE ENABLE BIT: 1->ENABLES RECEIVE MODE
        while (!(SSP1STAT & 0x01));
        sendACK();
        *result++ = SSP1BUF;
    }

    SSP1CON2 |= 0x08; //RCEN->RECEIVE ENABLE BIT: 1->ENABLES RECEIVE MODE
    while (!(SSP1STAT & 0x01));
    *result++ = SSP1BUF;
    sendNACK();

    isIdle();
    stopI2C();
}

void readPage(uint8_t *result,
        uint8_t deviceAddress,
        uint16_t byteAddress) {
    readRandomSEQ(result, deviceAddress, byteAddress, EEPROM_PAGE_SIZE);
}

void writeByte(uint8_t transmitData) {
    while (SSP1STAT & 0x01); //BUFFER FULL STATUS BIT: 1->DATA TRANSMIT IN
    //PROGRESS: 0->DATA TRANSMIT COMPLETE
    if (SSP1CON1 & 0x80) {//WCOL WRITE COLLISION DETECT BIT (TRANSMIT ONLY)
        //1->SSP1BUF IS WRITTEN WHILE IT IS STILL TRANSMITTING
        //0->NO COLLISION
        SSP1CON1 &= 0x7F; //CLEAR SSP1CON1 MSB->WCOL
    }
    SSP1BUF = transmitData; //SSP1BUF MSSP1 BUFFER REGISTER->SEND DATA
    while (SSP1STAT & 0x01);
}

void writeArray(uint8_t *transmitData,
        uint8_t deviceAddress,
        uint16_t byteAddress,
        uint8_t length) {
    isIdle();
    startI2C();

    uint8_t block = calcBlockAddress(byteAddress);
    block = (uint8_t) ((block << 1) & 0x06); //0b00000011 -> 0b00000110
    deviceAddress = (uint8_t) ((deviceAddress << 3) & 0xFE); //0b10100 -> 0b10100000
    deviceAddress = (uint8_t) (deviceAddress | block); //0b10100000 | 0b00000110
    //->0b10100110
    writeByte(deviceAddress);

    byteAddress = calcByteAddress(byteAddress);
    isIdle();
    writeByte(byteAddress);
    isIdle();

    for (int i = 0; i < length; i++) {
        writeByte(*transmitData++);
        isIdle();
    }

    stopI2C();
}

void writePage(uint8_t *transmitData,
        uint8_t deviceAddress,
        uint16_t byteAddress) {
    writeArray(transmitData, deviceAddress, byteAddress, EEPROM_PAGE_SIZE);
}

void writeAddress(ADDRESS *address,
        uint8_t deviceAddress,
        uint16_t byteAddress) {
    writePage(address->name, deviceAddress, byteAddress);
    byteAddress += 0x10;
    __delay_ms(4);
    writePage(address->telephone, deviceAddress, byteAddress);
    byteAddress += 0x10;
    __delay_ms(4);
    writePage(address->street, deviceAddress, byteAddress);
    byteAddress += 0x10;
    __delay_ms(4);
    writePage(address->city, deviceAddress, byteAddress);
    __delay_ms(4);
}

void readAddress(ADDRESS *address,
        uint8_t deviceAddress,
        uint16_t byteAddress) {
    readPage(address->name, deviceAddress, byteAddress);
    byteAddress += 0x10;
    readPage(address->telephone, deviceAddress, byteAddress);
    byteAddress += 0x10;
    readPage(address->street, deviceAddress, byteAddress);
    byteAddress += 0x10;
    readPage(address->city, deviceAddress, byteAddress);
}

void stopI2C(void) {
    SSP1CON2 |= 0x04; //SSP1 CONTROL REGISTER->STOP CONDITION ENABLE BIT
    while (SSP1CON2 & 0x04); //INITIATE STOP CONDITION->AUTOMATICALLY CLEARED
    //BY HARDWARE
}

void startI2C(void) {
    SSP1CON2 |= 0x01; //INITIATE START CONDITION->AUTOMATICALLY CLEARED
    //BY HARDWARE
    while (SSP1CON2bits.SEN);
}

void restartI2C(void) {
    SSP1CON2 |= 0x02; //INITIATE REPEATED START CONDITION->AUTOMATICALLY
    //CLEARED BY HARDWARE
    while (SSP1CON2bits.RSEN);
}

void isIdle(void) {
    while ((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
    //0b0000 0100 = 0x04
    //0b0000 0001 = 0x01
    //R/W READ/WRITE BIT INFORMATION: 1->TRANSMIT IS IN PROGRESS
    //0->TRANSMIT IS NOT IN PROGRESS
    //SEN->START CONDITION ENABLE/STRETCH ENABLE BIT
    //PEN->STOP CONDITION ENABLE BIT
    //RSEN->REPEATED START CONDITION ENABLE BIT
    //RCEN->RECEIVE ENABLE BIT
    //ACKEN->ACKNOWLEDGE SEQUENCE ENABLE BIT
}