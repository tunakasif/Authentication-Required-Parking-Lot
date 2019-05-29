#include "mbed.h"
#include "TextLCD.h"
#include "Servo.h"
#include "MFRC522.h"
#include "hcsr04.h"
#include <string>
#include <vector>

//KL25Z Pins for MFRC522 SPI interface
#define SPI_CS PTD0 // SDA
#define SPI_SCLK PTD1
#define SPI_MOSI PTD2
#define SPI_MISO PTD3
#define MF_RESET PTD5
//KL25Z Pins for HC-SR04 interface
#define ECHO PTD4
#define TRIG PTA12

// Constraints for the system
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
DigitalOut blueLED(LED3);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
Servo gate(PTA13);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);
HCSR04 dist_sensor(TRIG, ECHO);

// Global Variables
const std::string MASTER_ID = "1589AB";
int gate_distance_cm = 11;

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
    lcd.locate(14, 1);
    lcd.printf("" + gate_distance_cm);
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
}

/**
 * Rotates the servo to close the gate 
 */
void gate_close()
{
    gate = 0.0;
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

// RFID Functions
/**
 * gets the card ID from serial read and stores 
 * it as hexadecimal without spaces 
 */
void getCardID(std::string &currentCardID)
{
    currentCardID = "";
    char buffer[8]; // created for sprintf()

    for (uint8_t i = 0; i < RfChip.uid.size; i++)
    {
        sprintf(buffer, "%X", RfChip.uid.uidByte[i]);
        currentCardID += buffer;
    }
}

/**
 * prints the card ID in the form of hexadecimal 
 * without spaces
 */
void printCardID(std::string &cardID)
{
    lcd.printf(cardID.c_str());
}

/** 
 * checks the authorized ID list with the given card ID.
 * If there is a match returns true, if card is not in 
 * the list returns false
 */
bool checkList(std::vector<std::string> &id_list, std::string &cardID)
{
    for (int i = 0; i < id_list.size(); i++)
    {
        if (cardID == id_list.at(i))
        {
            return true;
        }
    }
    return false;
}

// LED Functions
/**
 * sets the rgb takes either 0 or 1 for each color
 */
void setLED(bool red, bool green, bool blue)
{
    redLED = red;
    greenLED = green;
    blueLED = blue;
}

// HC-SR04 Distance Sensor Functions
unsigned int get_distance_cm()
{
    dist_sensor.start();
    wait_ms(500);
    return dist_sensor.get_dist_cm();
}

float get_distance_mm()
{
    dist_sensor.start();
    wait_ms(500);
    return dist_sensor.get_dist_mm();
}

// Procedures
/**
 * Open Gate Procedure
 * 1) Set the LED to green
 * 2) Rotate the servo to open the gate
 * 3) Display Access Granted on the LCD
 */
void openProcedure()
{
    setLED(1, 0, 1); // set the LED green
    gate_open();
    lcd_grant_access();
    wait(3);
    do
    {
        // wait here until car clears the gate
    } while (dist_sensor.get_dist_cm() < gate_distance_cm);
}

/**
 * Intruder Procedure
 * 1) Set the LED to red
 * 2) Make sure gate is closed
 * 3) Display Access Denied on the LCD
 * 4) Wait for 3 seconds for message to be read
 * 5) Display default welcome screen afterwards
 */
void intruderProcedure()
{
    setLED(0, 1, 0); // set the LED red
    gate_close();
    lcd_intruder();
    wait(3);
    lcd_welcome();
}

/**
 * Close Gate Procedure
 * 1) Set the LED to red
 * 2) Rotate the servo to close the gate
 * 3) Display default welcome screen afterwards
 */
void closeProcedure()
{
    setLED(0, 1, 0); // set the LED red
    gate_close();
    lcd_welcome();
}

// program
int main()
{
    // variables
    std::string currentCardID = "";
    std::vector<std::string> id_list;

    // program code
    // initialize
    id_list.push_back(MASTER_ID);
    lcd_welcome();
    RfChip.PCD_Init();
    gate_initialize();
    setLED(0, 1, 0); // set the LED to red
    gate_distance_cm = dist_sensor.get_dist_cm();

    while (true)
    {
        // Look for new card
        if (!RfChip.PICC_IsNewCardPresent())
        {
            wait_ms(500);
            continue;
        }

        // Read the Card
        if (!RfChip.PICC_ReadCardSerial())
        {
            wait_ms(500);
            continue;
        }

        // Here card is read, get ID
        getCardID(currentCardID);

        // check if the card is registerred
        if (checkList(id_list, currentCardID))
        {
            openProcedure();
            wait(1);
            closeProcedure();
        }

        else
        {
            intruderProcedure();
        }
    }
}
