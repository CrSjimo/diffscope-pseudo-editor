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

    union {
        int64_t projectTimeSamples;
        int64_t systemTimeMs;
    };
    double projectTimeMusic;
    double barPositionMusic;

    int32_t numSamples;

};

DLL_EXPORT bool SingletonChecker() {
    return true;
}

float* buffer;
int bufferSize = 0;

float* buffer2;
int buffer2Size = 0;

void (*setDirty)(bool) = NULL;

float* readFile(const char* fileName, int* siz) {
    FILE *file = fopen(fileName, "rb");
    if(!file) return NULL;
    fseek(file, 40, SEEK_SET);
    fread(siz, sizeof(uint32_t), 1, file);
    float* buf = (float*)malloc(*siz * sizeof(float));
    fread(buf, *siz * sizeof(float), 1, file);
    fclose(file);
    return buf;
}

DLL_EXPORT enum Result Initializer() {
    buffer = readFile("D:\\dstest\\test.wav", &bufferSize);
    if(!buffer) return Failed;
    buffer2 = readFile("D:\\dstest\\test2.wav", &buffer2Size);
    if(!buffer2) return Failed;
    return Ok;
}

DLL_EXPORT enum Result Terminator() {
    free(buffer);
    free(buffer2);
    return Ok;
}

bool windowOpened = false;

DLL_EXPORT void WindowOpener() {
    if(setDirty) setDirty(true);
    if(!windowOpened) {
        MessageBoxA(NULL, "Window Opened", "DiffScope Editor", MB_OK|MB_ICONINFORMATION);
        windowOpened = true;
    }
}

DLL_EXPORT void WindowCloser() {
    if(windowOpened) {
        MessageBoxA(NULL, "Window Closed", "DiffScope Editor", MB_OK|MB_ICONINFORMATION);
        windowOpened = false;
    }
}

int64_t lastPos = 0;

DLL_EXPORT enum Result PlaybackProcessor(const struct PlaybackParameters *playbackParameters, bool isPlaying, int32_t numOutputs, float *const *outputs) {
    if(isPlaying) {
        lastPos = 0;
        if(playbackParameters->projectTimeSamples + playbackParameters->numSamples >= bufferSize) {
            return Failed;
        }
        for(int i = 0; i < numOutputs; i++) {
            memcpy(outputs[i], buffer + playbackParameters->projectTimeSamples, playbackParameters->numSamples * sizeof(float));
        }
    } else if(windowOpened) {
        if(lastPos + playbackParameters->numSamples >= buffer2Size) {
            return Failed;
        }
        for(int i = 0; i < numOutputs; i++) {
            memcpy(outputs[i], buffer2 + lastPos, playbackParameters->numSamples * sizeof(float));
        }
        lastPos += playbackParameters->numSamples;
    } else {
        lastPos = 0;
        for(int i = 0; i < numOutputs; i++) {
            memset(outputs[i], 0, playbackParameters->numSamples * sizeof(float));
        }
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

DLL_EXPORT void DirtySetterBinder(void (*setDirty_)(bool)) {
    setDirty = setDirty_;
}
