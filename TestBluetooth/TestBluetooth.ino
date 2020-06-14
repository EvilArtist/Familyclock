#include <SoftwareSerial.h>
#define BRX 6
#define BTX 7
SoftwareSerial mySerial(BRX, BTX); // RX, TX

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    // set the data rate for the SoftwareSerial port
    mySerial.begin(9600);
    mySerial.println("Hello, world?");
}

void loop()
{ // run over and over
    if (mySerial.available())
    {
        Serial.println(mySerial.read(), HEX);
    }
}
