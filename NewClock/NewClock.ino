#include <LedControl.h>
#include <DS1307.h>
#include <DateTime.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include "Effect.h"

// LED Matrix Controller
#define DIN 10
#define CLK 12
#define CS 11
// Real Time Clock
#define SCL 8
#define SDA 9
// LED
#define LEDW 4
#define LED0 5
#define LED1 PIN_A5
#define LED2 PIN_A4
#define LED3 PIN_A3
#define LED4 PIN_A2
//BLuetooth
#define BRX 6
#define BTX 7

uint8_t displayData[32] = {
    0x3C,
    0x42,
    0x81,
    0x81,
    0x81,
    0x81,
    0x42,
    0x3C,
    0xFF,
    0x7F,
    0x3F,
    0x1F,
    0x0F,
    0x07,
    0x03,
    0x01,
    0x01,
    0x03,
    0x07,
    0x0F,
    0x1F,
    0x3F,
    0x7F,
    0xFF,
    0xFF,
    0x81,
    0x81,
    0x81,
    0x81,
    0x81,
    0x81,
    255,
};

uint8_t nextData[32] = {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 255,
                         0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF,
                         0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01,
                         0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C};
const uint8_t Charactor[32][5] = {
    {0x7E, 0x85, 0x99, 0xA1, 0x7E}, //0
    {0x21, 0x41, 0xFF, 0x01, 0x01}, //1
    {0x43, 0x85, 0x89, 0x91, 0x61}, //2
    {0x42, 0x91, 0x91, 0x91, 0x6E}, //3
    {0x08, 0x18, 0x28, 0x48, 0xFF}, //4
    {0xF2, 0x91, 0x91, 0x91, 0x8E}, //5
    {0x7E, 0x89, 0x91, 0x91, 0x4E}, //6
    {0x80, 0x80, 0x8F, 0x90, 0xE0}, //7
    {0x6E, 0x91, 0x91, 0x91, 0x6E}, //8
    {0x72, 0x89, 0x89, 0x89, 0x7E}, //9
    {0x7E, 0x81, 0x81, 0x81, 0x42}, //C - 10
    {0xFF, 0x60, 0x18, 0x06, 0xFF}, //N - 11
    {0x80, 0x80, 0xFF, 0x80, 0x80}, //T - 12
    {0xFF, 0x10, 0x10, 0x10, 0x0F}, //h - 13
    {0x1E, 0x01, 0x41, 0x9E, 0x30}, //ứ - 14
    {0x08, 0x08, 0x08, 0x08, 0x08}, //- - 15
    {0x00, 0x22, 0x77, 0x22, 0x00}, //: - 16
    {0x7E, 0x81, 0x81, 0x89, 0x4E}, //G - 17
    {0x00, 0x11, 0x5F, 0x01, 0x00}, //i - 18
    {0x7F, 0x88, 0x88, 0x88, 0x70}, //P - 19
    {0x3A, 0x49, 0x49, 0x49, 0xBE}, //g - 20
    {0x60, 0x90, 0x90, 0x60, 0x00}  //˚
};

LedControl ledController = LedControl(DIN, CLK, CS, 4);
DS1307 rtc(SDA, SCL);
SoftwareSerial bluetooth(BRX, BTX);

uint8_t _systemEffectMode = 1;
const uint8_t NUMBER_OF_STATE = 6;
uint8_t _systemDataState = 0;
bool _effectChangeRequest = false;

bool isDotShown = false;
uint8_t _systemMode = 0;
uint8_t resettime = 0;
uint8_t _currentMinute = 0;
espace::Effect* effect;
unsigned long systemTime;

void Display();
void UpdateMode();
void ChangeEffect();
void ReadData();

void setup()
{
    for (int index = 0; index < ledController.getDeviceCount(); index++)
    {
        ledController.shutdown(index, false);
        ledController.setIntensity(index, 4);
    }
    Display();
    rtc.begin();
    
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
    pinMode(LEDW, OUTPUT);
    
    digitalWrite(LEDW, HIGH);
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, HIGH);
    // Serial.begin(9600);
    _systemDataState = 0;
    bluetooth.begin(9600);
    resettime = rtc.getTime().date;
    EEPROM.write(0, resettime);
    ReadData();
    ChangeEffect();
    delay(2000);
    systemTime = millis();
}

void loop()
{
    if (effect->State == espace::End)
    {
        _systemDataState ++;
        _systemDataState = _systemDataState % NUMBER_OF_STATE;
        ReadData();
        ChangeEffect();
    }
    if (effect->State == espace::Holding && _systemDataState < NUMBER_OF_STATE - 1)
    {
        if (millis() - systemTime > 500)
        {
            isDotShown = !isDotShown;
            systemTime = millis();
        }
        ReadData();
        effect->Assign(displayData, nextData);
    }
    effect->Apply(displayData, nextData);  
    Display();
    CheckBluetoothState();
    delay(10);
}

//--------------------------------------------------------------------------------
// Display
//-------------------------------------------------------------------------------
void Display()
{
    uint8_t shipper = 0;
    uint8_t row = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        shipper = 0x80;
        for (uint8_t j = 0; j < 8; j++)
        {
            row = 0x00;
            if ((displayData[i * 8 + 0] & shipper) != 0x00)
            {
                row = row | 128;
            }
            if ((displayData[i * 8 + 1] & shipper) != 0x00)
            {
                row = row | 64;
            }
            if ((displayData[i * 8 + 2] & shipper) != 0x00)
            {
                row = row | 32;
            }
            if ((displayData[i * 8 + 3] & shipper) != 0x00)
            {
                row = row | 16;
            }
            if ((displayData[i * 8 + 4] & shipper) != 0x00)
            {
                row = row | 8;
            }
            if ((displayData[i * 8 + 5] & shipper) != 0x00)
            {
                row = row | 4;
            }
            if ((displayData[i * 8 + 6] & shipper) != 0x00)
            {
                row = row | 2;
            }
            if ((displayData[i * 8 + 7] & shipper) != 0x00)
            {
                row = row | 1;
            }
            ledController.setRow(3 - i, j, row);
            shipper = shipper >> 1;
        }
    }
}

//--------------------------------------------------------------------------
// LED Transition Effects
//-------------------------------------------------------------------------

void ChangeEffect()
{
    uint8_t mode;
    mode = random(66);
    mode = mode % 6;
    
    if (effect != nullptr)
    {
        delete effect;
    }
    switch (mode)
    {
    case 0:
        effect = new espace::RowTransition();
        break;
    case 1:
        effect = new espace::ScanTransition();
        break;
    case 2:
        effect = new espace::FlyTransition();
        break;
    case 3:
        effect = new espace::RandomTransition();
        break;
    case 4:
        effect = new espace::ZoomTransition();
        break;
    case 5:
        effect = new espace::InvertTransition();
        break;
    default:
        effect = new espace::RowTransition();
        break;
    }
    if(_systemDataState == 0)
    {
        effect->HoldingDuration = 100;
    }
}

//--------------------------------------------------------------------------------------
// Real Time Controller
//--------------------------------------------------------------------------------------
void ReadData()
{
    ClearData();

    Time readTime = rtc.getTime();
    if (readTime.date > resettime && readTime.sec > 5)
    {
        rtc.setTime(readTime.hour, readTime.min, readTime.sec - 10);
        resettime = readTime.date;
        EEPROM.update(0, resettime);
    }

    if (_systemDataState > NUMBER_OF_STATE - 1 || _systemDataState == 0)
    {
        digitalWrite(LED0, HIGH);
        if (_systemMode == 0) //Normal
        {
            Insert2displayData(0, (readTime.hour % 100) / 10);
            Insert2displayData(1, readTime.hour % 10);
            if (isDotShown)
            {
                Insert2displayData(2, 16);
            }
            Insert2displayData(3, (readTime.min % 100) / 10);
            Insert2displayData(4, readTime.min % 10);
        }
        else if (_systemMode == 1)
        {
            Insert2displayData(0, (readTime.hour % 100) / 10);
            Insert2displayData(1, readTime.hour % 10);
            Insert2displayData(3, 17);
            Insert2displayData(4, 18);
        }
        else if (_systemMode == 2)
        {
            Insert2displayData(3, (readTime.hour % 100) / 10);
            Insert2displayData(4, readTime.hour % 10);
            Insert2displayData(0, 19);
            Insert2displayData(1, 13);
        }
    }
    else if (_systemDataState == 1)
    {
        digitalWrite(LED1, HIGH);
        isDotShown = false;
        if (readTime.dow > 1)
        {
            Insert2displayData(0, 12, true);
            Insert2displayData(1, 13, true);
            Insert2displayData(2, 14, true);
            Insert2displayData(3, readTime.dow % 10, true);
        }
        else
        {
            Insert2displayData(1, 10, true);
            Insert2displayData(2, 11, true);
        }
    }
    else if (_systemDataState == 2)
    {
        digitalWrite(LED2, HIGH);
        if (_systemMode == 0)
        {
            Insert2displayData(0, (readTime.date % 100) / 10);
            Insert2displayData(1, readTime.date % 10);
            Insert2displayData(2, 15);
            Insert2displayData(3, (readTime.mon % 100) / 10);
            Insert2displayData(4, readTime.mon % 10);
        }
        else if (_systemMode == 5)
        {
            Insert2displayData(0, (readTime.date % 100) / 10);
            Insert2displayData(1, readTime.date % 10);
            Insert2displayData(3, 11);
            Insert2displayData(4, 20);
        }
        else if (_systemMode == 4)
        {
            Insert2displayData(0, 11);
            Insert2displayData(1, 20);
            Insert2displayData(3, 12);
            Insert2displayData(4, 13);
        }
    }
    else if (_systemDataState == 3)
    {
        digitalWrite(LED3, HIGH);
        Date d(readTime.date, readTime.mon, readTime.year % 100);
        Date lunarDate = d.getLunarDateMonth();
        Insert2displayData(0, (lunarDate._date % 100) / 10);
        Insert2displayData(1, lunarDate._date % 10);
        Insert2displayData(2, 15);
        Insert2displayData(3, (lunarDate._month % 100) / 10);
        Insert2displayData(4, lunarDate._month % 10);
    }
    else if (_systemDataState == 4)
    {
        digitalWrite(LED4, HIGH);
        Insert2displayData(0, 2, true);
        Insert2displayData(1, 0, true);
        Insert2displayData(2, (readTime.year % 100) / 10, true);
        Insert2displayData(3, readTime.year % 10, true);
    }
    else if (_systemDataState == 5)
    {
        digitalWrite(LEDW, HIGH);
        
        float temperature = 0.0;
        for (uint8_t i = 0; i < 10; i++)
        {
            temperature += (5.0 * analogRead(A1) * 100.0 / 1024.0);
        }
        temperature = temperature/10;
        
        uint8_t tpInt = (uint8_t)temperature;
        tpInt -=5;
        uint8_t c = (tpInt % 100) / 10;
        uint8_t d = tpInt % 10;
        Insert2displayData(0, c, true);
        Insert2displayData(1, d, true);
        Insert2displayData(2, 21, true);
        Insert2displayData(3, 10, true);
    }
}

void Insert2displayData(uint8_t position, uint8_t charactorIndex, bool isFourSlot)
{
    uint8_t charactorPosition = 0;
    if (isFourSlot)
    {
        charactorPosition = position * 6 + 5;
    }
    else
    {
        charactorPosition = position * 6 + 1;
    }
    for (uint8_t i = 0; i < 5; i++)
    {
        nextData[charactorPosition + i] = Charactor[charactorIndex][i];
    }
}

void Insert2displayData(uint8_t position, uint8_t charactorIndex)
{
    Insert2displayData(position, charactorIndex, false);
}

void ClearData()
{
    for (uint8_t i = 0; i < 32; i++)
    {
        nextData[i] = 0;
    }
    digitalWrite(LEDW, LOW);
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
}

void ShiftdisplayData(uint8_t shiftbyte)
{
    for (uint8_t i = 0; i < 31; i++)
    {
        displayData[i] = displayData[i + 1];
    }
    displayData[31] = shiftbyte;
}

//------------------------------------------------------------------------------------
//BLuetooth
//------------------------------------------------------------------------------------
void CheckBluetoothState()
{
    if (_systemMode != 0)
    {
        return;
    }
    if (bluetooth.available())
    {
        uint8_t HH = bluetooth.read();
        if (HH > 24)
        {
            uint8_t s = bluetooth.read();
            for (int index = 0; index < ledController.getDeviceCount(); index++)
            {
                ledController.shutdown(index, false);
                ledController.setIntensity(index, s % 16);
            }
        }
        else
        {
            uint8_t mm = bluetooth.read();
            uint8_t dayOfWeek = bluetooth.read();
            uint8_t d = bluetooth.read();
            uint8_t month = bluetooth.read();
            uint16_t y = 2000 + bluetooth.read();

            rtc.setTime(HH, mm, 0);
            rtc.setDate(d, month, y);
            rtc.setDOW(dayOfWeek);

            digitalWrite(LED0, HIGH);
            digitalWrite(LEDW, HIGH);
            digitalWrite(LED1, HIGH);
            digitalWrite(LED2, HIGH);
            digitalWrite(LED3, HIGH);
            digitalWrite(LED4, HIGH);
            delay(200);

            digitalWrite(LEDW, LOW);
            digitalWrite(LED0, LOW);
            digitalWrite(LED1, LOW);
            digitalWrite(LED2, LOW);
            digitalWrite(LED3, LOW);
            digitalWrite(LED4, LOW);
            delay(200);

            digitalWrite(LED0, HIGH);
            digitalWrite(LEDW, HIGH);
            digitalWrite(LED1, HIGH);
            digitalWrite(LED2, HIGH);
            digitalWrite(LED3, HIGH);
            digitalWrite(LED4, HIGH);
            delay(200);

            digitalWrite(LED0, LOW);
            digitalWrite(LEDW, LOW);
            digitalWrite(LED1, LOW);
            digitalWrite(LED2, LOW);
            digitalWrite(LED3, LOW);
            digitalWrite(LED4, LOW);
            delay(200);

            resettime = d;
            EEPROM.update(0, resettime);
        }
        
        while (bluetooth.available())
        {
            bluetooth.read();
        }
    }
}
