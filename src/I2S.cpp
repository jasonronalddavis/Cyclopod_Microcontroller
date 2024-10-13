#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 17    // Bit Clock
#define PIN_I2S_LRC 16     // Left Right Clock (Word Select)
#define PIN_I2S_DIN 34     // Data In (SD)

// Constructor
I2S::I2S(MicType micType) {
    BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;  // Set to 32-bit to capture 24-bit data properly

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // Master mode, receiving data
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,  // 32-bit frames for 24-bit data
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Stereo (left and right channels)
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // Standard I2S format
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 1024,  // Increased buffer size to handle more data
        .use_apll = false,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,  // No MCLK pin
        .bck_io_num = PIN_I2S_BCLK,       // Bit clock
        .ws_io_num = PIN_I2S_LRC,         // Word select (LRCK)
        .data_out_num = I2S_PIN_NO_CHANGE, // No data output
        .data_in_num = PIN_I2S_DIN        // Data input (SD)
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_STEREO);
}

// Function to read from I2S
int I2S::Read(char* data, int numData) {
    size_t bytesRead;
    esp_err_t result = i2s_read(I2S_NUM_0, data, numData, &bytesRead, portMAX_DELAY);
    if (result == ESP_OK) {
        return bytesRead;  // Return the number of bytes read if successful.
    } else {
        return -1;  // Return -1 if there was an error.
    }
}

// Function to get the bits per sample
int I2S::GetBitPerSample() {
    return (int)BITS_PER_SAMPLE;
}
