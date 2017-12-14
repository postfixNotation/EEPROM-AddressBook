#include <xc.h>
#include <stdint.h>
#include "stdbool.h"

#define DEVICE_ADDRESS 0b10100
#define EEPROM_CAPACITY 1024
#define EEPROM_BLOCK_SIZE 256
#define EEPROM_PAGE_SIZE 16

#define RECEIVE_ARRAY_SIZE 16

#define INPUT 1
#define OUTPUT 0

#define HIGH 1
#define LOW 0

#define ANALOG 1
#define DIGITAL 0

#define HFINTOSC_NO_DIVIDER 0x60
#define EIGHT_MHZ 0x3
#define ENABLE_HFOSC 1
#define HFINTOSC 0x6

#define RB5TX 0
#define RB6RX 1
#define ASYNC 0
#define EIGHTBIT 0
#define LOWBAUD 0
#define EIGHTBIT_BAUD 0
#define BAUD_RATE 0xC

#define ENABLE_TRANSMIT 1
#define ENABLE_SERIAL 1
#define ENABLE_CONTINOUS 1
#define ENABLE_USART_INT 1
#define DISABLE_USART_INT 0
#define DISABLE_TRANSMIT_INT 0

#define ACK_RECEIVED 0
#define NACK_RECEIVED 1

#define LED_PIN TRISAbits.TRISA2
#define LED LATAbits.LATA2

#define ACKNOWLEDGE (SSP1CON2 & 0x40)

#define _XTAL_FREQ 8000000

#pragma config RSTOSC = HFINTOSC //CONFIGURATION WORD 1 RSTOSC
//RSTOSC POWER-UP DEFAULT VALUE FOR COSC BITS
//0b110->HFINTOSC->4MHz
#pragma config WDTE = OFF //WDT OPERATION MODE->WDT DISABLED
#pragma config PPS1WAY = OFF //PERIPHERAL PIN SELECT ONE-WAY CONTROL
//->THE PPSLOCK BIT CAN BE CLEARED AND
//SET ONLY ONCE IN SOFTWARE

//DECLARATIONS ADDRESS BOOK

typedef struct {
    char telephone[EEPROM_PAGE_SIZE]; //USED AS ID
    char name[EEPROM_PAGE_SIZE];
    char street[EEPROM_PAGE_SIZE];
    char city[EEPROM_PAGE_SIZE];
} ADDRESS;

typedef enum {
    NAME, TELEPHONE, STREET, CITY
} SECTION;

void setTelephone(ADDRESS*, char*, uint8_t);
void setName(ADDRESS*, char*, uint8_t);
void setStreet(ADDRESS*, char*, uint8_t);
void setCity(ADDRESS*, char*, uint8_t);

//DECLARATIONS UART
void uart_setup(void);
inline void receiveData(uint8_t*);
inline void transmitData(uint8_t*, uint8_t);
inline void transmitRAW(uint8_t*, uint8_t);
inline void transmitAddress(ADDRESS*);
inline void concatenateReceiveArray(void);
inline void receiveData_ISR(void);
inline bool validateAscii(char);

//ISR GLOBAL VARIABLES
volatile uint8_t receivedDataISR;
volatile bool receivedFlag = false;
volatile uint8_t receivedArray[RECEIVE_ARRAY_SIZE];
volatile uint8_t addressNumber;
volatile uint8_t inputSize = 0;
volatile bool startI2CTransmission = false;
volatile bool startI2CReception = false;
volatile bool buildAddress = false;
volatile SECTION currentSection = NAME;

//DECLARATIONS I2C
void i2c_setup(void);
void setupLED(void);
void isIdle(void);
void startI2C(void);
void restartI2C(void);
void stopI2C(void);

void writeByte(uint8_t);
void readCurrentByte(uint8_t*, uint8_t, uint16_t);
void readRandomByte(uint8_t*, uint8_t, uint16_t);
void readCurrentSEQ(uint8_t*, uint8_t, uint16_t, uint8_t);
void readRandomSEQ(uint8_t*, uint8_t, uint16_t, uint8_t);
void writeArray(uint8_t*, uint8_t, uint16_t, uint8_t);

void writePage(uint8_t*, uint8_t, uint16_t);
void readPage(uint8_t*, uint8_t, uint16_t);
void writeAddress(ADDRESS*, uint8_t, uint16_t);
void readAddress(ADDRESS*, uint8_t, uint16_t);

void isACK(void);
void delay(void);
void blinkLED(uint8_t);
void sendACK(void);
void sendNACK(void);
uint8_t calcBlockAddress(uint16_t);
uint8_t calcByteAddress(uint16_t);
uint16_t userToByteAddress(uint8_t);

//WRITE 0b1010 0000 = 0xA0
//READ  0b1010 0001 = 0xA1