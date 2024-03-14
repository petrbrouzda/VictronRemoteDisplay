#ifndef __FVE_DATA_H___
#define __FVE_DATA_H___

class FveDailySolarData {
    public:
        double solarYield = 0;
        double solarMaxPower = 0;
};

class FveData
{
    public:
        double accuP = 0;
        double accuU = 0;
        double accuI = 0;
        double accuSoc = 0;
        /** Usable battery SoC. Battery is set to cut-off at 20 % SoC, so accuSoc will return 20 %, but accuUsableSoc() will return 0 % */
        double accuUsableSoc();
        double accuCapacity();
        double accuRemainTimeHr();

        double consP = 0;

        double solarU = 0;
        FveDailySolarData day0;
        FveDailySolarData day1;
        FveDailySolarData day2;
        FveDailySolarData day3;
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
};

#endif