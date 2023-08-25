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

// Imports
#include <SPI.h>
#include <MFRC522.h>

// Pins
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#define BUZZER_PIN      4          // Pin for the buzzer
#define GREEN_LED_PIN   3          // Pin for the green LED
#define RED_LED_PIN     2          // Pin for the red LED

// Commands
#define STORE         's'
#define DELETE        'd'
#define VERIFY        'v'
#define LISTALLTAGS   'l'

// MFRC522 Instance
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Constants
const byte MAX_TAGS = 10;
const byte TAG_UID_LENGTH = 4;

struct StoredTag {
  byte uid[TAG_UID_LENGTH];
};

// Variables
StoredTag storedTags[MAX_TAGS];
byte storedTagCount = 0;

void setup() {
  delay(2000);				                          // Optional 2s delay. Some board do need more time after init to be ready
	Serial.begin(9600);		                        // Initialize serial communications with the PC
	while (!Serial);		                          // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			                            // Init SPI bus
	mfrc522.PCD_Init();		                        // Init MFRC522
	mfrc522.PCD_DumpVersionToSerial();	          // Show details of PCD - MFRC522 Card Reader details
	Serial.println("RFID-RC522 Initialized");
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

  byte currentTagUID[TAG_UID_LENGTH];
  Serial.print("Reading Tag UID -> ");
  for(byte i = 0; i < mfrc522.uid.size; i++) {
    currentTagUID[i] = mfrc522.uid.uidByte[i];
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(" ");
  };
  Serial.println();
  Serial.println("Done reading Tag UID");

  mfrc522.PICC_HaltA();

  Serial.println("Type in a command to perform an action");
  Serial.println("s: Store a tag");
  Serial.println("d: Delete a tag");
  Serial.println("v: Verify a tag/Check tag's permission");
  Serial.println("l: List all tags currently stored in memory");
  Serial.println();

  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case STORE:
        storeTag(currentTagUID);
        break;
      case DELETE:
        deleteTag(currentTagUID);
        break;
      case VERIFY:
        if(checkAccessPermission(currentTagUID)) {
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
        break;
      case LISTALLTAGS:
        listStoredTags();
        break;
    }
  }
}


 bool checkAccessPermission(const byte* tagUID) {
   for(byte i = 0; i < storedTagCount; i++) {
     if(memcmp(tagUID, storedTags[i].uid, TAG_UID_LENGTH) == 0) {
      return true;
     }
   }

   return false;
 }

 void storeTag(const byte* tagUID) {
    Serial.println("Storing UID in memory");
    if(storedTagCount < MAX_TAGS) {
      StoredTag newTag;

      memcpy(newTag.uid, tagUID, TAG_UID_LENGTH);
      storedTags[storedTagCount++] = newTag;
      Serial.print("Tag stored successfully. UID: ");

      for (byte i = 0; i < TAG_UID_LENGTH; i++) {
        // Printing the current tag that's been saved to serial.
        Serial.print(newTag.uid[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
     Serial.println("Storage is full. Cannot store more tags");
    }
 }

 void deleteTag(const byte* tagUID) {
    Serial.println("Deleting UID in memory");

   for (byte i = 0; i < TAG_UID_LENGTH; i++) {
      // Printing the current tag that's about to be deleted to serial.
      Serial.print(tagUID[i], HEX);
      Serial.print(" ");
    }

    for(byte i = 0; i < storedTagCount; i++) {
      if(memcmp(tagUID, storedTags[i].uid, TAG_UID_LENGTH) == 0) {
        for(byte j = 1; j < storedTagCount - 1; j++) {
          //Shift the remaining tags to fill the gap
          storedTags[j] = storedTags[j + 1];
        }
        storedTagCount--;
        Serial.print("-> Tag deleted successfully." );
        Serial.println();
        return;
      }
    }
    Serial.println("Tag not found. Deletion failed");
 }

 void listStoredTags() {
  Serial.println("Stored Tags:");
  for (byte i = 0; i < storedTagCount; i++) {
    Serial.print("Tag ");
    Serial.print(i + 1);
    Serial.print(": ");
    for (byte j = 0; j < TAG_UID_LENGTH; j++) {
      Serial.print(storedTags[i].uid[j], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}


void buzz(int pin, long onTime, long offTime) {
  digitalWrite(pin, HIGH);
  delay(onTime);
  digitalWrite(pin, LOW);
  delay(offTime);
}
