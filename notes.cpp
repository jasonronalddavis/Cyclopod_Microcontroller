#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include <SPI.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <driver/i2s.h>


// WiFi credentials
const char* ssid = "iPhone";
const char* password = "Barnes!!";
const char* serverUrl = "https://bright-dragon-1cbd6d.netlify.app/.netlify/functions/transcribe";

// I2S pins
#define I2S_WS 15    // LRCL
#define I2S_SD 32    // DOUT
#define I2S_SCK 14   // BCLK

// I2S configuration
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define CHANNELS 1
#define SAMPLE_BUFFER_SIZE 512

bool isAllZeros(int32_t* samples, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (samples[i] != 0) return false;
    }
    return true;
}

void setupI2S() {
    Serial.println("Starting I2S setup...");

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    Serial.println("I2S config structure created");

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    Serial.println("Pin config structure created");

    ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
    Serial.println("I2S driver installed");

    ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config));
    Serial.println("I2S pins set");

    ESP_ERROR_CHECK(i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_MONO));
    Serial.println("I2S clock set");
}

String recordAndEncodeAudio() {
    const size_t BUFFER_SIZE = 1024;
    int32_t* raw_samples = new int32_t[BUFFER_SIZE];
    int16_t* processed_samples = new int16_t[BUFFER_SIZE];
    String base64Audio = "";
    
    size_t bytesRead = 0;
    Serial.println("Starting audio recording...");
    
    // Read audio samples
    esp_err_t result = i2s_read(I2S_NUM_0, raw_samples, BUFFER_SIZE * 4, &bytesRead, portMAX_DELAY);
    
    if (result == ESP_OK && bytesRead > 0) {
        Serial.printf("Read %d bytes from I2S\n", bytesRead);
        
        // Process samples
        for (int i = 0; i < BUFFER_SIZE; i++) {
            // Convert 32-bit to 16-bit and normalize
            processed_samples[i] = raw_samples[i] >> 16;

            
        }
        
        // Print first few samples for debugging
        Serial.println("Sample values:");
        for (int i = 0; i < 5; i++) {
            Serial.printf("Raw[%d] = %ld, Processed[%d] = %d\n", 
                        i, raw_samples[i], i, processed_samples[i]);
        }
        
        // Convert to base64
        int encodedLen = Base64.encodedLength(bytesRead / 2);
        char* encodedString = new char[encodedLen + 1];
        
        Base64.encode(encodedString, (char*)processed_samples, bytesRead / 2);
        base64Audio = String(encodedString);
        
        Serial.printf("Recorded audio length: %d bytes\n", bytesRead);
        Serial.printf("Base64 length: %d\n", base64Audio.length());
        Serial.println("First 20 chars of base64: " + base64Audio.substring(0, 20));
        
        delete[] encodedString;
    } else {
        Serial.printf("Failed to read I2S data: %d\n", result);
    }
    
    // Clean up
    delete[] raw_samples;
    delete[] processed_samples;
    
    return base64Audio;
}

void sendAudioToServer(const String& base64Audio) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        Serial.println("Connecting to server...");
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        String payload = "{\"audio\":\"" + base64Audio + "\"}";
        Serial.println("Sending audio data...");
        Serial.printf("Payload size: %d bytes\n", payload.length());
        
        int httpResponseCode = http.POST(payload);
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response:");
            Serial.println(response);
            
            // Try to parse response and check decoded text
            if (response.indexOf("decodedText") > -1) {
                Serial.println("Successfully decoded audio on server");
            }
        } else {
            Serial.print("Error in sending request: ");
            Serial.println(httpResponseCode);
        }
        
        http.end();
    } else {
        Serial.println("WiFi Disconnected");
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    delay(1000);
    Serial.println("\n\nStarting setup...");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize I2S
    setupI2S();
}

void loop() {
    Serial.println("\n--- Starting new recording cycle ---");
    
    // Test microphone with direct reading
    int32_t test_samples[64];  // Small buffer for quick testing
    size_t bytes_read = 0;
    
    // Read a small batch of samples
    esp_err_t read_result = i2s_read(I2S_NUM_0, test_samples, sizeof(test_samples), &bytes_read, 100);
    
    // Debug microphone input
    Serial.println("Microphone Test Results:");
    Serial.printf("Bytes read: %d\n", bytes_read);
    Serial.printf("Read result: %d\n", read_result);
    
    // Print first few raw samples
    Serial.println("Raw microphone samples:");
    for (int i = 0; i < 5 && i < (bytes_read / 4); i++) {
        Serial.printf("Sample[%d] = %ld (0x%08lX)\n", i, test_samples[i], test_samples[i]);
    }
    
    // Process and normalize samples
    int16_t processed_samples[64];
    for (int i = 0; i < (bytes_read / 4); i++) {
        // Extract the most significant 16 bits and normalize
        processed_samples[i] = test_samples[i] >> 16;
    }
    
    // Print processed samples
    Serial.println("Processed samples:");
    for (int i = 0; i < 5 && i < (bytes_read / 4); i++) {
        Serial.printf("Processed[%d] = %d\n", i, processed_samples[i]);
    }
    
    // Convert test samples to base64
    int test_encoded_len = Base64.encodedLength(bytes_read);
    char* test_encoded = new char[test_encoded_len + 1];
    Base64.encode(test_encoded, (char*)processed_samples, bytes_read);
    
    Serial.println("Test Base64 snippet:");
    Serial.println(String(test_encoded).substring(0, 20) + "...");
    
    // Clean up
    delete[] test_encoded;
    
    // If we're getting valid data, proceed with full recording
    if (bytes_read > 0 && !isAllZeros(test_samples, bytes_read / 4)) {
        Serial.println("\nProceeding with full recording...");
        String audioData = recordAndEncodeAudio();
        
        if (audioData.length() > 0) {
            Serial.println("Audio recorded successfully");
            sendAudioToServer(audioData);
        } else {
            Serial.println("Failed to record audio");
        }
    } else {
        Serial.println("\nMicrophone not detecting input - check connections");
        // Print pin states
        Serial.println("Pin States:");
        Serial.printf("SCK (GPIO%d): %d\n", I2S_SCK, digitalRead(I2S_SCK));
        Serial.printf("WS  (GPIO%d): %d\n", I2S_WS, digitalRead(I2S_WS));
        Serial.printf("SD  (GPIO%d): %d\n", I2S_SD, digitalRead(I2S_SD));
    }
    
    Serial.println("Waiting before next recording...");
    delay(5000);  // 5 second delay between recordings
}