#include <DS1307.h>
#include <DateTime.h>

// Real Time Clock
#define SCL 8
#define SDA 9

DS1307 rtc(SDA, SCL);

void setup()
{
    Serial.begin(9600);
    Serial.print("begin setup rtc");
    rtc.begin();

    Serial.print("set day");
    rtc.setDOW(FRIDAY);

    Serial.print("set date");
    rtc.setDate(27,7,2019);
     
    Serial.print("set time");

    rtc.setTime(19,35,00);
    
    Serial.print("end setup rtc");
}

void loop()
{
    Serial.print("Read data");
    Time readTime = rtc.getTime();
    Serial.print(rtc.getTimeStr());
    
    // Wait one second before repeating :)
    delay (1000);
}