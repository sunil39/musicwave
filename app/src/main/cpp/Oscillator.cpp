//
// Created by Ev on 7/16/2020.
//

#include "Oscillator.h"
#include <cmath>

#define TWO_PI (3.14159 * 2)
#define AMPLITUDE 0.3
#define FREQUENCY 440.0

void Oscillator::setSampleRate(int32_t sampleRate) {
    phaseIncrement_ = (TWO_PI * FREQUENCY) / (double) sampleRate;
}

/*Sets isWaveOn_, which is used in render() to determine whether to output sine wave or silence. */
void Oscillator::setWaveOn(bool isWaveOn) {
    isWaveOn_.store(isWaveOn);
}

/* Places floating point sine wave values into the audioData array with each call. */
void Oscillator::render(float *audioData, int32_t numFrames) {

    // We set phase_ to 0 each time the wave is switched off in order to always start
    // with a sample value of 0. If we didn't do this, we'd sometimes hear clicks and pops
    // each time the wave is switched on.
    if (!isWaveOn_.load()) phase_ = 0;

    // We may still hear a click when we stop touching the screen because the final sample
    // is much larger than the zeros that will follow after the wave is turned off.
    // The discontinuity causes the click.
    // One possible solution is is to apply a fade-out to the last buffer of data.

    for (int i = 0; i < numFrames; i++) {

        if (isWaveOn_.load()) {

            // Calculates the next sample value for the sine wave.
            audioData[i] = (float) (sin(phase_) * AMPLITUDE);

            // Increments the phase, handling wrap around.
            phase_ += phaseIncrement_;
            if (phase_ > TWO_PI) phase_ -= TWO_PI;

        } else {
            // Outputs silence by setting sample value to zero.
            audioData[i] = 0;
        }
    }
}
