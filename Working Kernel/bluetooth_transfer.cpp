#include "bluetooth_transfer.h"
#include "filesystem.h"
#include <SoftwareSerial.h>

// Define Bluetooth module pins (RX, TX)
SoftwareSerial btSerial(6, 7);  // RX, TX for Arduino
bool btTransferActive = false;

void bt_init() {
  btSerial.begin(9600);  // Default HC-06 baud rate
  Serial.println(F("Bluetooth module initialized"));
}

void bt_send_file(const char* filename) {
  if (!SD.exists(filename)) {
    Serial.print(F("File not found: "));
    Serial.println(filename);
    return;
  }
  
  btTransferActive = true;
  Serial.print(F("Sending file via Bluetooth: "));
  Serial.println(filename);
  
  // Send file header
  btSerial.print("START:");
  btSerial.println(filename);
  
  // Open file and send contents
  File dataFile = SD.open(filename);
  if (dataFile) {
    while (dataFile.available()) {
      char c = dataFile.read();
      btSerial.write(c);
      // Small delay to prevent buffer overflow
      delay(10);
    }
    dataFile.close();
    
    // Send end marker
    btSerial.println("END:TRANSFER");
    Serial.println(F("File sent successfully"));
  } else {
    Serial.println(F("Error opening file"));
  }
  
  btTransferActive = false;
}

// void bt_receive_file() {
//   btTransferActive = true;
//   Serial.println(F("Waiting for Bluetooth file transfer..."));
//   Serial.println(F("Send file with format: START:filename followed by content then END:TRANSFER"));
  
//   String header = "";
//   bool receivingHeader = true;
//   bool receivingContent = false;
//   String filename = "";
//   File outputFile;
  
//   unsigned long timeout = millis() + 30000; // 30 second timeout
  
//   while (millis() < timeout) {
//     if (btSerial.available()) {
//       timeout = millis() + 30000; // Reset timeout on activity
      
//       if (receivingHeader) {
//         char c = btSerial.read();
//         if (c == '\n') {
//           // Process header
//           if (header.startsWith("START:")) {
//             filename = header.substring(6);
//             filename.trim();
//             Serial.print(F("Receiving file: "));
//             Serial.println(filename);
            
//             // Create or overwrite the file
//             if (SD.exists(filename.c_str())) {
//               SD.remove(filename.c_str());
//             }
//             outputFile = SD.open(filename.c_str(), FILE_WRITE);
            
//             if (!outputFile) {
//               Serial.println(F("Error creating output file"));
//               btTransferActive = false;
//               return;
//             }
            
//             receivingHeader = false;
//             receivingContent = true;
//             header = "";
//           } else {
//             header = "";
//           }
//         } else {
//           header += c;
//         }
//       } else if (receivingContent) {
//         // Check for end marker
//         if (btSerial.available() >= 12) { // Length of "END:TRANSFER"
//           String endCheck = "";
//           for (int i = 0; i < 12; i++) {
//             char c = btSerial.peek();
//             endCheck += c;
//             btSerial.read(); // Remove from buffer
//           }
          
//           if (endCheck == "END:TRANSFER") {
//             outputFile.close();
//             Serial.println(F("File received successfully"));
//             btTransferActive = false;
//             return;
//           } else {
//             // Not the end marker, write the first character to the file
//             outputFile.write(endCheck[0]);
//             // Put the other characters back in the check buffer
//             for (int i = 1; i < 12; i++) {
//               char c = endCheck[i];
//               outputFile.write(c);
//             }
//           }
//         } else {
//           // Still receiving content
//           char c = btSerial.read();
//           outputFile.write(c);
//         }
//       }
//     }
//   }
  
//   // If we get here, it's a timeout
//   Serial.println(F("Bluetooth transfer timed out"));
//   if (outputFile) {
//     outputFile.close();
//   }
//   btTransferActive = false;
// }

void bt_receive_file() {
  // Make sure SD card is initialized first
  if (!initSDCard()) {
    Serial.println(F("Cannot receive file - SD card not initialized."));
    return;
  }
  
  btTransferActive = true;
  Serial.println(F("Waiting for Bluetooth file transfer..."));
  Serial.println(F("Send file with format: START: content END_TRANSFER"));
  
  unsigned long startTime = millis();
  String fullMessage = "";
  bool messageReceived = false;
  
  // Wait up to 60 seconds for a message from btSerial
  while (millis() - startTime < 60000) { 
    if (btSerial.available()) {
      fullMessage = btSerial.readStringUntil('\n');
      fullMessage.trim();
      messageReceived = true;
      break;
    }
  }
  
  if (!messageReceived) {
    Serial.println(F("Timeout waiting for file transfer"));
    btTransferActive = false;
    return;
  }
  
  // Check that the message starts with "START:" and contains "END_TRANSFER"
  if (!fullMessage.startsWith("START:") || fullMessage.indexOf("END_TRANSFER") == -1) {
    Serial.println(F("Invalid file format"));
    btTransferActive = false;
    return;
  }
  
  // Extract the content between the markers
  int startIdx = strlen("START:"); // position right after "START:"
  int endIdx = fullMessage.indexOf("END_TRANSFER");
  String fileContent = fullMessage.substring(startIdx, endIdx);
  fileContent.trim();
  
  // Debug: Print extracted content
  Serial.println(F("Extracted file content:"));
  Serial.println(fileContent);
  
  // Define a default filename (alternatively, you could generate a unique name)
  String filename = "received.txt";
  Serial.print(F("Saving file as: "));
  Serial.println(filename);
  
  // Check if file already exists
  if (SD.exists(filename.c_str())) {
    Serial.println(F("File already exists. Overwriting..."));
  }
  
  // Create the file on the SD card
  File outputFile = SD.open(filename.c_str(), FILE_WRITE);
  if (!outputFile) {
    Serial.println(F("Error creating output file"));
    btTransferActive = false;
    return;
  }
  
  // Write content to the file
  outputFile.print(fileContent);
  outputFile.close();
  
  Serial.println(F("File received and saved successfully"));
  btTransferActive = false;
}

void bt_diagnostic() {
  Serial.println(F("Running Bluetooth diagnostics..."));
  Serial.println(F("Sending test message via Bluetooth"));
  
  btSerial.println("AT");
  delay(1000);
  
  Serial.println(F("Bluetooth module response:"));
  while (btSerial.available()) {
    Serial.write(btSerial.read());
  }
  
  btSerial.println("AT+VERSION");
  delay(1000);
  
  while (btSerial.available()) {
    Serial.write(btSerial.read());
  }
  
  Serial.println(F("Diagnostic complete"));
}