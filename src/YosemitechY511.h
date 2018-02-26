/*
 *YosemitechY511.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This file is for the Yosemitech Y511 Turbidity Sensor with Wiper
 *It is dependent on the YosemitechParent super class.
 *
 *Documentation for the Modbus Protocol commands and responses can be found
 *within the documentation in the YosemitechModbus library at:
 *https://github.com/EnviroDIY/YosemitechModbus
 *
 *These devices output very high "resolution" (32bits) so the resolutions are
 *based on their accuracy, not the resolution of the sensor
 *
 * For Turbidity:
 *     Accuracy is ＜5% or 0.3NTU
 *     Range is 0.1~1000 NTU
 *
 * For Temperature:
 *     Accuracy is ± 0.2°C
 *     Range is 0°C to + 50°C
 *
 * Time before sensor responds after power - 500ms
 * Time between "StartMeasurement" command and stable reading - 22sec
*/

#ifndef YosemitechY511_h
#define YosemitechY511_h

#include "YosemitechParent.h"
#include "VariableBase.h"

#define Y511_NUM_VARIABLES 2
#define Y511_WARM_UP_TIME_MS 500
#define Y511_STABILIZATION_TIME_MS 22000
#define Y511_MEASUREMENT_TIME_MS 1700

#define Y511_TURB_RESOLUTION 2
#define Y511_TURB_VAR_NUM 0

#define Y511_TEMP_RESOLUTION 2
#define Y511_TEMP_VAR_NUM 1

// The main class for the Decagon Y511
class YosemitechY511 : public YosemitechParent
{
public:
    // Constructors with overloads
    YosemitechY511(byte modbusAddress, Stream* stream, int8_t powerPin,
                   int8_t enablePin = -1, uint8_t measurementsToAverage = 1)
     : YosemitechParent(modbusAddress, stream, powerPin, enablePin, measurementsToAverage,
                        Y511, F("YosemitechY511"), Y511_NUM_VARIABLES,
                        Y511_WARM_UP_TIME_MS, Y511_STABILIZATION_TIME_MS, Y511_MEASUREMENT_TIME_MS)
    {}
    YosemitechY511(byte modbusAddress, Stream& stream, int8_t powerPin,
                   int8_t enablePin = -1, uint8_t measurementsToAverage = 1)
     : YosemitechParent(modbusAddress, stream,powerPin,  enablePin, measurementsToAverage,
                        Y511, F("YosemitechY511"), Y511_NUM_VARIABLES,
                        Y511_WARM_UP_TIME_MS, Y511_STABILIZATION_TIME_MS, Y511_MEASUREMENT_TIME_MS)
    {}
};


// Defines the Turbidity
class YosemitechY511_Turbidity : public Variable
{
public:
    YosemitechY511_Turbidity(Sensor *parentSense, String UUID = "", String customVarCode = "")
     : Variable(parentSense, Y511_TURB_VAR_NUM,
                F("turbidity"), F("nephelometricTurbidityUnit"),
                Y511_TURB_RESOLUTION,
                F("Y511Turbidity"), UUID, customVarCode)
    {}
};


// Defines the Temperature Variable
class YosemitechY511_Temp : public Variable
{
public:
    YosemitechY511_Temp(Sensor *parentSense, String UUID = "", String customVarCode = "")
     : Variable(parentSense, Y511_TEMP_VAR_NUM,
                F("temperature"), F("degreeCelsius"),
                Y511_TEMP_RESOLUTION,
                F("Y511temp"), UUID, customVarCode)
    {}
};

#endif