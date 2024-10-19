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
    // RIFF chunk descriptor
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';

    unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;  // File size minus the first 8 bytes
    header[4] = (byte)(fileSizeMinus8 & 0xFF);
    header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
    header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
    header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);

    // Format and subchunk descriptions
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';

    header[16] = 0x10;  // Subchunk1 size (16 for PCM)
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;

    // Audio format (1 for PCM)
    header[20] = 0x01;
    header[21] = 0x00;

    // Mono audio (1 channel)
    header[22] = 0x01;
    header[23] = 0x00;

    // Sample rate (16 kHz)
    header[24] = 0x80;  // 16000 in little-endian
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;

    // Byte rate (SampleRate * NumChannels * BitsPerSample / 8)
    unsigned int byteRate = 16000 * 1 * 16 / 8;  // 16-bit mono audio
    header[28] = (byte)(byteRate & 0xFF);
    header[29] = (byte)((byteRate >> 8) & 0xFF);
    header[30] = (byte)((byteRate >> 16) & 0xFF);
    header[31] = (byte)((byteRate >> 24) & 0xFF);

    // Block align (NumChannels * BitsPerSample / 8)
    header[32] = 0x02;
    header[33] = 0x00;

    // Bits per sample (16 bits)
    header[34] = 0x10;
    header[35] = 0x00;

    // Subchunk2 ID
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';

    // Subchunk2 size (NumSamples * NumChannels * BitsPerSample / 8)
    header[40] = (byte)(waveDataSize & 0xFF);
    header[41] = (byte)((waveDataSize >> 8) & 0xFF);
    header[42] = (byte)((waveDataSize >> 16) & 0xFF);
    header[43] = (byte)((waveDataSize >> 24) & 0xFF);
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


void Audio::EncodeToBase64() {
    // Cast paddedHeader to uint8_t* for base64 encoding
    content = base64::encode(reinterpret_cast<const uint8_t*>(paddedHeader), sizeof(paddedHeader));
    content.replace("\n", "");  // Remove any newline characters

    // Encode wavData chunks
    for (int j = 0; j < wavDataSize / dividedWavDataSize; ++j) {
        String encodedChunk = base64::encode(reinterpret_cast<const uint8_t*>(wavData[j]), dividedWavDataSize);
        encodedChunk.replace("\n", "");  // Remove newline characters
        content += encodedChunk;  // Append each chunk to the content
    }

    // Debugging output to ensure encoding is correct
    Serial.println("Full Encoded Audio Data:");
    Serial.println(content);
}
