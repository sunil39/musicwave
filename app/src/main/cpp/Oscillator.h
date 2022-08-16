//
// Created by Ev on 7/16/2020.
//

#ifndef WAVEMAKER_OSCILLATOR_H
#define WAVEMAKER_OSCILLATOR_H


#include <cstdint>
#include <atomic>

class Oscillator {
public:
    void setWaveOn(bool isWaveOn);
    void setSampleRate(int32_t sampleRate);
    void render(float *audioData, int32_t numFrames);

private:

    // Atomic data type read/write operations are guaranteed to happen in one instruction.
    // Using an atomic data type is recommended to sync data across threads.
    // e.g. Main thread calls isWaveOn() to set isWaveOn_,
    // while the audio thread calls render() to read the value.
    std::atomic<bool> isWaveOn_{false};
    double phase_ = 0.0;
    double phaseIncrement_ = 0.0;
};


#endif //WAVEMAKER_OSCILLATOR_H
