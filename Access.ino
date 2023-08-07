/*
 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#define BUZZER_PIN      4          // Pin for the buzzer
#define GREEN_LED_PIN   3          // Pin for the green LED
#define RED_LED_PIN     2          // Pin for the red LED

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
// byte accessUID[4] = {06, 11, A1, 01};
byte accessUID[] = {0x0C, 0x93, 0xB9, 0x70};

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  pinMode(BUZZER_PIN, OUTPUT); 
  pinMode(GREEN_LED_PIN, OUTPUT);    // Set green LED pin as output
  pinMode(RED_LED_PIN, OUTPUT);      // Set red LED pin as output
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	// mfrc522.PICC_DumpToSerial(&(mfrc522.uid));


if (memcmp(mfrc522.uid.uidByte, accessUID, sizeof(accessUID)) == 0) {
		Serial.println("Access Granted");
    digitalWrite(RED_LED_PIN, LOW);     // Turn on red LED
    digitalWrite(GREEN_LED_PIN, HIGH);   // Turn on green LED
    buzz(BUZZER_PIN, 100, 500);          // Buzz the buzzer for 100ms (short duration)
	} else {
		Serial.println("Access Denied");
    digitalWrite(RED_LED_PIN, HIGH);     // Turn on red LED
    digitalWrite(GREEN_LED_PIN, LOW);   // Turn off green LED
    buzz(BUZZER_PIN, 1000, 1000);        // Buzz the buzzer for 1000ms (long duration)
	}

	mfrc522.PICC_HaltA();
}

void buzz(int pin, long onTime, long offTime) {
  digitalWrite(pin, HIGH);
  delay(onTime);
  digitalWrite(pin, LOW);
  delay(offTime);
}
