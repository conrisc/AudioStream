#include "RtAudio.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "Analyser.cpp"
#include "UdpClient.h"

/*
typedef char MY_TYPE;
#define FORMAT RTAUDIO_SINT8
*/

typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16

/*
typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24

typedef signed long MY_TYPE;
#define FORMAT RTAUDIO_SINT32

typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32

typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64
*/

// Platform-dependent sleep routines.
#if defined(WIN32)
#include <windows.h>
#define SLEEP(milliseconds) Sleep((DWORD)milliseconds)
#else // Unix variants
#include <unistd.h>
#define SLEEP(milliseconds) usleep((unsigned long)(milliseconds * 1000.0))
#endif

void usage(void) {
	// argument specifications
	std::cout << "\nuseage: record N fs bf <device> <channelOffset>\n";
	std::cout << "    where N = number of channels,\n";
	std::cout << "    fs = the sample rate (e.g. 44100),\n";
	std::cout << "    bf = buffer frames (e.g 2048),\n";
	std::cout << "    device = optional device to use (default = 0),\n";
	std::cout << "    channelOffset = an optional channel offset on the device (default = 0).\n\n";
	// exit(0);
}

struct InputData {
	vector<MY_TYPE> buffer;
	unsigned int bufferFrames;
	unsigned long bufferBytes;
	unsigned int channels;
};

UdpClient client;
Analyser analyser;

int counter = 0;
unsigned int bufferFramesRead = 512;

// Interleaved buffers
int input(
	void * /*outputBuffer*/,
	void *inputBuffer,
	unsigned int nBufferFrames,
	double /*streamTime*/,
	RtAudioStreamStatus /*status*/,
	void *data
) {
	InputData *iData = (InputData *)data;
	MY_TYPE *bufferData = (MY_TYPE *)inputBuffer;
	// unsigned long length = nBufferFrames * iData->channels;

	std::shift_left(iData -> buffer.begin(), iData -> buffer.end(), bufferFramesRead);

	// TODO: find better approach
	size_t j = 0;
	for (size_t i = iData -> bufferFrames - bufferFramesRead; i < iData -> bufferFrames; i++ ) { // assuming there is only one channel
		iData -> buffer[i] = bufferData[j++];
	}

	LogAverages visualization = analyser.getVisualization(iData -> buffer);
	client.send(visualization.buffer, visualization.bufferBytes);

	// 	return 2;
	return 0;
}

int main(int argc, char *argv[]) {
	unsigned int channels = 1, fs = 30000, bufferFrames = 2048, device = 0, offset = 0;

	usage();

	RtAudio adc;
	if (adc.getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		exit(1);
	}

	if (argc > 1)
		channels = (unsigned int)atoi(argv[1]);
	if (argc > 2)
		fs = (unsigned int)atoi(argv[2]);
	if (argc > 3)
		bufferFrames = (unsigned int)atof(argv[3]);
	if (argc > 4)
		device = (unsigned int)atoi(argv[4]);
	if (argc > 5)
		offset = (unsigned int)atoi(argv[5]);

	// Let RtAudio print messages to stderr.
	adc.showWarnings(true);

	// Set our stream parameters for input only.
	RtAudio::StreamParameters iParams;
	if (device == 0)
		iParams.deviceId = adc.getDefaultOutputDevice();
	// iParams.deviceId = adc.getDefaultInputDevice();
	else
		iParams.deviceId = device;

	iParams.nChannels = channels;
	iParams.firstChannel = offset;

	InputData data;


	try {
		adc.openStream(NULL, &iParams, FORMAT, fs, &bufferFramesRead, &input, (void *)&data);
	} catch (RtAudioError &e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		goto cleanup;
	}

	data.buffer.assign(bufferFrames, 0); // assuming there is only one channel
	data.bufferFrames = bufferFrames;
	data.bufferBytes = bufferFrames * channels * sizeof(MY_TYPE);
	data.channels = channels;

	try {
		adc.startStream();
	} catch (RtAudioError &e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		goto cleanup;
	}

	while (adc.isStreamRunning()) {
		SLEEP(5000); // wake every 100 ms to check if we're done
	}

cleanup:
	if (adc.isStreamOpen())
		adc.closeStream();

	return 0;
}