//Temp sensor driver for DS18B20 (Particle platform, DallasTemperature library)

#include "DS18B20.h"
#include "DallasTemperature.h"

static OneWire  oneWire(DS18B20_PIN);
static DallasTemperature sensors(&oneWire);

bool DS18B20_Init(void) {
    sensors.begin();
    return sensors.getDeviceCount() > 0;
}

bool DS18B20_Present(void) {
    return sensors.getDeviceCount() > 0;
}

float DS18B20_ReadTempC(void) {
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

bool DS18B20_ReadRaw(int16_t *raw) {
    DeviceAddress addr;
    if (!sensors.getAddress(addr, 0)) return false;
    sensors.requestTemperatures();
    int32_t val = sensors.getTemp(addr);
    if (val == DEVICE_DISCONNECTED_RAW   ||
        val == DEVICE_FAULT_OPEN_RAW     ||
        val == DEVICE_FAULT_SHORTGND_RAW ||
        val == DEVICE_FAULT_SHORTVDD_RAW) return false;
    *raw = (int16_t)val;
    return true;
}