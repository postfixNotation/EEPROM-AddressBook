#include "declarations.h"

//INTERRUPT SERVICE ROUTINE

void interrupt ISR(void) {
    if (PIE3bits.SSP1IE && PIR3bits.SSP1IF) {
        PIR3bits.SSP1IF = 0;
    } else if (PIE3bits.BCL1IE && PIR3bits.BCL1IF) { //BUS COLLISION
        //INTERRUPT ENABLE BIT: ENABLED/DISABLED->1/0
        //BUS COLLISION INTERRUPT FLAG BIT: 1->BUS COLLISION WAS
        //DETECTED
        PIR3bits.BCL1IF = 0;
    } else if (PIE3bits.RC1IE && PIR3bits.RC1IF) {
        receiveData_ISR();
    }
}
