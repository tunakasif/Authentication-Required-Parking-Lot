#include "mbed.h"
#include "TextLCD.h"
#include "HCSR04.h"

//KL25Z Pins for MFRC522 SPI interface
#define TRIG PTA12
#define ECHO PTD4

// Constraints for the system
Timer timer;
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);

int main()
{
    HCSR04 sensor(ECHO, TRIG);
    sensor.setRanges(2, 30);
    lcd.cls();
    lcd.printf("Min=%g Max=%g", sensor.getMinRange(), sensor.getMaxRange());

    while (true)
    {
        timer.reset();
        timer.start();
        sensor.startMeasurement();
        while (!sensor.isNewDataReady())
        {
            // wait for new data
            // waiting time depends on the distance
        }
        lcd.locate(0, 1);
        lcd.printf("Dist: %5.1f mm", sensor.getDistance_mm());
        timer.stop();
        wait_ms(500 - timer.read_ms()); // time the loop
    }
}
