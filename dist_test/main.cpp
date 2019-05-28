#include "mbed.h"
#include "hcsr04.h"
#include "TextLCD.h"
 
DigitalOut myled(LED1);
HCSR04  usensor(PTA12, PTD4);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
unsigned int dist;

int main()
{
    lcd.cls();
    lcd.printf("Tuna");
    while(1) {
        usensor.start();
        wait_ms(500); 
        dist=usensor.get_dist_cm();
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("cm: %ld", dist);
    }
}