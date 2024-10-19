#include "Audio.h"
#include <SD.h>
 #include <SD_MMC.h>
 #include <SPIFFS.h>
 #include <FFat.h>
#include "Audio.h"
#include "I2S.h"
#include <base64.h>

// Constructor
Audio::Audio(MicType micType) {
    this->wavDataSize = 32000;  // Total size of the audio data
    this->dividedWavDataSize = 1024;  // Size of each chunk of the audio data

    // Allocate memory for the divided WAV data
    wavData = new char*[wavDataSize / dividedWavDataSize];  
    for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) {
        wavData[i] = new char[dividedWavDataSize];
    }

    // Initialize the content string for Base64 encoding
    content = "";
}

// Destructor
Audio::~Audio() {
    for (int i = 0; i < wavDataSize / dividedWavDataSize; ++i) {
        delete[] wavData[i];
    }
    delete[] wavData;
}

// Function to create the WAV file header
void Audio::CreateWavHeader(byte* header, int waveDataSize) {
    // (No changes to this function)
}

// Function to record audio data using the I2S class
void Audio::Record(I2S &i2s) {
    // Create WAV header
    CreateWavHeader(reinterpret_cast<byte*>(paddedHeader), wavDataSize);

    size_t bytes_read;
    for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
        // Read data from I2S using the I2S class
        int result = i2s.Read(wavData[j], dividedWavDataSize);
        if (result == -1) {
            Serial.println("Error reading I2S data");
            break;
        }
    }

    Serial.println("Audio recorded successfully.");
}

// Function to encode the recorded audio to Base64
void Audio::EncodeToBase64() {
    content = base64::encode(reinterpret_cast<const uint8_t*>(paddedHeader), sizeof(paddedHeader));
    content.replace("\n", "");

    // Debugging: Print the first few bytes of the Base64-encoded header
    Serial.println("Encoded header:");
    Serial.println(content.substring(0, 100));  // Print the first 100 characters of the encoded header

    for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
        String encodedChunk = base64::encode(reinterpret_cast<const uint8_t*>(wavData[j]), dividedWavDataSize);
        encodedChunk.replace("\n", "");
        content += encodedChunk;
        
        // Debugging: Print the first few bytes of each chunk
        if (j < 5) {  // Print only the first few chunks for verification
            Serial.println("Encoded chunk " + String(j) + ":");
            Serial.println(encodedChunk.substring(0, 100));  // Print first 100 characters of each chunk
        }
    }

    // Debugging output for full encoded data
    Serial.println("Full Encoded Audio Data (first 1000 characters):");
    Serial.println(content.substring(0, 1000));  // Print the first 1000 characters of the full encoded data
}
