#include "mbed.h"
#include "MFRC522.h"
#include "TextLCD.h"

//KL25Z Pins for MFRC522 SPI interface
#define SPI_CS PTD0 // SDA
#define SPI_SCLK PTD1
#define SPI_MOSI PTD2
#define SPI_MISO PTD3
#define MF_RESET PTD5
// KL25Z Pins for Debug UART port
#define UART_RX PTA1
#define UART_TX PTA2

DigitalOut LedRed(LED_RED);
DigitalOut LedGreen(LED_GREEN);
DigitalOut LedBlue(LED_BLUE);

Serial DebugUART(UART_TX, UART_RX);
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30, TextLCD::LCD16x2);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);

int main()
{
    // LCD
    LedRed = 1;
    LedGreen = 0;
    LedBlue = 0;
    lcd.cls();

    // Set debug UART speed
    DebugUART.baud(115200);

    // Init. RC522 Chip
    RfChip.PCD_Init();

    while (true)
    {
        LedRed = 1;
        LedGreen = 0;
        LedBlue = 0;

        // Look for new cards
        if (!RfChip.PICC_IsNewCardPresent())
        {
            wait_ms(500);
            continue;
        }

        LedRed = 0;
        LedGreen = 0;
        LedBlue = 1;

        // Select one of the cards
        if (!RfChip.PICC_ReadCardSerial())
        {
            wait_ms(500);
            continue;
        }

        LedRed = 0;
        LedGreen = 1;
        LedBlue = 0;

        // Print Card UID
        lcd.cls();
        printf("Card UID: ");
        lcd.printf("Card UID: ");
        lcd.locate(0, 1);
        for (uint8_t i = 0; i < RfChip.uid.size; i++)
        {
            printf(" %X02", RfChip.uid.uidByte[i]);
            lcd.printf("%X:", RfChip.uid.uidByte[i]);
        }
        printf("\n\r");

        // Print Card type
        uint8_t piccType = RfChip.PICC_GetType(RfChip.uid.sak);
        printf("PICC Type: %s \n\r", RfChip.PICC_GetTypeName(piccType));
        wait_ms(1000);
    }
}
