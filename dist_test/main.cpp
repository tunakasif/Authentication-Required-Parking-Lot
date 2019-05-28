#include "mbed.h"
#include "hcsr04.h"
#include "TextLCD.h"

HCSR04 sensor(PTA12, PTD4);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
float dist;
unsigned int dist_cm;

int main()
{
    lcd.cls();
    while (true)
    {
        sensor.start();
        wait_ms(500);
        dist = sensor.get_dist_mm();
        dist_cm = sensor.get_dist_cm();
        lcd.cls();
        lcd.locate(0, 0);
        lcd.printf("Dist: %.1f mm", dist);
        lcd.locate(0, 1);
        lcd.printf("Dist: %d cm", dist_cm);
    }
}