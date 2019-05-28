#include "mbed.h"
#include "MFRC522.h"
#include "TextLCD.h"

//KL25Z Pins for MFRC522 SPI interface
#define SPI_CS PTD0 // SDA
#define SPI_SCLK PTD1
#define SPI_MOSI PTD2
#define SPI_MISO PTD3
#define MF_RESET PTD5

DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);
DigitalOut blueLED(LED3);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);

int main()
{
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

        // Print Card UID
        lcd.cls();
        lcd.printf("Card UID: ");
        lcd.locate(0, 1);
        for (uint8_t i = 0; i < RfChip.uid.size; i++)
        {
            lcd.printf("%X:", RfChip.uid.uidByte[i]);
        }
        wait(1);
    }
}
