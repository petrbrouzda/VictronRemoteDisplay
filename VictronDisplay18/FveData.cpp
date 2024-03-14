#include "FveData.h"
#include <Arduino.h>
#include "toolkit.h"
#include "config.h"

void FveData::update() {
    this->updateTime = millis();
}

const char * FveData::toString() {
    sprintf( this->buffer, "accu: %.1f %% (%.1f %%), %.1f W, %.1f V, %.1f A; consumer: %.1f W; solar: %.1f W, %.0f V; LED: abs:%d, blk:%d, flt:%d, inv:%d, LOW:%d, OVL:%d, TMP:%d; daily: %.01f (%.0f), %.1f (%.0f), %.1f (%.0f), %.1f (%.0f) kWh (max W)",
        this->accuUsableSoc(), this->accuSoc, this->accuP, this->accuU, this->accuI,
        this->consP,
        this->getSolarP(), this->solarU,
        this->ledAbsorption, this->ledBulk, this->ledFloat, this->ledInverter, this->ledLowBattery, this->ledOverload, this->ledTemperature,
        this->day0.solarYield, this->day0.solarMaxPower,
        this->day1.solarYield, this->day1.solarMaxPower,
        this->day2.solarYield, this->day2.solarMaxPower,
        this->day3.solarYield, this->day3.solarMaxPower
    );
    return this->buffer;
}


double FveData::getSolarP()  {
    double d = this->consP + this->accuP + INVERTER_OWN_CONSUMPTION;
    if( d<20.0 ) return 0;
    return d;
}




double FveData::accuCapacity() {
  return BATTERY_CAPACITY_KWH * 0.8 * this->accuUsableSoc() / 100.0;
}

double FveData::accuRemainTimeHr() {
  if( this->accuP < 0 ) {
    return - (this->accuCapacity() * 1000.0 / this->accuP);
  } else {
    return 0;
  }
}

double FveData::accuUsableSoc() {
    return map_double( this->accuSoc, 20.0, 100.0, 0.0, 100.0 );
}
