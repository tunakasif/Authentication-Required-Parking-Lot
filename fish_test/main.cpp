#include "mbed.h"
#include "TextLCD.h"

TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30,
            TextLCD::LCD16x2);
DigitalIn fish(PTA5);
InterruptIn fish_sensor(PTA5);
DigitalIn fish_pin(PTA5);

// global constants
const int NUMBER_OF_PARK_SPOTS = 5;

// global variables
int avaliableSpots;

void fish_ISR()
{
    // since the flying fish can bounce this wait and
    // pin check makes sure that it is a rising edge
    wait(0.1);
    if (fish_pin.read() == 1)
    {
        if (avaliableSpots < NUMBER_OF_PARK_SPOTS)
        {
            avaliableSpots++;
        }
    }
}

int main()
{
    // program code
    // Initialize
    avaliableSpots = NUMBER_OF_PARK_SPOTS;
    avaliableSpots = 3;
    fish_sensor.rise(&fish_ISR);

    while (true)
    {
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("%d", avaliableSpots);
        wait(1);
    }
}
