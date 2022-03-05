#include "RtAudio.h"

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
	// Error function in case of incorrect command-line
	// argument specifications
	std::cout << "\nuseage: record N fs <duration> <device> <channelOffset>\n";
	std::cout << "    where N = number of channels,\n";
	std::cout << "    fs = the sample rate,\n";
	std::cout << "    duration = optional time in seconds to record (default = "
	             "8.0),\n";
	std::cout << "    device = optional device to use (default = 0),\n";
	std::cout << "    and channelOffset = an optional channel offset on the "
	             "device (default = 0).\n\n";
	// exit(0);
}

struct InputData {
	// MY_TYPE *buffer;
	// unsigned long bufferBytes;
	// unsigned long totalFrames;
	unsigned long frameCounter;
	unsigned int channels;
};

UdpClient client;
Analyser analyser;

int counter = 0;

// Interleaved buffers
int input(void * /*outputBuffer*/, void *inputBuffer,
          unsigned int nBufferFrames, double /*streamTime*/,
          RtAudioStreamStatus /*status*/, void *data) {
	InputData *iData = (InputData *)data;
	MY_TYPE *bufferData = (MY_TYPE *)inputBuffer;
	// unsigned long bufferBytes = nBufferFrames * iData->channels *
	// sizeof(MY_TYPE);
	unsigned long length = nBufferFrames * iData->channels;

	// if (counter++ % 10 == 0) {
		// std::cout << "Data: ";
		// for (unsigned int i = 0; i < 100; i++) {
		// 	std::cout << bufferData[i] << ',';
		// }
		// std::cout << std::endl;

		// std::cout << nBufferFrames << std::endl;
		// char *lol = (char*)"Fajnie"; // bad practice
		LogAverages visualization = analyser.getVisualization(bufferData, length);
		client.send(visualization.buffer, visualization.bufferBytes);
		// double tmp[nBufferFrames];
		// for (unsigned int i = 0; i < nBufferFrames; i++) {
		// 	tmp[i] = bufferData[i];
		// }
		// auto fft = Aquila::FftFactory::getFft(nBufferFrames);
		// Aquila::SpectrumType spectrum = fft->fft(tmp);
		// Aquila::TextPlot plot("Spectrum");
		// plot.plotSpectrum(spectrum);
	// }

	// Simply copy the data to our allocated buffer.
	unsigned int frames = nBufferFrames;
	// if (iData->frameCounter + nBufferFrames > iData->totalFrames) {
	// frames = iData->totalFrames - iData->frameCounter;
	// iData->bufferBytes = frames * iData->channels * sizeof(MY_TYPE);
	// }

	// unsigned long offset = iData->frameCounter * iData->channels;
	// memcpy(iData->buffer + offset, inputBuffer, iData->bufferBytes);
	iData->frameCounter += frames;
	// std::cout<<"Frames: "<<iData->frameCounter<<std::endl;

	// if (iData->frameCounter >= iData->totalFrames)
	// 	return 2;

	return 0;
}

int main(int argc, char *argv[]) {
	unsigned int channels = 2, fs = 44100, bufferFrames, device = 0, offset = 0;
	double time = 8.0;
	// FILE *fd;

	// minimal command-line checking
	// if (argc < 3 || argc > 6)
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
		time = (double)atof(argv[3]);
	if (argc > 4)
		device = (unsigned int)atoi(argv[4]);
	if (argc > 5)
		offset = (unsigned int)atoi(argv[5]);

	// Let RtAudio print messages to stderr.
	adc.showWarnings(true);

	// Set our stream parameters for input only.
	bufferFrames = 2048;
	RtAudio::StreamParameters iParams;
	if (device == 0)
		iParams.deviceId = adc.getDefaultOutputDevice();
	// iParams.deviceId = adc.getDefaultInputDevice();
	else
		iParams.deviceId = device;
	iParams.nChannels = channels;
	iParams.firstChannel = offset;

	InputData data;
	// data.buffer = 0;
	try {
		adc.openStream(NULL, &iParams, FORMAT, fs, &bufferFrames, &input,
		               (void *)&data);
	} catch (RtAudioError &e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		goto cleanup;
	}

	// data.bufferBytes = bufferFrames * channels * sizeof(MY_TYPE);
	// data.totalFrames = (unsigned long)(fs * time);
	data.frameCounter = 0;
	data.channels = channels;
	// unsigned long totalBytes;
	// totalBytes = data.totalFrames * channels * sizeof(MY_TYPE);

	// Allocate the entire data buffer before starting stream.
	// data.buffer = (MY_TYPE *)malloc(totalBytes);
	// if (data.buffer == 0) {
	// 	std::cout << "Memory allocation error ... quitting!\n";
	// 	goto cleanup;
	// }

	try {
		adc.startStream();
	} catch (RtAudioError &e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		goto cleanup;
	}

	std::cout << "\nRecording for " << time
	          << " seconds ... writing file 'record.raw' (buffer frames = "
	          << bufferFrames << ")." << std::endl;

	while (adc.isStreamRunning()) {
		SLEEP(5000); // wake every 100 ms to check if we're done
	}

cleanup:
	if (adc.isStreamOpen())
		adc.closeStream();
	// if (data.buffer)
	// 	free(data.buffer);

	return 0;
}