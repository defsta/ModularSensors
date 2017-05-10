/*
 *ModemSupport.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This file is for turning modems on and off to save power.  It is more-or-less
 *a wrapper for tinyGSM library:  https://github.com/vshymanskyy/TinyGSM
*/

#ifndef modem_onoff_h
#define modem_onoff_h

#include <Arduino.h>
#include "LoggerBase.h"

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900) || \
    defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7) || \
    defined(TINY_GSM_MODEM_M590) || defined(TINY_GSM_MODEM_ESP8266) || \
    defined(TINY_GSM_MODEM_XBEE)
  #define USE_TINY_GSM
  // #define TINY_GSM_DEBUG Serial
  #define TINY_GSM_YIELD() { delay(3);}
  #include <TinyGsmClient.h>
#else
  #define DBG(...)
#endif

// For the various communication devices"
typedef enum DTRSleepType
{
  held = 0,  // Turns the modem on by setting the onoff/DTR/Key high and off by setting it low
  pulsed,  // Turns the modem on and off by pulsing the onoff/DTR/Key pin on for 2 seconds
  reverse,  // Turns the modem on by setting the onoff/DTR/Key LOW and off by setting it HIGH
  always_on
} DTRSleepType;


/* ===========================================================================
* Classes for turning modems on and off
* IDEA FOR THIS TAKEN FROM SODAQ'S MODEM LIBRARIES
* ========================================================================= */


/* ===========================================================================
* Functions for the OnOff class
* ========================================================================= */

class ModemOnOff
{
public:
    // Constructor
    ModemOnOff()
    {
        _vcc33Pin = -1;
        _onoff_DTR_pin = -1;
        _status_CTS_pin = -1;
    }

    // Initializes the instance
    virtual void init(int vcc33Pin, int onoff_DTR_pin, int status_CTS_pin)
    {
        DBG(F("Initializing modem on/off..."));
        if (vcc33Pin >= 0) {
          _vcc33Pin = vcc33Pin;
          // First write the output value, and only then set the output mode.
          digitalWrite(_vcc33Pin, LOW);
          pinMode(_vcc33Pin, OUTPUT);
        }
        if (onoff_DTR_pin >= 0) {
            _onoff_DTR_pin = onoff_DTR_pin;
            // First write the output value, and only then set the output mode.
            digitalWrite(_onoff_DTR_pin, LOW);
            pinMode(_onoff_DTR_pin, OUTPUT);
        }
        if (status_CTS_pin >= 0) {
            _status_CTS_pin = status_CTS_pin;
            pinMode(_status_CTS_pin, INPUT);
        }
        DBG(F("   ... Success!\n"));
    }

    virtual bool isOn(void)
    {
        if (_status_CTS_pin >= 0) {
            bool status = digitalRead(_status_CTS_pin);
            // DBG(F("Is modem on? "), status, F("\n"));
            return status;
        }
        // No status pin. Let's assume it is on.
        return true;
    }

    virtual bool on(void) = 0;

    virtual bool off(void) = 0;

protected:
    int8_t _vcc33Pin;
    int8_t _onoff_DTR_pin;
    int8_t _status_CTS_pin;

    void powerOn(void)
    {
        if (_vcc33Pin >= 0) {
            digitalWrite(_vcc33Pin, HIGH);
            DBG(F("Sending power to modem.\n"));
        }
    }

    void powerOff(void)
    {
        if (_vcc33Pin >= 0) {
            digitalWrite(_vcc33Pin, LOW);
            DBG(F("Cutting modem power.\n"));
        }
    }

};



/* ===========================================================================
* Functions for pulsed method.
* This turns the modem on and off by turning the onoff/DTR/Key pin on for two
* seconds and then back off.
* This is used by the Sodaq GPRSBee v0.4 and the Adafruit Fona.
* ========================================================================= */

// Turns the modem on and off by pulsing the onoff/DTR/Key pin on for 2 seconds
class pulsedOnOff : public ModemOnOff
{
public:
    bool on(void) override
    {
        powerOn();
        DBG(F("Pulsing modem to on with pin "));
        DBG(_onoff_DTR_pin, F("\n"));
        if (!isOn()) {pulse();}
        // Wait until is actually on
        for (unsigned long start = millis(); millis() - start < 5000; )
        {
            if (isOn())
            {
                DBG(F("Modem now on.\n"));
                return true;
            }
          delay(5);
        }
        DBG(F("Failed to turn modem on.\n"));
        return false;
    }

    bool off(void) override
    {
        if (isOn()) {pulse();}
        else DBG(F("Modem was not ever on.\n"));
        // Wait until is off
        for (unsigned long start = millis(); millis() - start < 5000; )
        {
            if (!isOn())
            {
                DBG(F("Modem now off.\n"));
                powerOff();
                return true;
            }
            delay(5);
        }
        DBG(F("Failed to turn modem off.\n"));
        powerOff();
        return false;
    }

private:
    void pulse(void)
    {
        if (_onoff_DTR_pin >= 0)
        {
            digitalWrite(_onoff_DTR_pin, LOW);
            delay(200);
            digitalWrite(_onoff_DTR_pin, HIGH);
            delay(2500);
            digitalWrite(_onoff_DTR_pin, LOW);
        }
    }
};


/* ===========================================================================
* Functions for held method.
* This turns the modem on by setting the onoff/DTR/Key pin high and off by
* setting it low.
* This is used by the Sodaq GPRSBee v0.6.
* ========================================================================= */

// Turns the modem on by setting the onoff/DTR/Key high and off by setting it low
class heldOnOff : public ModemOnOff
{
public:
    bool on(void) override
    {
        powerOn();
        if (_onoff_DTR_pin <= 0) {return true;}
        else
        {
            DBG(F("Turning modem on by setting pin "));
            DBG(_onoff_DTR_pin);
            DBG(F(" high\n"));
            digitalWrite(_onoff_DTR_pin, HIGH);
            // Wait until is actually on
            for (unsigned long start = millis(); millis() - start < 5000; )
            {
                if (isOn())
                {
                    DBG(F("Modem now on.\n"));
                    return true;
                }
                delay(5);
            }
            DBG(F("Failed to turn modem on.\n"));
            return false;
        }
    }

    bool off(void) override
    {
        if (_onoff_DTR_pin <= 0) {return true;}
        else
        {
            if (!isOn()) DBG(F("Modem was not ever on.\n"));
            digitalWrite(_onoff_DTR_pin, LOW);
            // Wait until is off
            for (unsigned long start = millis(); millis() - start < 5000; )
            {
                if (!isOn())
                {
                    DBG(F("Modem now off.\n"));
                    powerOff();
                    return true;
                }
                delay(5);
            }
            DBG(F("Failed to turn modem off.\n"));
            powerOff();
            return false;
        }
    }
};


/* ===========================================================================
* Functions for reverse method.
* This turns the modem on by setting the onoff/DTR/Key pin LOW and off by
* setting it HIGH.
* This is used by the XBee's
* ========================================================================= */

// Turns the modem on by setting the onoff/DTR/Key LOW and off by setting it HIGH
class reverseOnOff : public ModemOnOff
{
public:
    bool isOn(void) override
    {
        if (_status_CTS_pin >= 0) {
            bool status = digitalRead(_status_CTS_pin);
            // DBG(F("Is modem on? "), status, F("\n"));
            return !status;
        }
        // No status pin. Let's assume it is on.
        return true;
    }

    bool on(void) override
    {
        powerOn();
        DBG(F("Turning modem on on by setting pin "));
        DBG(_onoff_DTR_pin);
        DBG(F(" low\n"));
        if (_onoff_DTR_pin >= 0) {
            digitalWrite(_onoff_DTR_pin, LOW);
        }
        // Wait until is actually on
        for (unsigned long start = millis(); millis() - start < 5000; )
        {
            if (isOn())
            {
                DBG(F("Modem now on.\n"));
                return true;
            }
            delay(5);
        }
        DBG(F("Failed to turn modem on.\n"));
        return false;
    }

    bool off(void) override
    {
        if (!isOn()) DBG(F("Modem was not ever on.\n"));
        if (_onoff_DTR_pin >= 0) {
            digitalWrite(_onoff_DTR_pin, HIGH);
        }
        // Wait until is off
        for (unsigned long start = millis(); millis() - start < 5000; )
        {
            if (!isOn())
            {
                DBG(F("Modem now off.\n"));
                powerOff();
                return true;
            }
            delay(5);
        }
        DBG(F("Failed to turn modem off.\n"));
        powerOff();
        return false;
    }
};


/* ===========================================================================
* Functions for the modem class
* This is basically a wrapper for TinyGsm
* ========================================================================= */

class loggerModem
{
public:
    void setupModem(Stream *modemStream,
                    int vcc33Pin,
                    int status_CTS_pin,
                    int onoff_DTR_pin,
                    DTRSleepType sleepType,
                    const char *APN)
    {
    _APN = APN;
    init(modemStream, vcc33Pin, status_CTS_pin, onoff_DTR_pin, sleepType);
    }

    void setupModem(Stream *modemStream,
                    int vcc33Pin,
                    int status_CTS_pin,
                    int onoff_DTR_pin,
                    DTRSleepType sleepType,
                    const char *ssid,
                    const char *pwd)
    {
    _ssid = ssid;
    _pwd = pwd;
    init(modemStream, vcc33Pin, status_CTS_pin, onoff_DTR_pin, sleepType);
    }

    bool connectNetwork(void)
    {
        bool retVal = false;

        // Check if the modem is on; turn it on if not
        if(!modemOnOff->isOn()) modemOnOff->on();
        // Check again if the modem is on.  If it still isn't on, give up
        if(!modemOnOff->isOn()) return false;

        #if defined(TINY_GSM_MODEM_XBEE) || defined(TINY_GSM_MODEM_ESP8266)
        if (_ssid)
        {
            DBG(F("\nConnecting to WiFi network...\n"));
            if (!_modem->waitForNetwork(10000L)){
                DBG("... Connection failed.  Resending credentials...", F("\n"));
                _modem->networkConnect(_ssid, _pwd);
                if (!_modem->waitForNetwork(45000L)){
                    DBG("... Connection failed", F("\n"));
                } else {
                    retVal = true;
                    DBG("... Success!", F("\n"));
                }
            } else {
                DBG("... Success!", F("\n"));
                retVal = true;
            }
        }
        else
        {
        #endif
        #if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900) || \
            defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7) || \
            defined(TINY_GSM_MODEM_M590) || defined(TINY_GSM_MODEM_XBEE)
            DBG(F("\nWaiting for cellular network...\n"));
            if (!_modem->waitForNetwork(55000L)){
                DBG("... Connection failed.", F("\n"));
            } else {
                _modem->gprsConnect(_APN, "", "");
                DBG("... Success!", F("\n"));
                retVal = true;
            }
        #endif
        #if defined(TINY_GSM_MODEM_XBEE) || defined(TINY_GSM_MODEM_ESP8266)
        }
        #endif

        return retVal;
    }

    void disconnectNetwork(void)
    {
    #if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900) || \
        defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7) || \
        defined(TINY_GSM_MODEM_M590) || defined(TINY_GSM_MODEM_XBEE)
        _modem->gprsDisconnect();
    #endif
    }

    int connect(const char *host, uint16_t port)
    {
    #if defined(USE_TINY_GSM)
        return  _client->connect(host, port);
    #else
        return 0;
    #endif
    }

    void stop(void)
    {
    #if defined(USE_TINY_GSM)
        return _client->stop();
    #endif
    }

    // Used to empty out the buffer after a post request.
    // Removing this may cause communication issues. If you
    // prefer to not see the std::out, remove the print statement
    void dumpBuffer(Stream *stream, int timeDelay = 5, int timeout = 5000)
    {
        delay(timeDelay);
        while (timeout-- > 0 && stream->available() > 0)
        {
            #if defined(TINY_GSM_DEBUG)
            DBG((char)stream->read());
            #else
            stream->read();
            #endif
            delay(timeDelay);
        }
        DBG(F("\n"));
    }

    // Get the time from NIST via TIME protocol (rfc868)
    // This would be much more efficient if done over UDP, but I'm doing it
    // over TCP because I don't have a UDP library for all the modems.
    uint32_t getNISTTime(void)
    {

        // Make TCP connection
        #if defined(TINY_GSM_MODEM_XBEE)
        connect("time-c.nist.gov", 37);  // XBee cannot resolve time.nist.gov
        #else
        connect("time.nist.gov", 37);
        #endif

        // XBee needs to send something before the connection is actually made
        #if defined(TINY_GSM_MODEM_XBEE)
        stream->write("Hi!");
        delay(75);  // Need this delay!  Can get away with 50, but 100 is safer.
        #endif

        // Response is returned as 32-bit number as soon as connection is made
        // Connection is then immediately closed, so there is no need to close it
        uint32_t secFrom1900 = 0;
        byte response[4] = {0};
        for (uint8_t i = 0; i < 4; i++)
        {
            response[i] = stream->read();
            secFrom1900 += 0x000000FF & response[i];
            // DBG("\n*****",String(secFrom1900, BIN),"*****");
            if (i+1 < 4) {secFrom1900 = secFrom1900 << 8;}
        }
        dumpBuffer(stream);
        // DBG("\n*****",secFrom1900,"*****");

        // Return the timestamp
        uint32_t unixTimeStamp = secFrom1900 - 2208988800;
        DBG(F("Timesamp returned by NIST (UTC): "), unixTimeStamp, F("\n"));
        // If before Jan 1, 2017 or after Jan 1, 2030, most likely an error
        if (unixTimeStamp < 1483228800 || unixTimeStamp > 1893456000) return 0;
        else return unixTimeStamp;
    }

    bool syncDS3231(void)
    {
        uint32_t start_millis = millis();

        // Get the time stamp from NIST and adjust it to the correct time zone
        // for the logger.
        uint32_t nist = getNISTTime();
        uint32_t nist_logTZ = nist + Logger::getTimeZone()*3600;
        uint32_t nist_rtcTZ = nist_logTZ - Logger::getTZOffset()*3600;
        DBG(F("        Correct Time for Logger: "), nist_logTZ, F(" -> "), \
            Logger::formatDateTime_ISO8601(nist_logTZ), F("\n"));

        // See how long it took to get the time from NIST
        int sync_time = (millis() - start_millis)/1000;

        // Check the current RTC time
        uint32_t cur_logTZ = Logger::getNow();
        DBG(F("           Time Returned by RTC: "), cur_logTZ, F(" -> "), \
            Logger::formatDateTime_ISO8601(cur_logTZ), F("\n"));
        // DBG(F("Offset: "), abs(nist_logTZ - cur_logTZ), F("\n"));

        // If the RTC and NIST disagree by more than 5 seconds, set the clock
        if ((abs(nist_logTZ - cur_logTZ) > 5) && (nist != 0))
        {
            rtc.setEpoch(nist_rtcTZ + sync_time/2);
            PRINTOUT(F("Clock synced to NIST!\n"));
            return true;
        }
        else
        {
            PRINTOUT(F("Clock already within 5 seconds of NIST.\n"));
            return false;
        }
    }

    Stream *stream;
    ModemOnOff *modemOnOff;

private:
    void init(Stream *modemStream, int vcc33Pin, int status_CTS_pin, int onoff_DTR_pin,
              DTRSleepType sleepType)
    {
        // Set up the method for putting the modem to sleep
        switch(sleepType)
        {
            case pulsed:
            {
                static pulsedOnOff pulsed;
                modemOnOff = &pulsed;
                pulsed.init(vcc33Pin, onoff_DTR_pin, status_CTS_pin);
                break;
            }
            case held:
            {
                static heldOnOff held;
                modemOnOff = &held;
                held.init(vcc33Pin, onoff_DTR_pin, status_CTS_pin);
                break;
            }
            case reverse:
            {
                static reverseOnOff reverse;
                modemOnOff = &reverse;
                reverse.init(vcc33Pin, onoff_DTR_pin, status_CTS_pin);
                break;
            }
            default:
            {
                static heldOnOff held;
                modemOnOff = &held;
                held.init(-1, -1, -1);
                break;
            }
        }

        #if defined(USE_TINY_GSM)

            // Initialize the modem
            DBG(F("Initializing GSM modem instance..."));
            static TinyGsm modem(*modemStream);
            _modem = &modem;
            static TinyGsmClient client(modem);
            _client = &client;

            // Check if the modem is on; turn it on if not
            if(!modemOnOff->isOn()) modemOnOff->on();
            // Check again if the modem is on.  Only "begin" if it responded.
            if(modemOnOff->isOn())
            {
                _modem->begin();
                #if defined(TINY_GSM_MODEM_XBEE)
                    _modem->setupPinSleep();
                #endif
                modemOnOff->off();
            }
            stream = _client;
            DBG(F("   ... Complete!\n"));

        #else
            stream = modemStream;
        #endif
    }

#if defined(USE_TINY_GSM)
    TinyGsm *_modem;
    TinyGsmClient *_client;
#endif

    const char *_APN;
    const char *_ssid;
    const char *_pwd;
};

#endif /* modem_onoff_h */