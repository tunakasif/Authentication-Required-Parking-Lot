#include "mbed.h"
#include "LCD.h"
#include "Gate.h"
#include "RFID.h"
#include "DistSensor.h"
#include "Procedures.h"
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
InterruptIn fish_sensor(PTA5);
InterruptIn card_register(PTA4);
DigitalIn fish_pin(PTA5);
DigitalIn register_pin(PTA4);
Timer timer_gate;
Timer debounce;
Timer timer_register;
TextLCD lcd(PTE20, PTE21, PTE22, PTE23, PTE29, PTE30,
            TextLCD::LCD16x2);
Servo gate(PTA13);
MFRC522 RfChip(SPI_MOSI, SPI_MISO, SPI_SCLK, SPI_CS, MF_RESET);
HCSR04 dist_sensor(TRIG, ECHO);

// Global Constants
const std::string MASTER_ID = "1589AB";
const int NUMBER_OF_PARK_SPOTS = 3;
const int REGISTER_PERIOD = 10; // seconds
const int DEFAULT_LCD_WAIT = 3; // seconds

// Global Variables
std::vector<std::string> id_list;
int avaliableSpots;

// program
int main()
{
    // variables
    std::string currentCardID = "";
    int gate_distance_cm;

    // program code
    // initialize
    gate_distance_cm = get_distance_cm();
    avaliableSpots = NUMBER_OF_PARK_SPOTS;
    debounce.start();
    card_register.rise(&register_ISR);
    fish_sensor.rise(&fish_ISR);
    id_list.push_back(MASTER_ID);
    RfChip.PCD_Init();
    setLED(0, 1, 0); // set the LED to red
    lcd_welcome();
    gate_initialize();

    while (true)
    {
        lcd_welcome();
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
        if (checkList(currentCardID))
        {
            openProcedure(gate_distance_cm);
            wait(1);
            closeProcedure();
        }

        else
        {
            intruderProcedure();
        }
    }
}

// LCD Functions
/**
 * Prints welcome screen to LCD
 */
void lcd_welcome()
{
    lcd.cls();
    lcd.printf("Welcome! Free: %d", avaliableSpots);
    lcd.locate(0, 1);
    lcd.printf("Scan Your Card");
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
 * Indicates the parking lot is full
 */
void lcd_park_full()
{
    lcd.cls();
    lcd.printf("Parking Lot Full");
    lcd.locate(0, 1);
    lcd.printf("Come Back Later");
}

/**
 * Prints the first step of card registration 
 * process. Asks for master card scan.
 */
void lcd_register_master()
{
    lcd.cls();
    lcd.printf("Please Scan");
    lcd.locate(0, 1);
    lcd.printf("Master Card");
}

/**
 * Prints the second step of card registration 
 * process. Asks for new card scan.
 */
void lcd_register_new()
{
    lcd.cls();
    lcd.printf("Please Scan");
    lcd.locate(0, 1);
    lcd.printf("New Card");
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
bool checkList(std::string &cardID)
{
    for (int i = 0; i < id_list.size(); i++)
    {
        if (cardID == id_list.at(i))
            return true;
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
/**
 * initiates the HC-SR04 sensor and
 * returns the distance in cm
 */
unsigned int get_distance_cm()
{
    dist_sensor.start();
    wait_ms(300);
    return dist_sensor.get_dist_cm();
}

// Procedures
/**
 * Open Gate Procedure
 * 1) Set the LED to green
 * 2) Rotate the servo to open the gate
 * 3) Display Access Granted on the LCD
 * 4) Check either a car passed the barrier 
 * or there is a car in the gateway
 * 5) In either case decreases the number of free spots
 * 6) Keeps the gate open if there is a car in the gateway
 */
void openProcedure(int &gate_distance_cm)
{
    if (avaliableSpots > 0)
    {
        // variables
        int distance_cm;
        int count = 0;

        // function code
        timer_gate.reset();
        timer_gate.start();
        setLED(1, 0, 1); // set the LED green
        gate_open();
        lcd_grant_access();
        distance_cm = get_distance_cm();
        while ((timer_gate.read() < DEFAULT_LCD_WAIT) ||
               (distance_cm < gate_distance_cm))
        {
            distance_cm = get_distance_cm();
            if ((count == 0) && (distance_cm < gate_distance_cm))
            {
                // set LED blue to indicate
                setLED(1, 1, 1);
                avaliableSpots--;
                count++;
            }
        }
        timer_gate.stop();
    }

    else
    {
        lcd_park_full();
        wait(DEFAULT_LCD_WAIT);
    }
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
    wait(DEFAULT_LCD_WAIT);
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

/**
 * Flying Fish Sensor ISR for card count
 * 
 * Triggerred by the rising edge of flying fish
 * sensor. And used to keep the track of the 
 * cars that exit. 
 */
void fish_ISR()
{
    // since the flying fish can bounce this wait and
    // pin check makes sure that it is a rising edge
    wait(0.1);
    if (fish_pin.read() == 1)
    {
        if (avaliableSpots < NUMBER_OF_PARK_SPOTS)
            avaliableSpots++;
    }
}

/**
 * ISR of register is triggerred by the rising edge of 
 * the push button.
 * 
 * Gives 10 seconds to scan master card and additional 
 * 10 more seconds for the new card after master card is 
 * scanned. If the scanned card is not in the list adds to
 * list. 
 */
void register_ISR()
{
    if (debounce.read_ms() > 100 && register_pin.read() == 1) // only allow toggle if debounce
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
                    setLED(0, 1, 0); // set LED red

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
                    setLED(1, 0, 1); // set LED green

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
            setLED(0, 1, 0); // set LED red
            timer_register.stop();
        }
        debounce.reset(); // restart timer when the toggle is performed
        lcd_welcome();
    }
}