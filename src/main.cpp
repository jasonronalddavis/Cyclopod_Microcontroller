#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include <SPI.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <driver/i2s.h>

const char* ssid = "iPhone";
const char* password = "Barnes!!";
const char* serverUrl = "https://66b91a5dad1e3000084a8740--bright-dragon-1cbd6d.netlify.app/api/transcribe";

// I2S pins
#define I2S_WS  16    // Word Select (LRCLK)
#define I2S_SD  32    // Serial Data (DOUT)
#define I2S_SCK 17    // Serial Clock (BCLK)

// I2S configuration parameters
#define SAMPLE_RATE 400
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNELS I2S_CHANNEL_FMT_ONLY_LEFT
#define RECORD_TIME_SECONDS 1

void sendAudioToServer(const String& base64Audio) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        Serial.println("Connecting to server...");
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        // Construct the JSON payload
        String payload = "{\"audio\":\"" + base64Audio + "\"}";
        Serial.print("Payload size: ");
        Serial.println(payload.length());

        // Send POST request
        int httpResponseCode = http.POST(payload);
        yield();  // Let the system handle background tasks to avoid watchdog reset

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Transcription Response: ");
            Serial.println(response);
        } else {
            Serial.print("Error in sending request: ");
            Serial.println(httpResponseCode);
        }

        http.end(); // Free resources
        yield();  // Let the system handle background tasks
    } else {
        Serial.println("Wi-Fi Disconnected");
    }
}

void sendSingleCharacterToServer() {
    String singleCharBase64 = "QQ==";  // "A" in Base64
    Serial.println("Sending single character to server...");
    sendAudioToServer(singleCharBase64);
}

void setup() {
    Serial.begin(115200);
    delay(2000);  // Allow the serial port to stabilize
    Serial.println("Starting setup...");

    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");

    // Initialize I2S
    Serial.println("Initializing I2S...");
    i2s_config_t i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = CHANNELS,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK) {
        Serial.println("I2S driver installed successfully.");
    } else {
        Serial.println("Failed to install I2S driver.");
    }

    if (i2s_set_pin(I2S_NUM_0, &pin_config) == ESP_OK) {
        Serial.println("I2S pins set successfully.");
    } else {
        Serial.println("Failed to set I2S pins.");
    }

    i2s_zero_dma_buffer(I2S_NUM_0);
    Serial.println("I2S initialized.");
}

void loop() {
    Serial.print("Free heap memory: ");
    Serial.println(ESP.getFreeHeap());

    // Send single-character test to server for troubleshooting
    sendSingleCharacterToServer();

    delay(10000);  // Delay before the next attempt
}