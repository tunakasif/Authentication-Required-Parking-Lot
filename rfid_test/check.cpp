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
void getCardID(std::string &currentCardID)
{
    currentCardID = "";
    for (uint8_t i = 0; i < RfChip.uid.size; i++)
    {
        currentCardID += RfChip.uid.uidByte[i];
    }
}

void printCardID(std::string &cardID)
{
    for (int i = 0; i < RfChip.uid.size; i++)
    {
        lcd.printf("%X:", cardID[i]);
    }
}

int main()
{
    // variables
    std::string currentCardID = "";
    std::vector<std::string> id_list;
    id_list.push_back(MASTER_ID);

    // program code
    // Initialize
    lcd.cls();
    RfChip.PCD_Init();

    // wait for new card read then execute procedure for the new card
    while (true)
    {
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

        wait(1);
    }
}
