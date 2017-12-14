# EEPROM-AddressBook
***
This source code shows the implementation of an address book based on a "Microchip" microcontroller (PIC16LF15345), a "STMicroelectronics" EEPROM (M24C08) using I2C and a UART (FTDI) based PC communication. This project is part of a computer science course and has been implemented during an intership (cooperative studies).
The source code has been written using "MPLAB X IDE 4.05" and "XC (v1.44)" as C compiler. You can use either "HTERM" (recommended) or "GTKTERM" as terminal program. The UART interface has been implemented for 9600 Baud.
***
## Create user/address protocol
ASCII control characters have been used to implement this simple protocol. Be aware that for this implementation only 1kByte of EEPROM storage are available. Therefore each address part such as **name**, **telephone number** and so forth is limited to 16 Bytes which corresponds to 16 alphanumerical characters; expressed as a regular expression (consider the space sign inside the square brackets): **[0-9a-zA-Z, ]{1,16}**
#### ASCII Control Characters
* 0x1: Start of Heading
* 0x2: Start of Text
* 0x3: End of Text
* 0x4: End of Transmission
* 0x5: Enquiry
#### Add user/address
Use the following command in **HTERM** to add a new user. Make sure control characters are send as plain numbers/integers and not as their respective ASCII representation. All placeholders except **USER** have to be alphanumerical characters, so numbers as for instance telephone numbers are also send as their respective ASCII representation.

    0x1 USER 0x2 USER_NAME 0x3 TELEPHONE_NUMBER 0x3 STREET 0x3 CITY 0x4

##### Example

    0x1 9 0x2 Max Mustermann 0x3 0123456789 0x3 Strasse 1 0x3 Berlin 0x4
* **USER:** integer (0-15)
* **USER_NAME:** alphanumerical characters (maximum 16 characters)
* **TELEPHONE_NUMBER:** alphanumerical characters (maximum 16 characters)
* **STREET:** alphanumerical characters (maximum 16 characters)
* **CITY:** alphanumerical characters (maximum 16 characters)
***
## Read user/address protocol
This protocol is mostly aquivalent to creating new users. It basically consists of sending the user information **USER** combined with the ASCII control character **0x5**. Be aware that all **USER** values are valid and result in receiving data even if no user has been created previously. This program basically returns whatever is written on the EEPROM which might result in arbitrary data if no user/address has been created beforehand.
#### Enquiring existing users
The procedure is aquivalent to creating new users/addresses. So open a terminal program and type:

    0x1 USER 0x5
    
##### Example

    0x1 9 0x5
##### Terminal Return Example

    ****Address*****
    Max Mustermann
    0123456789
    Musterstrasse 12
    12345, Berlin
    ****************
This formatting is achieved by activating "Newline at LF" (LF = line feed) and unchecking "Show newline characters".
***
## Datasheets
* [Microchip Microcontroller PIC16LF15345](http://ww1.microchip.com/downloads/en/DeviceDoc/40001865B.pdf)
* [STMicroelectronics EEPROM M24C08](http://www.st.com/content/ccc/resource/technical/document/datasheet/cc/f5/a5/01/6f/4b/47/d2/DM00070057.pdf/files/DM00070057.pdf/jcr:content/translations/en.DM00070057.pdf)
