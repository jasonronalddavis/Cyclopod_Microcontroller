#include <WiFi.h>
#include <HTTPClient.h>
#include <Base64.h>
#include <driver/i2s.h>



// WiFi credentials
const char* ssid = "iPhone";
const char* password = "Barnes!!";
const char* serverUrl = "https://bright-dragon-1cbd6d.netlify.app/.netlify/functions/transcribe";

// Sample I2S pins for ESP32-S3
#define I2S_WS 42    // Word Select (LRCL) pin
#define I2S_SD 41    // Serial Data (DOUT) pin 
#define I2S_SCK 40   // Serial Clock (BCLK) pin

// Configuration
#define SAMPLE_RATE 16000
#define SAMPLE_BUFFER_SIZE 512
#define RECORDING_TIME_MS 2000  // 2 seconds per recording
#define SAMPLES_PER_RECORDING (SAMPLE_RATE * RECORDING_TIME_MS / 1000)

// Debug options
#define DEBUG_I2S_SETUP true
#define DEBUG_AUDIO_SAMPLES true
#define DEBUG_AUDIO_STATS true
#define DEBUG_PIN_STATUS true
#define DEBUG_RECORDING_PROGRESS true

// Buffer for audio samples and processing  
int32_t samples[SAMPLE_BUFFER_SIZE];
int16_t recording_buffer[SAMPLES_PER_RECORDING];

// Function prototypes
void setupWiFi();
void setupI2S();
void recordAudio();
void sendAudioToServer();
void debugPinStatus();
void printAudioStats(int16_t* buffer, size_t size);
void printSampleHistogram(int16_t* buffer, size_t size);
size_t encode_base64(const unsigned char* input, size_t length, char* output);

// Global state
bool recording_ready = false;
int recording_index = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);  // Allow serial to stabilize
  
  Serial.println("\n\n==============================================");
  Serial.println("ESP32 I2S Microphone Debugging");
  Serial.println("==============================================");
  
  // Debug system info
  Serial.println("\n--- SYSTEM INFO ---");
  Serial.printf("ESP32 SDK: %s\n", ESP.getSdkVersion());
  Serial.printf("CPU Freq: %dMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  
  if (DEBUG_PIN_STATUS) {
    Serial.println("\n--- INITIAL PIN STATUS ---");
    debugPinStatus();
  }
  
  // Connect to WiFi
  setupWiFi();
  
  // Setup I2S
  setupI2S();
  
  Serial.println("\nSystem ready - debug recording will start automatically");
}

void debugPinStatus() {
  // Check pin modes and voltages
  // Configure pins temporarily as inputs to read values
  pinMode(I2S_SCK, INPUT);
  pinMode(I2S_WS, INPUT);
  pinMode(I2S_SD, INPUT);
  
  delay(10); // Allow pins to settle
  
  // Read digital values
  int sck_value = digitalRead(I2S_SCK);
  int ws_value = digitalRead(I2S_WS);
  int sd_value = digitalRead(I2S_SD);
  
  // Read analog values (approximate voltage)
  int sck_analog = analogRead(I2S_SCK);
  int ws_analog = analogRead(I2S_WS);
  int sd_analog = analogRead(I2S_SD);
  
  // Convert to millivolts (ESP32 ADC is 12-bit, approximately 0-4095 for 0-3.3V)
  float sck_mv = (sck_analog / 4095.0) * 3300.0;
  float ws_mv = (ws_analog / 4095.0) * 3300.0;
  float sd_mv = (sd_analog / 4095.0) * 3300.0;
  
  Serial.println("I2S Pin Status:");
  Serial.printf("SCK (GPIO%d): Digital=%d, Analog=%.0fmV\n", I2S_SCK, sck_value, sck_mv);
  Serial.printf("WS  (GPIO%d): Digital=%d, Analog=%.0fmV\n", I2S_WS, ws_value, ws_mv);
  Serial.printf("SD  (GPIO%d): Digital=%d, Analog=%.0fmV\n", I2S_SD, sd_value, sd_mv);
  
  // Reset pins for I2S use
  pinMode(I2S_SCK, OUTPUT);
  pinMode(I2S_WS, OUTPUT);
  pinMode(I2S_SD, INPUT);
}

void setupWiFi() {
  Serial.println("\n--- WIFI SETUP ---");
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("Signal strength: %ddBm\n", WiFi.RSSI());
  } else {
    Serial.println("\nFailed to connect to WiFi, but continuing...");
  }
}

void setupI2S() {
  Serial.println("\n--- I2S SETUP ---");
  
  if (DEBUG_I2S_SETUP) {
    Serial.printf("Configuring I2S with:\n");
    Serial.printf("- Sample rate: %d Hz\n", SAMPLE_RATE);
    Serial.printf("- Bit depth: 32-bit\n");
    Serial.printf("- Channel: Mono (Left)\n");
    Serial.printf("- SCK pin: GPIO%d\n", I2S_SCK);
    Serial.printf("- WS pin: GPIO%d\n", I2S_WS);
    Serial.printf("- SD pin: GPIO%d\n", I2S_SD);
  }
  // I2S configuration
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
  
  // Install I2S driver
  Serial.println("Installing I2S driver...");
  esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (result != ESP_OK) {
    Serial.printf("Error installing I2S driver: %d\n", result);
    return;
  }
  Serial.println("I2S driver installed successfully");
  
  // Set I2S pins
  Serial.println("Setting I2S pins...");
  result = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (result != ESP_OK) {
    Serial.printf("Error setting I2S pins: %d\n", result);
    return;
  }
  Serial.println("I2S pins set successfully");
  
  // Print pin status after I2S initialization if debugging enabled
  if (DEBUG_PIN_STATUS) {
    Serial.println("\n--- PIN STATUS AFTER I2S INIT ---");
    debugPinStatus();
  }
  
  // Try a quick read to verify I2S is working
  Serial.println("\nPerforming initial I2S read test...");
  size_t bytes_read = 0;
  result = i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytes_read, 100);
  
  if (result == ESP_OK && bytes_read > 0) {
    Serial.printf("I2S read test successful! Read %d bytes\n", bytes_read);
    
    // Print first few samples
    int samples_read = bytes_read / sizeof(int32_t);
    Serial.println("First 5 samples:");
    for (int i = 0; i < 5 && i < samples_read; i++) {
      int16_t sample = samples[i] >> 16;
      Serial.printf("Sample[%d] = %d (0x%04X)\n", i, sample, (uint16_t)sample);
    }
  } else {
    Serial.printf("I2S read test failed: error=%d, bytes_read=%d\n", result, bytes_read);
  }
  
  Serial.println("I2S initialization complete");
}

void recordAudio() {
  Serial.println("\n--- RECORDING AUDIO ---");
  
  // Reset the buffer
  memset(recording_buffer, 0, sizeof(recording_buffer));
  recording_index = 0;
  
  // Print a progress bar
  if (DEBUG_RECORDING_PROGRESS) {
    Serial.print("[");
    for (int i = 0; i < 20; i++) Serial.print(" ");
    Serial.print("]");
  }
  
  // Record until buffer is full
  unsigned long start_time = millis();
  int last_progress = 0;
  int batch_count = 0;
  
  while (recording_index < SAMPLES_PER_RECORDING) {
    // Yield to allow other processes
    delay(1);
    
    // Read a batch of samples
    size_t bytes_read = 0;
    esp_err_t result = i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytes_read, 100);
    
    if (result != ESP_OK) {
      Serial.printf("I2S read error: %d\n", result);
      continue;
    }
    
    int samples_read = bytes_read / sizeof(int32_t);
    batch_count++;
    
    // Process samples
    for (int i = 0; i < samples_read && recording_index < SAMPLES_PER_RECORDING; i++) {
      // Extract the 16 most significant bits
      recording_buffer[recording_index++] = samples[i] >> 16;
    }
    
    // Update progress bar
    if (DEBUG_RECORDING_PROGRESS) {
      int progress = (recording_index * 20) / SAMPLES_PER_RECORDING;
      if (progress > last_progress) {
        Serial.print("\r[");
        for (int i = 0; i < progress; i++) Serial.print("=");
        for (int i = progress; i < 20; i++) Serial.print(" ");
        Serial.print("]");
        last_progress = progress;
      }
    }
    
    // Debug some samples periodically
    if (DEBUG_AUDIO_SAMPLES && batch_count % 10 == 0) {
      Serial.printf("\nBatch %d samples: ", batch_count);
      for (int i = 0; i < 3 && i < samples_read; i++) {
        Serial.printf("%d ", samples[i] >> 16);
      }
    }
  }
  
  float duration = (millis() - start_time) / 1000.0;
  Serial.println("\nRecording complete!");
  Serial.printf("Duration: %.1f seconds, %d samples, %d batches\n", duration, recording_index, batch_count);
  
  // Print detailed audio stats
  if (DEBUG_AUDIO_STATS) {
    printAudioStats(recording_buffer, recording_index);
    printSampleHistogram(recording_buffer, recording_index);
  }
  
  recording_ready = true;
}

void printAudioStats(int16_t* buffer, size_t size) {
  // Skip if buffer is empty
  if (size == 0) {
    Serial.println("No audio data to analyze");
    return;
  }
  
  // Calculate stats
  int16_t min_sample = 32767;
  int16_t max_sample = -32768;
  long sum = 0;
  long sum_abs = 0;
  
  for (size_t i = 0; i < size; i++) {
    int16_t sample = buffer[i];
    
    // Update min/max
    if (sample < min_sample) min_sample = sample;
    if (sample > max_sample) max_sample = sample;
    
    // Update sums
    sum += sample;
    sum_abs += abs(sample);
  }
  
  // Calculate averages
  float mean = (float)sum / size;
  float mean_abs = (float)sum_abs / size;
  
  // Calculate RMS (root mean square)
  float sum_squared = 0;
  for (size_t i = 0; i < size; i++) {
    int16_t sample = buffer[i];
    float diff = sample - mean;
    sum_squared += diff * diff;
  }
  float rms = sqrt(sum_squared / size);
  
  // Calculate zero crossings
  int zero_crossings = 0;
  for (size_t i = 1; i < size; i++) {
    if ((buffer[i - 1] >= 0 && buffer[i] < 0) || 
        (buffer[i - 1] < 0 && buffer[i] >= 0)) {
      zero_crossings++;
    }
  }
  
  // Print results
  Serial.println("\n--- AUDIO STATISTICS ---");
  Serial.printf("Min: %d, Max: %d, Range: %d\n", min_sample, max_sample, max_sample - min_sample);
  Serial.printf("Mean: %.2f, Abs Mean: %.2f, RMS: %.2f\n", mean, mean_abs, rms);
  Serial.printf("Zero crossings: %d (%.1f Hz)\n", zero_crossings, zero_crossings * SAMPLE_RATE / (2.0 * size));
  
  // Signal quality assessment
  Serial.println("\n--- SIGNAL QUALITY ASSESSMENT ---");
  if (max_sample - min_sample < 100) {
    Serial.println("WARNING: Very low amplitude! Check microphone connections.");
  } else if (max_sample - min_sample < 1000) {
    Serial.println("Low amplitude signal - microphone may not be picking up sound well.");
  } else if (max_sample - min_sample > 30000) {
    Serial.println("High amplitude signal - good audio level detected!");
  } else {
    Serial.println("Medium amplitude signal - acceptable audio level.");
  }
  
  if (abs(mean) > 1000) {
    Serial.println("WARNING: High DC offset! Microphone may not be properly grounded.");
  }
  
  if (zero_crossings < size / 100) {
    Serial.println("WARNING: Few zero crossings detected - may be DC offset or no audio.");
  }
}

void printSampleHistogram(int16_t* buffer, size_t size) {
  if (size == 0) return;
  
  // Define histogram bins
  const int NUM_BINS = 10;
  int bins[NUM_BINS] = {0};
  
  // Find min/max for scaling
  int16_t min_val = 32767;
  int16_t max_val = -32768;
  
  for (size_t i = 0; i < size; i++) {
    if (buffer[i] < min_val) min_val = buffer[i];
    if (buffer[i] > max_val) max_val = buffer[i];
  }
  
  // Ensure we don't divide by zero
  if (max_val == min_val) {
    max_val = min_val + 1;
  }
  
  // Count samples in each bin
  for (size_t i = 0; i < size; i++) {
    int bin = ((int)buffer[i] - min_val) * NUM_BINS / (max_val - min_val);
    if (bin >= NUM_BINS) bin = NUM_BINS - 1;
    if (bin < 0) bin = 0;
    bins[bin]++;
  }
  
  // Find max bin height for scaling
  int max_bin_height = 0;
  for (int i = 0; i < NUM_BINS; i++) {
    if (bins[i] > max_bin_height) max_bin_height = bins[i];
  }
  
  // Print histogram
  Serial.println("\n--- AUDIO HISTOGRAM ---");
  for (int i = 0; i < NUM_BINS; i++) {
    int bar_height = max_bin_height > 0 ? (bins[i] * 20 / max_bin_height) : 0;
    Serial.printf("[%5d-%5d]: %5d ", 
                 min_val + i * (max_val - min_val) / NUM_BINS,
                 min_val + (i + 1) * (max_val - min_val) / NUM_BINS - 1,
                 bins[i]);
    
    for (int j = 0; j < bar_height; j++) {
      Serial.print("#");
    }
    Serial.println();
  }
}

void sendAudioToServer() {
  if (!recording_ready) {
    Serial.println("No recording to send");
    return;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - can't send audio");
    return;
  }
  
  // Check if there's actual audio content
  int16_t min_sample = 32767;
  int16_t max_sample = -32768;
  
  for (int i = 0; i < SAMPLES_PER_RECORDING; i++) {
    if (recording_buffer[i] < min_sample) min_sample = recording_buffer[i];
    if (recording_buffer[i] > max_sample) max_sample = recording_buffer[i];
  }
  
  if (max_sample - min_sample < 100) {
    Serial.println("No significant audio detected - skipping");
    return;
  }
  
  Serial.println("\n--- SENDING AUDIO TO SERVER ---");
  
  // Base64 encode the audio - do this in chunks to avoid memory issues
  const int CHUNK_SIZE = 1024; // Process 1KB at a time
  const int AUDIO_SIZE = SAMPLES_PER_RECORDING * sizeof(int16_t);
  const int CHUNKS = (AUDIO_SIZE + CHUNK_SIZE - 1) / CHUNK_SIZE;
  
  // Prepare HTTP client
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  // Start building the JSON payload
  String payload = "{\"audio\":\"";
  
  // Base64 encode in chunks
  char chunk_buffer[CHUNK_SIZE];
  char encoded_chunk[CHUNK_SIZE * 2]; // Base64 encoding expands size
  
  for (int chunk = 0; chunk < CHUNKS; chunk++) {
    // Calculate chunk boundaries
    int offset = chunk * CHUNK_SIZE;
    int size = min(CHUNK_SIZE, AUDIO_SIZE - offset);
    
    // Copy chunk to buffer
    memcpy(chunk_buffer, ((char*)recording_buffer) + offset, size);
    
    // Encode the chunk and append to payload
    size_t encoded_len = encode_base64((unsigned char*)chunk_buffer, size, encoded_chunk);
    encoded_chunk[encoded_len] = '\0';
    
    payload += encoded_chunk;
    
    // Allow other processes
    delay(1);
    
    // Show progress
    Serial.printf("Encoding chunk %d/%d\n", chunk + 1, CHUNKS);
  }
  
  // Complete the JSON payload
  payload += "\", \"format\":\"pcm\"}";
  
  Serial.printf("Payload size: %d bytes\n", payload.length());
  Serial.println("Sending request...");
  
  // Send the request
  unsigned long start_time = millis();
  int httpResponseCode = http.POST(payload);
  unsigned long elapsed = millis() - start_time;
  
  // Process response
  if (httpResponseCode > 0) {
    Serial.printf("HTTP response code: %d (took %d ms)\n", httpResponseCode, elapsed);
    
    String response = http.getString();
    Serial.println("Server response:");
    Serial.println(response);
    
    // Extract the transcription
    if (response.indexOf("decodedText") > -1) {
      int start_pos = response.indexOf("decodedText") + 13;
      if (response.charAt(start_pos) == '"') start_pos++;
      
      int end_pos = response.indexOf("\"", start_pos);
      if (end_pos == -1) end_pos = response.indexOf("}", start_pos);
      
      if (start_pos > 13 && end_pos > start_pos) {
        String transcription = response.substring(start_pos, end_pos);
        
        Serial.println("\n========================");
        Serial.println("TRANSCRIPTION: " + transcription);
        Serial.println("========================\n");
      }
    }
  } else {
    Serial.printf("HTTP error: %d\n", httpResponseCode);
  }
  
  http.end();
  recording_ready = false;
}

// Simple Base64 encoder to avoid library issues
size_t encode_base64(const unsigned char* input, size_t length, char* output) {
  const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
  
  size_t i = 0, j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];
  size_t output_length = 0;
  
  while (length--) {
    char_array_3[i++] = *(input++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      
      for (i = 0; i < 4; i++) {
        output[output_length++] = base64_chars[char_array_4[i]];
      }
      i = 0;
    }
  }
  
  if (i) {
    for (j = i; j < 3; j++) {
      char_array_3[j] = '\0';
    }
    
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    
    for (j = 0; j < i + 1; j++) {
      output[output_length++] = base64_chars[char_array_4[j]];
    }
    
    while (i++ < 3) {
      output[output_length++] = '=';
    }
  }
  
  return output_length;
}

void loop() {
  static enum {
    STATE_IDLE,
    STATE_RECORDING,
    STATE_SENDING
  } state = STATE_IDLE;
  
  static unsigned long state_time = 0;
  static int cycle_count = 0;
  
  switch (state) {
    case STATE_IDLE:
      if (millis() - state_time > 3000) { // Wait 3 seconds between cycles
        Serial.printf("\n=== Starting Cycle #%d ===\n", ++cycle_count);
        
        // Debug pin status periodically
        if (DEBUG_PIN_STATUS && cycle_count % 3 == 0) {
          Serial.println("\n--- PIN STATUS BEFORE RECORDING ---");
          debugPinStatus();
        }
        
        state = STATE_RECORDING;
        state_time = millis();
      }
      break;
      
    case STATE_RECORDING:
      recordAudio();
      state = STATE_SENDING;
      state_time = millis();
      break;
      
    case STATE_SENDING:
      sendAudioToServer();
      state = STATE_IDLE;
      state_time = millis();
      
      // Print a memory status update
      Serial.printf("\nHeap status - Free: %d, Min: %d\n", 
                   ESP.getFreeHeap(), ESP.getMinFreeHeap());
      break;
  }
  
  // Important: Allow other tasks to run
  delay(10);
}
