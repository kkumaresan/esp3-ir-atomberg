#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

// Pin definitions
const uint16_t RECV_PIN = 10; // IR receiver pin
const uint16_t SEND_PIN = 7;  // IR sender pin

// IR receiver object
IRrecv irrecv(RECV_PIN, 1024); // Added buffer size
decode_results results;

// IR sender object
IRsend irsend(SEND_PIN);

// Forward declarations
void printReceivedSignal();
void handleSerialInput();

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for serial to stabilize

  Serial.println("\n=== IR Remote Control ===");
  Serial.println("Initializing IR Receiver on GPIO 10...");

  // Initialize IR receiver
  irrecv.enableIRIn();

  Serial.println("IR Receiver initialized!");
  Serial.println("Initializing IR Sender on GPIO 7...");

  // Initialize IR sender
  irsend.begin();

  Serial.println("IR Sender initialized!");
  Serial.println("\nCommands:");
  Serial.println("  Receive: Point IR remote at GPIO 10");
  Serial.println("  Send: Enter hex code (e.g., 12345678) to send via GPIO 7");
  Serial.println("===========================\n");
}

void loop() {
  // Check for received IR signal
  if (irrecv.decode(&results))
  {
    printReceivedSignal();
    irrecv.resume(); // Resume receiver to capture next signal
  }

  // Check for serial input to send IR signal
  if (Serial.available())
  {
    handleSerialInput();
  }

  delay(50);
}

void printReceivedSignal()
{
  Serial.print("Received: ");
  Serial.print("Protocol=");
  Serial.print(typeToString(results.decode_type));
  Serial.print(" | Code=0x");
  Serial.println(uint64ToString(results.value, 16));
}

void handleSerialInput()
{
  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.length() == 0)
  {
    return;
  }

  // Parse hex code from input
  uint64_t code = 0;

  // Check if input contains protocol prefix (e.g., "NEC:12345678")
  int colonIndex = input.indexOf(':');
  String hexString;

  if (colonIndex != -1)
  {
    // Protocol specified, extract just the hex part
    hexString = input.substring(colonIndex + 1);
  }
  else
  {
    // No protocol specified, use whole input as hex code
    hexString = input;
  }

  // Convert hex string to uint64_t
  code = strtoul(hexString.c_str(), NULL, 16);

  if (code == 0 && hexString != "0")
  {
    Serial.println("Invalid hex code. Please enter a valid hex value (e.g., 12345678)");
    return;
  }

  // Send as generic/raw NEC signal (most common protocol)
  Serial.print("Sending code 0x");
  Serial.println(uint64ToString(code, 16));

  irsend.sendNEC(code, 32); // Send as 32-bit NEC code

  delay(100);
}