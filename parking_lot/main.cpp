#include "mbed.h"
#include "TextLCD.h"
#include "Servo.h"

TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
Servo gate(PTA13);

// LCD Functions
void lcd_welcome()
{
    lcd.cls();
    lcd.printf("Welcome! Please");
    lcd.locate(0, 1);
    lcd.printf("Read Your Card");
}

void lcd_verify()
{
    lcd.cls();
    lcd.printf("Verifying...");
}

void lcd_grant_access()
{
    lcd.cls();
    lcd.printf("Access Granted");
    lcd.locate(0, 1);
    lcd.printf("Welcome!");
}

void lcd_intruder()
{
    lcd.cls();
    lcd.printf("Access Denied");
    lcd.locate(0, 1);
    lcd.printf("Use A Valid Card");
}

void lcd_count()
{
    int number = 0;
    while (number < 10)
    {
        lcd.locate(15, 1);
        lcd.printf("%d", number);
        number++;
        wait(1);
    }
}

// Gate Functions
void gate_open()
{
    gate = 1.0;
    lcd_grant_access();
}

void gate_close()
{
    gate = 0.0;
    lcd_welcome();
}

void gate_initialize()
{
    gate.calibrate(0.0005, 0.0);
    wait(0.02);
    gate_close();
}

int main()
{
    // variables
    int numberOfOpenClose = 3;

    // program code
    gate_initialize();

    lcd_welcome();
    lcd_count();
    lcd_verify();
    wait(2);
    lcd_grant_access();
    wait(2);
    lcd_intruder();
    wait(2);

    wait(3);
    while (numberOfOpenClose > 0)
    {
        gate_open();
        wait(2);
        gate_close();
        wait(2);
        numberOfOpenClose--;
    }
}
