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
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);

// Global Variables
const std::string MASTER_ID = "36A1B815";

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

int main()
{
    // variables
    std::string currentCardID = "";
    std::vector<std::string> id_list;

    // program code
    // Initialize
    id_list.push_back(MASTER_ID);
    lcd.cls();
    RfChip.PCD_Init();

    // wait for new card read then execute procedure for the new card
    while (true)
    {
        // set the LCD
        lcd.cls();
        lcd.printf("Read Card");

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

        if (checkList(id_list, currentCardID))
        {
            lcd.printf("Open");
        }
        else
        {
            lcd.printf("Close");
        }
        wait(2);
    }
}
