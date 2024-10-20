#include "Audio.h"
#include "CloudSpeechClient.h"
#include <SPI.h>
#include <WiFi.h>
#include "network_param.h"

// WiFi and API Key configuration
const char* sSid = "iPhone";
const char* passWord = "Barnes!!";
const char* apikeyy = "AIzaSyB_eFvcXt8CfCLd-fMd83Ze0bcwHRBdFFc";  // API key for Google Cloud

I2S i2s(ADMP441);  // Initialize the I2S object with the ADMP441 mic type

void setup() {
    // Begin serial communication for debugging
    Serial.begin(115200);
    delay(500);

    // Initialize WiFi connection
    Serial.println("Initializing WiFi connection...");
    WiFi.begin(sSid, passWord);
    int attempts = 0;

    // Attempt to connect to WiFi with a timeout
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {  // 20-second timeout
        delay(1000);
        Serial.print("Attempting to connect to WiFi... ");
        Serial.println(attempts + 1);
        attempts++;
    }

    // Check if WiFi connection was successful
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to WiFi. Check your credentials.");
        return;
    }
    Serial.println("WiFi connected successfully!");

    // Begin audio recording process
    Serial.println("\nRecord start!\n");
    Serial.println("Initializing Audio...");

    // Create the Audio object (linked to the I2S class)
    Audio* audio = new Audio(ADMP441);  // Initialize Audio with microphone type

    Serial.println("Starting audio recording...");
    audio->Record(i2s);  // Pass I2S object to record audio

    Serial.println("Recording Completed. Now Processing...");

    // Initialize the CloudSpeechClient for Google Speech-to-Text API
    Serial.println("Initializing CloudSpeechClient...");
    CloudSpeechClient* cloudSpeechClient = new CloudSpeechClient(USE_APIKEY);  // Change to USE_APIKEY

    // Transcribe the recorded audio
    cloudSpeechClient->Transcribe(audio);

    // Clean up resources
    delete cloudSpeechClient;
    delete audio;

    Serial.println("Process completed.");
}

void loop() {
    // Nothing required in the loop
}
