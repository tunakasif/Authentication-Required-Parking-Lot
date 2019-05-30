#include "mbed.h"
#include "MFRC522.h"
#include "TextLCD.h"
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
InterruptIn card_register(PTA5);
DigitalIn register_pin(PTA5);
Timer timer_register;
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);

// Global Variables
const std::string MASTER_ID = "1589AB";
const int REGISTER_PERIOD = 10; // seconds
const int DEFAULT_LCD_WAIT = 3; // seconds
std::vector<std::string> id_list;

// methods
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
bool checkList(std::string &cardID)
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

void lcd_welcome()
{
    lcd.cls();
    lcd.printf("Welcome! Free:");
    lcd.locate(0, 1);
    lcd.printf("Scan Your Card");
}

void lcd_intruder()
{
    lcd.cls();
    lcd.printf("Access Denied");
    lcd.locate(0, 1);
    lcd.printf("Use A Valid Card");
}

void lcd_register_master()
{
    lcd.cls();
    lcd.printf("Please Scan");
    lcd.locate(0, 1);
    lcd.printf("Master Card");
}

void lcd_register_new()
{
    lcd.cls();
    lcd.printf("Please Scan");
    lcd.locate(0, 1);
    lcd.printf("New Card");
}

void register_ISR()
{
    // since the flying fish can bounce this wait and
    // pin check makes sure that it is a rising edge
    wait(0.1);
    if (register_pin.read() == 1)
    {
        // variables
        std::string currentCardID = "";

        // ISR code
        lcd_register_master();
        timer_register.reset();
        timer_register.start();

        while (timer_register.read() < REGISTER_PERIOD && currentCardID != MASTER_ID)
        {
            // set the LED blue
            redLED = 1;
            greenLED = 1;
            blueLED = 1;

            lcd.locate(14, 1);
            lcd.printf("%d", (int)(REGISTER_PERIOD - timer_register.read()));

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

            getCardID(currentCardID);
            if (currentCardID != MASTER_ID)
            {
                // set the LED red
                redLED = 0;
                greenLED = 1;
                blueLED = 0;

                lcd_intruder();
                wait(std::min(DEFAULT_LCD_WAIT,
                              (int)(timer_register.read() - 1)));
                lcd_register_master();
            }
        }

        if (currentCardID == MASTER_ID)
        {
            // set the LED green
            redLED = 1;
            greenLED = 0;
            blueLED = 1;
            lcd_register_new();
            wait(DEFAULT_LCD_WAIT);
            timer_register.reset();

            while (timer_register.read() < REGISTER_PERIOD && currentCardID == MASTER_ID)
            {
                // set the LED red
                redLED = 0;
                greenLED = 1;
                blueLED = 0;

                lcd.locate(14, 1);
                lcd.printf("%d", (int)(REGISTER_PERIOD - timer_register.read()));

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

                // set the LED green
                redLED = 1;
                greenLED = 0;
                blueLED = 1;

                getCardID(currentCardID);
                if (!checkList(currentCardID))
                {
                    id_list.push_back(currentCardID);
                    lcd.cls();
                    lcd.printf("New Card Added");
                    wait(DEFAULT_LCD_WAIT);
                }
                else
                {
                    lcd.cls();
                    lcd.printf("Already In List");
                    wait(DEFAULT_LCD_WAIT);
                }
            }
        }
        lcd_welcome();
        timer_register.stop();
    }
}

int main()
{
    // variables
    std::string currentCardID = "";

    // program code
    // Initialize
    id_list.push_back(MASTER_ID);
    lcd.cls();
    RfChip.PCD_Init();
    card_register.rise(&register_ISR);

    // wait for new card read then execute procedure for the new card
    while (true)
    {
        // set the LCD
        lcd.cls();
        lcd_welcome();

        // set the LED to red
        redLED = 0;
        greenLED = 1;
        blueLED = 0;

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

        // set the LED green
        redLED = 1;
        greenLED = 0;
        blueLED = 1;

        getCardID(currentCardID);
        lcd.cls();
        printCardID(currentCardID);
        lcd.locate(0, 1);

        if (checkList(currentCardID))
        {
            lcd.printf("Open");
        }
        else
        {
            lcd.printf("Close");
        }
        wait(5);
    }
}
