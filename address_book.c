#include "declarations.h"

void setTelephone(ADDRESS *address, char* telephone, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        *((address->telephone) + i) = *(telephone + i);
    }

    if (EEPROM_PAGE_SIZE >= length) {
        for (uint8_t j = length; j < EEPROM_PAGE_SIZE; j++) {
            *((address->telephone) + j) = 0xFF;
        }
    }
}

void setName(ADDRESS *address, char* name, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        *((address->name) + i) = *(name + i);
    }

    if (EEPROM_PAGE_SIZE >= length) {
        for (uint8_t j = length; j < EEPROM_PAGE_SIZE; j++) {
            *((address->name) + j) = 0xFF;
        }
    }
}

void setStreet(ADDRESS *address, char* street, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        *((address->street) + i) = *(street + i);
    }

    if (EEPROM_PAGE_SIZE >= length) {
        for (uint8_t j = length; j < EEPROM_PAGE_SIZE; j++) {
            *((address->street) + j) = 0xFF;
        }
    }
}

void setCity(ADDRESS *address, char* city, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        *((address->city) + i) = *(city + i);
    }

    if (EEPROM_PAGE_SIZE >= length) {
        for (uint8_t j = length; j < EEPROM_PAGE_SIZE; j++) {
            *((address->city) + j) = 0xFF;
        }
    }
}
