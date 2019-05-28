#include "mbed.h"
#include "TextLCD.h"
#include "Servo.h"
#include "MFRC522.h"
#include <string>
#include <vector>

//KL25Z Pins for MFRC522 SPI interface
#define SPI_CS PTD0 // SDA
#define SPI_SCLK PTD1
#define SPI_MOSI PTD2
#define SPI_MISO PTD3
#define MF_RESET PTD5

// Constraints for the system
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
DigitalOut blueLED(LED3);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
Servo gate(PTA13);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);

// LCD Functions
/**
 * Prints welcome screen to LCD
 */
void lcd_welcome()
{
    lcd.cls();
    lcd.printf("Welcome! Please");
    lcd.locate(0, 1);
    lcd.printf("Read Your Card");
}

/**
 * Prints "Verifying..." to LCD
 */
void lcd_verify()
{
    lcd.cls();
    lcd.printf("Verifying...");
}

/**
 * Prints access granted to LCD
 */
void lcd_grant_access()
{
    lcd.cls();
    lcd.printf("Access Granted");
    lcd.locate(0, 1);
    lcd.printf("Welcome!");
}

/**
 * Prints access denied to LCD
 */
void lcd_intruder()
{
    lcd.cls();
    lcd.printf("Access Denied");
    lcd.locate(0, 1);
    lcd.printf("Use A Valid Card");
}

/**
 * Counts to 10 at bottom-right corner
 * of the LCD 
 */
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
/**
 * Rotates the servo to open the gate 
 * Prints welcome screen to LCD
 */
void gate_open()
{
    gate = 1.0;
    lcd_grant_access();
}

/**
 * Rotates the servo to open the gate 
 * Displays default welcome screen on LCD afterwards
 */
void gate_close()
{
    gate = 0.0;
    lcd_welcome();
}

/**
 * Calibrates the servo 
 * Initializes as closed
 */
void gate_initialize()
{
    gate.calibrate(0.0005, 0.0);
    wait(0.02);
    gate_close();
}

// program
int main()
{
    // variables
    int numberOfOpenClose = 3;

    // program code
    gate_initialize();

    // test LCD
    lcd_welcome();
    lcd_count();
    lcd_verify();
    wait(2);
    lcd_grant_access();
    wait(2);
    lcd_intruder();
    wait(2);

    // Test Servo
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
