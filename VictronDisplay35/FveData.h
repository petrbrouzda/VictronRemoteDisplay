#ifndef __FVE_DATA_H___
#define __FVE_DATA_H___

#include "src/extgfx/ChartDataSource.h"

class FveData
{
    public:
        FveData();

        ArrayCDS* dailyYield;
        float monthYield();

        /** max consumption [W], one value per minute */
        RingBufCDS *minuteCons;

        /** solar power [W], one value per minute */
        RingBufCDS *minuteProd;

        int temperature;
        int temperatureEnv;

        double accuP = 0;
        double accuU = 0;
        double accuI = 0;
        double accuSoc = 0;
        /** Usable battery SoC. Battery is set to cut-off at 20 % SoC, so accuSoc will return 20 %, but accuUsableSoc() will return 0 % */
        double accuUsableSoc();
        double accuCapacity();
        double accuRemainTimeHr();

        void setConsP( double consP );
        double getConsP();

        double minuteMaxConsP;
        void clearMinuteMaxConsP();

        double solarU = 0;
        double solarP = 0;

        double getMaxSolarP();

        /** today max solar power from history */
        double day0solarMaxPower = 0;
        double getSolarP();

        int ledAbsorption = 0;
        int ledBulk = 0;
        int ledFloat = 0;
        int ledInverter = 0;
        int ledLowBattery = 0;
        int ledOverload = 0;
        int ledTemperature = 0;

        void update();

        /* statistics */
        long updateTime = 0;
        long msgs = 0;
        long msgsStatusTime = 0;
        long prevThoughput = -1;

        const char * toString();

    private:
        char buffer[512];
        double _consP = 0;
};

#endif