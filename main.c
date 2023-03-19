#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DLL_EXPORT __declspec(dllexport)

enum Result {
    Ok,
    Failed,
};

struct PlaybackParameters {

    double sampleRate;
    double tempo;

    int64_t projectTimeSamples;
    double projectTimeMusic;
    double barPositionMusic;

    int32_t numSamples;

};

DLL_EXPORT bool SingletonChecker() {
    return true;
}

float* buffer;
int bufferSize = 0;

DLL_EXPORT enum Result Initializer() {
    FILE *file = fopen("D:\\dstest\\test.wav", "rb");
    if(!file) return Failed;
    fseek(file, 40, SEEK_SET);
    uint32_t dataSize;
    fread(&dataSize, sizeof(uint32_t), 1, file);
    buffer = (float*)malloc(dataSize * sizeof(float));
    fread(buffer, dataSize * sizeof(float), 1, file);
    bufferSize = dataSize;
    fclose(file);
    return Ok;
}

DLL_EXPORT enum Result Terminator() {
    free(buffer);
    return Ok;
}

bool windowOpened = false;

DLL_EXPORT void WindowOpener() {
    if(!windowOpened) {
        MessageBoxA(NULL, "Window Opened", "DiffScope VSTi", MB_OK|MB_ICONINFORMATION);
        windowOpened = true;
    }
}

DLL_EXPORT void WindowCloser() {
    if(windowOpened) {
        MessageBoxA(NULL, "Window Closed", "DiffScope VSTi", MB_OK|MB_ICONINFORMATION);
        windowOpened = false;
    }
}

DLL_EXPORT enum Result PlaybackProcessor(const struct PlaybackParameters *playbackParameters,int32_t numOutputs, float *const *outputs) {
    if(playbackParameters->projectTimeSamples + playbackParameters->numSamples >= bufferSize) {
        return Failed;
    }
    for(int i = 0; i < numOutputs; i++) {
        memcpy(outputs[i], buffer + playbackParameters->projectTimeSamples, playbackParameters->numSamples * sizeof(float));
    }
    return Ok;
}

DLL_EXPORT enum Result StateChangedCallback(uint64_t size, const uint8_t *data) {
    FILE* file = fopen("D:\\dstest\\state", "wb");
    if(!file) return Failed;
    fwrite(data, sizeof(uint8_t), size, file);
    fclose(file);
    return Ok;
}

DLL_EXPORT enum Result StateWillSaveCallback(uint64_t* size, uint8_t** data) {
    FILE* file = fopen("D:\\dstest\\state", "rb");
    if(!file) return Failed;
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *data = (uint8_t*)malloc(*size);
    fread(*data, 1, *size, file);
    fclose(file);
    return Ok;
}

DLL_EXPORT void StateSavedAsyncCallback(uint8_t* dataToFree) {
    free(dataToFree);
}
