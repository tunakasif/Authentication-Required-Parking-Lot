#include "mbed.h"
#include "hcsr04.h"
#include "TextLCD.h"

HCSR04 sensor(PTA12, PTD4);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
unsigned int dist;

int main()
{
    lcd.cls();
    while (true)
    {
        sensor.start();
        wait_ms(500);
        dist = sensor.get_dist_cm();
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("cm: %ld", dist);
    }
}