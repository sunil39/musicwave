//
// Created by Ev on 7/16/2020.
//

#include <android/log.h>
#include "AudioEngine.h"
#include <thread>
#include <mutex>

// Double-buffering offers a good trade-off between latency and protection against glitches.
constexpr int32_t kBufferSizeInBursts = 2;

// Get data into our stream.
// We could write directly using AAudioStream_write(), but dataCallback() is better for low-latency
// apps; it's called from a high-priority thread.
aaudio_data_callback_result_t dataCallback(
        AAudioStream *stream,
        void *userData, // Pointer to our Oscillator object
        void *audioData,
        int32_t numFrames) {

    // render Oscillator to audio data array.
    // audioData is cast to a float because that's the format we expect in our render() method.
    ((Oscillator *) (userData))->render(static_cast<float *>(audioData), numFrames);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

// If our default audio device changes, restart the stream.
void errorCallback(AAudioStream *stream,
                   void *userData,
                   aaudio_result_t error){

    // Note that the callback cannot restart the audio stream directly.
    // Instead, to restart the stream we create a std::function which points
    // to AudioEngine::restart(), then invoke the function from a separate std::thread.
    //
    // One should never attempt to start or stop streams from within the error callback.
    // Doing so may result in the app crashing.
    // Always use a separate thread to perform stream operations.
    if (error == AAUDIO_ERROR_DISCONNECTED){
        std::function<void(void)> restartFunction = std::bind(&AudioEngine::restart,
                                                              static_cast<AudioEngine *>(userData));
        new std::thread(restartFunction);
    }
}

bool AudioEngine::start() {
    // Create audio stream.
    AAudioStreamBuilder *streamBuilder;
    AAudio_createStreamBuilder(&streamBuilder);

    // Set up audio stream.
    AAudioStreamBuilder_setFormat(streamBuilder, AAUDIO_FORMAT_PCM_FLOAT);
    AAudioStreamBuilder_setChannelCount(streamBuilder, 1);
    AAudioStreamBuilder_setPerformanceMode(streamBuilder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setDataCallback(streamBuilder, ::dataCallback, &oscillator_); // Callback setup
    AAudioStreamBuilder_setErrorCallback(streamBuilder, ::errorCallback, this);

    // Opens the stream.
    aaudio_result_t result = AAudioStreamBuilder_openStream(streamBuilder, &stream_);
    if (result != AAUDIO_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error opening stream %s",
                            AAudio_convertResultToText(result));
        return false;
    }

    // Retrieves the native sample rate of the stream for our oscillator.
    int32_t sampleRate = AAudioStream_getSampleRate(stream_);
    oscillator_.setSampleRate(sampleRate);

    // Sets the buffer size. Bursts are discrete amounts of data being written during each callback.
    AAudioStream_setBufferSizeInFrames(
            stream_, AAudioStream_getFramesPerBurst(stream_) * kBufferSizeInBursts);

    // Starts the stream (Consume audio and trigger data callbacks).
    // We keep the stream on and send zeros when the tone is off
    // (as opposed to stopping and starting the stream) to avoid warm-up latency.
    result = AAudioStream_requestStart(stream_);
    if (result != AAUDIO_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "AudioEngine", "Error starting stream %s",
                            AAudio_convertResultToText(result));
        return false;
    }

    AAudioStreamBuilder_delete(streamBuilder);
    return true;
}

// Since the restart function may be called from multiple threads
// (for instance, if we receive multiple disconnect events in quick succession),
// we protect the critical sections of code with a std::mutex.
void AudioEngine::restart(){

    static std::mutex restartingLock;
    if (restartingLock.try_lock()){
        stop();
        start();
        restartingLock.unlock();
    }
}

void AudioEngine::stop() {
    if (stream_ != nullptr) {
        AAudioStream_requestStop(stream_);
        AAudioStream_close(stream_);
    }
}

void AudioEngine::setToneOn(bool isToneOn) {
    oscillator_.setWaveOn(isToneOn);
}
