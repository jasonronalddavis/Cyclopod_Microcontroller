#ifndef AUDIO_H
#define AUDIO_H

#include <driver/i2s.h>
#include <Arduino.h>
#include <I2S.h>

class Audio {
public:
    int wavDataSize;                // Total size of the audio data
    int dividedWavDataSize;         // Size of each divided chunk of data
    char paddedHeader[44];          // WAV header for the audio file
    char** wavData;                 // Array of pointers for WAV data chunks

    // Constructor
    Audio(MicType micType);

    // Destructor
    ~Audio();

    // Function to record audio data from I2S
    void Record(I2S &i2s);  // Update the signature to include the I2S reference

    // Function to create the WAV file header
    void CreateWavHeader(byte* header, int waveDataSize);
};

#endif