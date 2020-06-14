#include <LedControl.h>
#include <DS1307.h>
#include <DateTime.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

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
#define LED1 A5
#define LED2 A4
#define LED3 A3
#define LED4 A2
//BLuetooth
#define BRX 6
#define BTX 7

uint8_t _data[32] = {
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

uint8_t _nextData[32] = {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 255,
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

uint8_t _currentEffectState = 0;
uint8_t _systemEffectMode = 1;
const uint8_t NUMBER_OF_STATE = 6;
uint8_t _systemDataState = 0;
bool _effectChangeRequest = false;

bool _isDotShown = false;
uint8_t _systemMode = 0;
uint8_t resettime = 0;

void Display();
void UpdateMode();
void SelectEffectMode();
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

    rtc.setDOW(SATURDAY);

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
    
    _systemDataState = NUMBER_OF_STATE - 1;
    bluetooth.begin(9600);
    resettime = rtc.getTime().date;
    EEPROM.write(0, resettime);
    delay(2000);
}

void loop()
{
    ReadData();
    UpdateMode();
    Display();
    CheckBluetoothState();
    delay(5);
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
            if ((_data[i * 8 + 0] & shipper) != 0x00)
            {
                row = row | 128;
            }
            if ((_data[i * 8 + 1] & shipper) != 0x00)
            {
                row = row | 64;
            }
            if ((_data[i * 8 + 2] & shipper) != 0x00)
            {
                row = row | 32;
            }
            if ((_data[i * 8 + 3] & shipper) != 0x00)
            {
                row = row | 16;
            }
            if ((_data[i * 8 + 4] & shipper) != 0x00)
            {
                row = row | 8;
            }
            if ((_data[i * 8 + 5] & shipper) != 0x00)
            {
                row = row | 4;
            }
            if ((_data[i * 8 + 6] & shipper) != 0x00)
            {
                row = row | 2;
            }
            if ((_data[i * 8 + 7] & shipper) != 0x00)
            {
                row = row | 1;
            }
            ledController.setRow(3 - i, j, row);
            shipper = shipper >> 1;
        }
    }
}

void UpdateMode()
{
    if (_systemMode > 0)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            _data[i] = _nextData[i];
        }
    }
    else
    {
        if (_systemDataState > NUMBER_OF_STATE)
        {
            NoEffect();
        }
        else
        {
            if (_systemDataState == 0)
            {
                _systemEffectMode = 1;
            }
            SelectEffectMode();
        }
    }
}

//--------------------------------------------------------------------------
// LED Transition Effects
//-------------------------------------------------------------------------
void NoEffect()
{
    if (_currentEffectState < 7)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            _data[i] = _nextData[i];
        }
        _currentEffectState++;
    }
    else
    {
        NextEffect();
    }
}

void RowTransition()
{
    uint8_t Collumn = 0x00;
    if (_currentEffectState < 9)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            if (i % 2 == 0)
                Collumn = _nextData[i] >> (8 - _currentEffectState);
            else
                Collumn = _nextData[i] << (8 - _currentEffectState);
            _data[i] = Collumn;
        }
    }
    else if (_currentEffectState == 30)
    {
        NextEffect();
    }
}

void RowOut()
{
    uint8_t collumn = 0x00;
    if (_currentEffectState < 9)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            if (i % 2 == 0)
                collumn = _nextData[i] >> _currentEffectState;
            else
                collumn = _nextData[i] << _currentEffectState;
            _data[i] = collumn;
        }
    }
    else if (_currentEffectState == 30)
    {
        NextEffect();
    }
}

void NonSynchronizeShift()
{
    if (_currentEffectState < 156)
    {
        uint8_t cummulate = 0;
        if (_currentEffectState > 29)
            cummulate = (_currentEffectState - 28) / 4;
        for (uint8_t i = 31 - cummulate; i > 0; i--)
        {
            _data[i] = _data[i - 1];
        }
        if (_currentEffectState % 5 == 0)
        {
            _data[0] = _nextData[31 - _currentEffectState / 5];
        }
        else
        {
            _data[0] = 0;
        }
    }
    else if (_currentEffectState == 220)
    {
        NextEffect();
    }
}

void NonSynchronizeUnshift()
{
    if (_currentEffectState < 200)
    {
        uint8_t cummulate = _currentEffectState / 4;
        // if (_currentEffectState > 27) cummulate = (_currentEffectState - 28)/4;
        if (_currentEffectState % 4 == 0)
        {
            if (cummulate > 0 && cummulate < 32)
            {
                _data[cummulate - 1] = _nextData[cummulate];
                _data[cummulate] = 0;
            }
        }
        else
        {
            if (cummulate >= 0 && cummulate < 32)
                _data[cummulate] = 0;
        }
        if (cummulate > 31)
            cummulate = 31;
        for (uint8_t i = 0; i < cummulate; i++)
        {
            _data[i] = _data[i + 1];
        }
    }
    else if (_currentEffectState == 200)
    {
        NextEffect();
    }
}

void ScanTwice()
{
    if (_currentEffectState < 32)
    {
        _data[_currentEffectState] = 0xFF;
        _data[31 - _currentEffectState] = 0xFF;
        if (_currentEffectState > 0 && _currentEffectState < 16)
        {
            _data[_currentEffectState - 1] = _nextData[_currentEffectState - 1] & 0xAA;
            _data[32 - _currentEffectState] = _nextData[32 - _currentEffectState] & 0x55;
            _data[_currentEffectState] = 0xFF;
            _data[31 - _currentEffectState] = 0xFF;
        }
        else if (_currentEffectState > 15 && _currentEffectState < 32)
        {
            _data[_currentEffectState] = _nextData[_currentEffectState];
            _data[31 - _currentEffectState] = _nextData[31 - _currentEffectState];
            if (_currentEffectState < 31)
            {
                _data[_currentEffectState + 1] = 0xFF;
                _data[30 - _currentEffectState] = 0xFF;
            }
        }
    }
    else if (_currentEffectState == 50)
    {
        NextEffect();
    }
}

void ReScanTwice()
{
    if (_currentEffectState < 32)
    {
        _data[_currentEffectState] = 0xFF;
        _data[31 - _currentEffectState] = 0xFF;
        if (_currentEffectState > 0 && _currentEffectState < 16)
        {
            _data[_currentEffectState - 1] = _nextData[_currentEffectState - 1] & 0x55;
            _data[32 - _currentEffectState] = _nextData[32 - _currentEffectState] & 0xAA;
            _data[_currentEffectState] = 0xFF;
            _data[31 - _currentEffectState] = 0xFF;
        }
        else if (_currentEffectState > 15 && _currentEffectState < 32)
        {
            _data[_currentEffectState] = 0;
            _data[31 - _currentEffectState] = 0;
            if (_currentEffectState < 31)
            {
                _data[_currentEffectState + 1] = 0xFF;
                _data[30 - _currentEffectState] = 0xFF;
            }
        }
    }
    if (_currentEffectState == 35)
    {
        NextEffect();
    }
}

void FlyIn()
{
    if (_currentEffectState < 60)
    {
        uint8_t Emax;
        if (_currentEffectState > 7)
            Emax = 8;
        else
            Emax = _currentEffectState;
        for (uint8_t i = 0; i <= Emax; i++)
        {
            if (_currentEffectState - i < 32)
            {
                _data[_currentEffectState - i] = _nextData[_currentEffectState - i] << (7 - i);
            }
        }
    }
    else
    {
        NextEffect();
    }
}

void FlyOut()
{
    if (_currentEffectState < 60)
    {
        uint8_t Emax;
        if (_currentEffectState > 7)
            Emax = 8;
        else
            Emax = _currentEffectState;
        for (uint8_t i = 0; i <= Emax; i++)
        {
            if (_currentEffectState - i < 32)
            {
                _data[_currentEffectState - i] = _nextData[_currentEffectState - i] >> i;
            }
        }
    }
    else
    {
        NextEffect();
    }
}

void RandomAppear()
{
    uint8_t Rmask;
    if (_currentEffectState == 0)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            Rmask = random();
            _data[i] = 0;
        }
    }
    else if (_currentEffectState < 6)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            Rmask = random();
            _data[i] = _data[i] | (Rmask & _nextData[i]);
        }
    }
    else if (_currentEffectState == 6)
    {
        for (uint8_t i = 0; i < 32; i++)
            _data[i] = _nextData[i];
    }
    else if (_currentEffectState == 20)
    {
        NextEffect();
    }
}

void RandomDisappear()
{
    uint8_t Rmask, EffectState;

    if (_currentEffectState == 0)
        for (uint8_t i = 0; i < 32; i++)
            _data[i] = _nextData[i];
    else if (_currentEffectState < 6)
    {
        for (uint8_t i = 0; i < 32; i++)
        {
            Rmask = random(256);
            _data[i] = _data[i] & Rmask;
        }
    }
    else if (_currentEffectState == 6)
    {
        for (uint8_t i = 0; i < 32; i++)
            _data[i] = 0;
    }
    else if (_currentEffectState == 10)
    {
        NextEffect();
    }
}

void NextEffect()
{
    _currentEffectState = 0;
    if (_systemEffectMode % 2 == 0 && _systemDataState != NUMBER_OF_STATE)
    {
        _systemEffectMode++;
    }
    else
    {
        _systemEffectMode = random(55);
        _systemEffectMode = _systemEffectMode % 5;
        _systemEffectMode = _systemEffectMode * 2;
        _systemDataState = (_systemDataState + 1) % (NUMBER_OF_STATE + 15);
    }
    _effectChangeRequest = true;
}

void SelectEffectMode()
{
    uint8_t DelayTime = 1;
    _effectChangeRequest = false;
    switch (_systemEffectMode)
    {
    case 0:
        RowTransition();
        break;
    case 1:
        RowOut();
        break;
    case 2:
        NonSynchronizeShift();
        break;
    case 3:
        NonSynchronizeUnshift();
        break;
    case 4:
        ScanTwice();
        break;
    case 5:
        ReScanTwice();
        break;
    case 6:
        FlyIn();
        break;
    case 7:
        FlyOut();
        break;
    case 8:
        RandomAppear();
        break;
    case 9:
        RandomDisappear();
        break;
    case 10:
        Shift_data(10);
        break;
    }
    if (_effectChangeRequest == false)
        _currentEffectState++;
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
            Insert2_data(0, (readTime.hour % 100) / 10);
            Insert2_data(1, readTime.hour % 10);
            if (_systemDataState % 2 == 0)
            {
                Insert2_data(2, 16);
            }
            Insert2_data(3, (readTime.min % 100) / 10);
            Insert2_data(4, readTime.min % 10);
        }
        else if (_systemMode == 1)
        {
            Insert2_data(0, (readTime.hour % 100) / 10);
            Insert2_data(1, readTime.hour % 10);
            Insert2_data(3, 17);
            Insert2_data(4, 18);
        }
        else if (_systemMode == 2)
        {
            Insert2_data(3, (readTime.hour % 100) / 10);
            Insert2_data(4, readTime.hour % 10);
            Insert2_data(0, 19);
            Insert2_data(1, 13);
        }
    }
    else if (_systemDataState == 1)
    {
        digitalWrite(LED1, HIGH);
        _isDotShown = false;
        if (readTime.dow > 1)
        {
            Insert2_data(0, 12, true);
            Insert2_data(1, 13, true);
            Insert2_data(2, 14, true);
            Insert2_data(3, readTime.dow % 10, true);
        }
        else
        {
            Insert2_data(1, 10, true);
            Insert2_data(2, 11, true);
        }
    }
    else if (_systemDataState == 2)
    {
        digitalWrite(LED2, HIGH);
        if (_systemMode == 0)
        {
            Insert2_data(0, (readTime.date % 100) / 10);
            Insert2_data(1, readTime.date % 10);
            Insert2_data(2, 15);
            Insert2_data(3, (readTime.mon % 100) / 10);
            Insert2_data(4, readTime.mon % 10);
        }
        else if (_systemMode == 5)
        {
            Insert2_data(0, (readTime.date % 100) / 10);
            Insert2_data(1, readTime.date % 10);
            Insert2_data(3, 11);
            Insert2_data(4, 20);
        }
        else if (_systemMode == 4)
        {
            Insert2_data(0, 11);
            Insert2_data(1, 20);
            Insert2_data(3, 12);
            Insert2_data(4, 13);
        }
    }
    else if (_systemDataState == 3)
    {
        digitalWrite(LED3, HIGH);
        Date d(readTime.date, readTime.mon, readTime.year % 100);
        Date lunarDate = d.getLunarDateMonth();
        Insert2_data(0, (lunarDate._date % 100) / 10);
        Insert2_data(1, lunarDate._date % 10);
        Insert2_data(2, 15);
        Insert2_data(3, (lunarDate._month % 100) / 10);
        Insert2_data(4, lunarDate._month % 10);
    }
    else if (_systemDataState == 4)
    {
        digitalWrite(LED4, HIGH);
        Insert2_data(0, 2, true);
        Insert2_data(1, 0, true);
        Insert2_data(2, (readTime.year % 100) / 10, true);
        Insert2_data(3, readTime.year % 10, true);
    }
    else if (_systemDataState == 5)
    {
        digitalWrite(LEDW, HIGH);
        float temperature = (5.0 * analogRead(A1) * 100.0 / 1024.0);
        int tpInt = (int)temperature - 5;
        Insert2_data(0, (tpInt % 100) / 10, true);
        Insert2_data(1, tpInt % 10, true);
        Insert2_data(2, 21, true);
        Insert2_data(3, 10, true);
    }
}

void Insert2_data(uint8_t position, uint8_t charactorIndex, bool isFourSlot)
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
        _nextData[charactorPosition + i] = Charactor[charactorIndex][i];
    }
}

void Insert2_data(uint8_t position, uint8_t charactorIndex)
{
    Insert2_data(position, charactorIndex, false);
}

void ClearData()
{
    for (uint8_t i = 0; i < 32; i++)
    {
        _nextData[i] = 0;
    }
    digitalWrite(LEDW, LOW);
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
}

void Shift_data(uint8_t shiftbyte)
{
    for (uint8_t i = 0; i < 31; i++)
    {
        _data[i] = _data[i + 1];
    }
    _data[31] = shiftbyte;
}

uint8_t shiftCharactor(uint8_t CharactorIndex, uint8_t _currentEffectState)
{
    if (Charactor[CharactorIndex][0] == 0)
    {
        Shift_data(Charactor[CharactorIndex][_currentEffectState % 5]);
        if (_currentEffectState == 5)
            return 0;
        return _currentEffectState + 1;
    }
    else
    {
        if (_currentEffectState % 6 == 0)
            Shift_data(0);
        else
            Shift_data(Charactor[CharactorIndex][(_currentEffectState - 1) % 5]);
        if (_currentEffectState == 6)
            return 0;
        return _currentEffectState + 1;
    }
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
        uint8_t mm = bluetooth.read();
        uint8_t dayOfWeek = bluetooth.read();
        uint8_t date = bluetooth.read();
        uint8_t month = bluetooth.read();
        uint8_t year = 2000 + bluetooth.read();
        while (bluetooth.available())
        {
            bluetooth.read();
        }

        rtc.setTime(HH, mm, 0);
        rtc.setDate(date, month, year);
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

        resettime = date;
        EEPROM.update(0, resettime);
        _currentEffectState = 0;
    }
}
