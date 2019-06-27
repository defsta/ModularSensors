/*
 *TallyCounterI2C.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This file is for an external Event counter, used to measure windspeed or rainfall
 *
 *Documentation for the sensor can be found at:
 * https://github.com/NorthernWidget-Skunkworks/Project-Tally​
 * https://github.com/NorthernWidget-Skunkworks/Tally_Library/tree/Dev_I2C
 *
 * For Wind Speed:
 *  Accuracy and resolution are dependent on the sensor used:
 *  Inspeed WS2R Version II Reed Switch Anemometer
 *    https://www.store.inspeed.com/Inspeed-Version-II-Reed-Switch-Anemometer-Sensor-Only-WS2R.htm
 *
 * Assume sensor is immediately stable
*/

#include "TallyCounterI2C.h"


// The constructor - because this is I2C, only need the power pin and rain per event if a non-standard value is used
TallyCounterI2C::TallyCounterI2C(uint8_t i2cAddressHex)
     : Sensor("TallyCounterI2C", TALLY_NUM_VARIABLES,
              TALLY_WARM_UP_TIME_MS, TALLY_STABILIZATION_TIME_MS, TALLY_MEASUREMENT_TIME_MS,
              -1, -1, 1)
{
    _i2cAddressHex  = i2cAddressHex;
}
// Destructor
TallyCounterI2C::~TallyCounterI2C(){};


String TallyCounterI2C::getSensorLocation(void)
{
    String address = F("I2C_0x");
    address += String(_i2cAddressHex, HEX);
    return address;
}


bool TallyCounterI2C::setup(void)
{
    counter_internal.begin();
    return Sensor::setup();  // this will set pin modes and the setup status bit
}


bool TallyCounterI2C::addSingleMeasurementResult(void)
{
    bool success = false;

    //intialize values
    int16_t events = -9999;  // Number of events

    // Get data from external event counter
    // // if the 'requestFrom' returns 0, it means no bytes were received
    // if (Wire.requestFrom(int(_i2cAddressHex), 2))
    // {
        MS_DBG(getSensorNameAndLocation(), F("is reporting:"));

        events = counter_internal.Peek(); //Read data from counter without clearing
        counter_internal.Clear(); //Clear count value

        if (events < 0) events = -9999;  // If negetive value results, return failure

        MS_DBG(F("  Events:"), events);
    // }
    // else MS_DBG(F("No bytes received from"), getSensorNameAndLocation());

    verifyAndAddMeasurementResult(TALLY_EVENTS_VAR_NUM, events);

    // Unset the time stamp for the beginning of this measurement
    _millisMeasurementRequested = 0;
    // Unset the status bits for a measurement request (bits 5 & 6)
    _sensorStatus &= 0b10011111;

    // Return true when finished
    return true;
}