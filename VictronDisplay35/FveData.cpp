#include "FveData.h"
#include "src/toolkit/map_double.h"
#include "config.h"

void FveData::update() {
    this->updateTime = millis();
}

const char * FveData::toString() {
    sprintf( this->buffer, "accu: %.1f %% (%.1f %%), %.1f W, %.1f V, %.1f A; consumer: %.1f W; solar: %.1f W, %.0f V; LED: abs:%d, blk:%d, flt:%d, inv:%d, LOW:%d, OVL:%d, TMP:%d; daily: %.01f (%.0f), %.1f (%.0f), %.1f, %.1f kWh (max W)",
        this->accuUsableSoc(), this->accuSoc, this->accuP, this->accuU, this->accuI,
        this->_consP,
        this->getSolarP(), this->solarU,
        this->ledAbsorption, this->ledBulk, this->ledFloat, this->ledInverter, this->ledLowBattery, this->ledOverload, this->ledTemperature,
        this->dailyYield->getItem(0) , this->day0solarMaxPower,
        this->dailyYield->getItem(1),
        this->dailyYield->getItem(2),
        this->dailyYield->getItem(3)
    );
    return this->buffer;
}


double FveData::getSolarP()  {
    return this->solarP;
}

/**
 * Returns "today max solar power"
 * Statistics (and therefore day0solarMaxPower) are updated after about one minute.
 * So "current power" can be higher than "today's max power". To avoid confusion, this function fix it.
 */
double FveData::getMaxSolarP() {
  if( this->getSolarP() > this->day0solarMaxPower ) {
    return this->getSolarP();
  } else {
    return this->day0solarMaxPower;
  }
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

void FveData::setConsP(double consP)
{
  this->_consP = consP;
  if( consP > this->minuteMaxConsP ) {
    this->minuteMaxConsP = consP;
  }
}

double FveData::getConsP()
{
    return this->_consP;
}

void FveData::clearMinuteMaxConsP()
{
  this->minuteMaxConsP = _consP;
}


// must be the same as CHART_WIDTH in display.ino
#define MAX_ITEMS_CHART 229

FveData::FveData()
{
  this->dailyYield = new ArrayCDS(32);
  this->minuteCons = new RingBufCDS(MAX_ITEMS_CHART);
  this->minuteProd = new RingBufCDS(MAX_ITEMS_CHART);
  this->minuteMaxConsP = 0;
}

float FveData::monthYield()
{
  float sum = 0;
  for( int i=0; i<dailyYield->getNumItems(); i++ ) {
    float v = dailyYield->getItem(i);
    if( !isnan(v) ) {
      sum += v;
    }
  }
  return sum;
}

double FveData::accuUsableSoc()
{
    return map_double( this->accuSoc, 20.0, 100.0, 0.0, 100.0 );
}
