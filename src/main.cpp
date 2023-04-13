#include <Arduino.h>

volatile bool edge                 = false;
volatile unsigned long last_micros = 0;
volatile unsigned long microdiff   = 0;

enum class states
{
    NONE,
    SYNC1,
    SYNC2
} state;

void my_isr()
{
    unsigned long m = micros();
    microdiff       = m - last_micros;
    last_micros     = m;
    edge            = true;
}

void setup()
{
    pinMode(2, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(2), my_isr, CHANGE);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
    if (edge)
    {
        switch (state)
        {
            case states::NONE:
            {
                if (microdiff > 4200 and microdiff < 4300)
                {
                    state = states::SYNC1;
                    digitalWrite(LED_BUILTIN, LOW);
                }
                break;
            }
            case states::SYNC1:
            {
                if (microdiff > 5700 and microdiff < 5800)
                {
                    state = states::SYNC2;
                    digitalWrite(LED_BUILTIN, HIGH);
                }
                else
                {
                    state = states::NONE;
                }
                break;
            }
            case states::SYNC2:
            {
                if (microdiff > 5200 and microdiff < 5300)
                {
                    // full sync
                    digitalWrite(LED_BUILTIN, LOW);
                }
                else
                {
                    state = states::NONE;
                }
            }
            default:
                break;
        }
        edge = false;
    }
}
