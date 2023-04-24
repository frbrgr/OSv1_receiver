#pragma once

#include <Arduino.h>
#include <stdint.h>

class Decoder
{
    bool _valid      = false;
    int _temperature = 0;

public:
    void set_data(uint32_t);
    bool is_valid() const { return _valid; };
    double get_temperature() const { return _temperature / 10.0; };
    String get_temp_string() const;
};
