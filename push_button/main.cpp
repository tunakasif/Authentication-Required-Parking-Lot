#include "mbed.h"
#include "TextLCD.h"

TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30,
            TextLCD::LCD16x2);
DigitalIn button_pin(PTA4);
DigitalOut led1(LED1);
InterruptIn button(PTA4);    // Interrupt on digital pushbutton input 
Timer debounce;                  // define debounce timer

void toggle(void);               // function prototype

int count;

int main() 
{
    count = 0;
    lcd.cls();
    lcd.printf("Hi");
    debounce.start();
    button.rise(&toggle);          // attach the address of the toggle
    
    while (true)
    {
        lcd.locate(0, 0);
        lcd.printf("Hi, %d", button_pin.read());
        wait(0.2);
    }
}                                          
// function to the rising edge
void toggle() 
{
    if (debounce.read_ms() > 100 && button_pin.read() == 1) // only allow toggle if debounce
    {      
        count++;
        lcd.locate(0, 1);
        lcd.printf("%d", count);
        led1=!led1; 
        debounce.reset();              // restart timer when the toggle is performed
    }
}
