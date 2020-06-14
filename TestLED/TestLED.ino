#define LED0 A2
#define LED1 A3
#define LED2 A4
#define LED3 A5
#define LED4 5

using namespace Espace;
void setup()
{
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
}

void loop()
{
    digitalWrite(LED0, HIGH);
    delay(500);
    digitalWrite(LED1, HIGH);
    delay(500);
    digitalWrite(LED2, HIGH);
    delay(500);
    digitalWrite(LED3, HIGH);
    delay(500);
    digitalWrite(LED4, HIGH);
    delay(500);
}
