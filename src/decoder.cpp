#include "decoder.hpp"

/*
 * Message Format:
 * +------------------------------------+
 * |MSB           32 bits            LSB|
 * +--------+-----------+-------+-------+
 * |    2   |     4     |   1   |   1   |Nibble
 * +--------+-----------+-------+-------+
 * |Checksum|Temperature|Channel|Rolling|
 * |        |           |       |Code   |
 * +--------+-----------+-------+-------+
 */

struct raw_message_data
{
    uint8_t checksum;
    uint16_t temp;
    uint8_t channel_code;
};

union message
{
    uint32_t bytes;
    raw_message_data data;
};

/** Validate the message checksum. We add up the three data bytes and compare them to the original checksum byte. */
static bool validate_message(const uint32_t& msg)
{
    auto b   = reinterpret_cast<const byte*>(&msg);
    byte sum = 0;
    sum += b[0];
    sum += b[1];
    sum += b[2];
    return sum == b[3];
}

/** Convert temperature from BCD to an integer representing tenths of degrees.
 * For example, 20.5°C are represented as the value 205
 * The input includes a status nibble and three nibbles for temperature
 */
static int decode_temperature(uint16_t temp)
{
    const byte high = highByte(temp); // status nibble and first temperature nibble
    const byte low  = lowByte(temp);  // two temperature nibbles
    int temperature = (high & 0x0F) * 100;
    temperature += ((low & 0xF0) >> 4) * 10;
    temperature += (low & 0x0F);

    if ((high >> 5) & 0x1) // the second bit in the status nibble indicates negative temperature
    {
        temperature *= -1;
    }
    return temperature;
}

void Decoder::set_data(uint32_t d)
{
    if (!validate_message(d))
    {
        _valid = false;
        return;
    }
    message m;
    m.bytes      = d;
    _temperature = decode_temperature(m.data.temp);
    _valid       = true;
}

String Decoder::get_temp_string() const
{
    return String(get_temperature(), 1) + "°C";
}
