#include <Arduino.h>

volatile bool edge = false;

volatile unsigned long last_micros = 0;
volatile unsigned long microdiff   = 0;

bool last_bit        = 1;
bool half_short      = true;
uint8_t decoded_bits = 0;
uint32_t message     = 0;

enum class states
{
    NONE,
    SYNC1,
    SYNC2,
    MSG
} state;

void my_isr()
{
    unsigned long m = micros();
    microdiff       = m - last_micros;
    last_micros     = m;
    edge            = true;
}

bool is_short_pulse(unsigned long d)
{
    return (d > 1100 and d < 1800);
}

bool is_long_pulse(unsigned long d)
{
    return (d > 2600 and d < 3300);
}

void emit_bit(uint8_t b, uint8_t bit_pos)
{
    bitWrite(message, bit_pos, b);
}

void setup()
{
    pinMode(2, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), my_isr, CHANGE);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(19200);
    Serial.println("OSv1_receiver");
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
                    state = states::MSG;
                    digitalWrite(LED_BUILTIN, LOW);
                }
                else
                {
                    state = states::NONE;
                }
                break;
            }
            case states::MSG:
            {
                if (is_long_pulse(microdiff))
                {
                    // LONG: flip bit
                    last_bit = !last_bit;
                    emit_bit(last_bit, decoded_bits);
                    decoded_bits++;
                    half_short = false;
                }
                else if (is_short_pulse(microdiff))
                {
                    // SHORT: keep bit
                    if (half_short)
                    {
                        emit_bit(last_bit, decoded_bits);
                        decoded_bits++;
                        half_short = false;
                    }
                    else
                    {
                        half_short = true;
                    }
                }
                else
                {
                    // error
                    Serial.println("Error after bit " + String(decoded_bits) + " - pulse length was (us) " + String(microdiff));
                    state        = states::NONE;
                    decoded_bits = 0;
                    message      = 0;
                }
                if (decoded_bits >= 32)
                {
                    // full message received
                    state        = states::NONE;
                    decoded_bits = 0;
                    Serial.println(message, HEX);
                    message    = 0;
                    last_bit   = 1;
                    half_short = true;
                }
                break;
            }
            default:
                break;
        }
        edge = false;
    }
}
