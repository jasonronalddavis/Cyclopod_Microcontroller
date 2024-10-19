#include "I2S.h"
#define SAMPLE_RATE (16000)
#define PIN_I2S_BCLK 17    // Bit Clock
#define PIN_I2S_LRC 16     // Left Right Clock (Word Select)
#define PIN_I2S_DIN 34     // Data In (SD)



I2S::I2S(MicType micType) {
    if (micType == ADMP441) {
        BITS_PER_SAMPLE = I2S_BITS_PER_SAMPLE_32BIT;  // ADMP441 outputs 24-bit, so we capture in 32-bit frames
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // I2S master mode, receiving data
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = BITS_PER_SAMPLE,  // 32-bit frames to capture 24-bit data
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // ADMP441 is mono, use only left channel
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // Use the updated standard I2S format
            .intr_alloc_flags = 0,
            .dma_buf_count = 8,  // Adjust buffer count to a reasonable value
            .dma_buf_len = 1024,  // Buffer length to handle large data chunks
            .use_apll = false,  // APLL not required
        };

        i2s_pin_config_t pin_config = {
            .bck_io_num = PIN_I2S_BCLK,  // Bit clock
            .ws_io_num = PIN_I2S_LRC,    // Word select (LRC)
            .data_out_num = I2S_PIN_NO_CHANGE,  // No data output
            .data_in_num = PIN_I2S_DIN,  // Data input (SD)
        };

        i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        i2s_set_pin(I2S_NUM_0, &pin_config);
        i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
    }
}


// Function to read from I2S
int I2S::Read(char* data, int numData) {
    size_t bytesRead;
    esp_err_t result = i2s_read(I2S_NUM_0, data, numData, &bytesRead, portMAX_DELAY);
    if (result == ESP_OK) {
        return bytesRead;  // Return the number of bytes read if successful
    } else {
        return -1;  // Return -1 if there was an error
    }
}

// Function to get the bits per sample
int I2S::GetBitPerSample() {
    return (int)BITS_PER_SAMPLE;
}
