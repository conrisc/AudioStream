#include "RtAudio.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "Analyser.cpp"
#include "CommunicationController.cpp"
#include "UdpClient.h"

/*
typedef char MY_TYPE;
#define FORMAT RTAUDIO_SINT8

typedef signed short MY_TYPE;
#define FORMAT RTAUDIO_SINT16

typedef S24 MY_TYPE;
#define FORMAT RTAUDIO_SINT24

typedef signed long MY_TYPE;
#define FORMAT RTAUDIO_SINT32

typedef float MY_TYPE;
#define FORMAT RTAUDIO_FLOAT32
*/

typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64

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
	std::cout << "\nuseage: N fs abf rbf <device> <channelOffset>\n";
	std::cout << "    where N = number of channels,\n";
	std::cout << "    fs = the sample rate (e.g. 44100),\n";
	std::cout << "    abf = buffer frames for analyse (e.g 2048),\n";
	std::cout << "    rbf = buffer frames for read (e.g 512),\n";
	std::cout << "    device = optional device to use (default = 0),\n";
	std::cout << "    channelOffset = an optional channel offset on the device (default = 0).\n\n";
	// exit(0);
}

struct InputData {
	vector<MY_TYPE> buffer;
	unsigned int bufferFrames;
	unsigned int bufferFramesRead;
	unsigned long bufferBytes;
	unsigned int channels;
};

CommunicationController communicationCtrl;
Analyser *analyser;
// UdpClient client("127.0.0.1", 8005); // FOR TESTING
// UdpClient arduino("192.168.1.20", 2390);
UdpClient esp("192.168.1.28", 1234);


int counter = 0;

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

	std::shift_left(iData -> buffer.begin(), iData -> buffer.end(), iData -> bufferFramesRead);

	// TODO: find better approach
	size_t j = 0;
	for (size_t i = iData -> bufferFrames - iData -> bufferFramesRead; i < iData -> bufferFrames; i++ ) { // assuming there is only one channel
		iData -> buffer[i] = bufferData[j++];
	}

	VisualizationData visualization = analyser -> getVisualization(iData -> buffer);
	// vector<PIXEL> scaledSpectrum = communicationCtrl.getLightMatrixColumnMsg(visualization.spectrum, visualization.max);
	vector<PIXEL> scaledSpectrum = communicationCtrl.getFrequencySpectrumMsg(visualization.spectrum, visualization.max);

	// client.send({(char*)scaledSpectrum.data(), sizeof(decltype(scaledSpectrum)::value_type) * scaledSpectrum.size()});
	esp.send({(char*)scaledSpectrum.data(), sizeof(PIXEL) * scaledSpectrum.size()});

	// 	return 2;
	return 0;
}

int main(int argc, char *argv[]) {
	unsigned int channels = 1, fs = 30000, bufferFrames = 4096, bufferFramesRead = 512, device = 0, offset = 0;

	usage();

	// Set led brightness
	std::cout<<"Led brightness [0-255]: ";
	unsigned short brightness;
	std::cin >> brightness;
	vector<PIXEL> brightnessMsg = communicationCtrl.getBrightnessMsg(brightness);
	esp.send({(char*)brightnessMsg.data(), sizeof(PIXEL) * brightnessMsg.size()});
	// Set led max brightness
	std::cout<<"Led max brightness [0-255]: ";
	std::cin >> brightness;
	brightnessMsg = communicationCtrl.getMaxBrightnessMsg(brightness);
	esp.send({(char*)brightnessMsg.data(), sizeof(PIXEL) * brightnessMsg.size()});
	//**

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
		bufferFramesRead = (unsigned int)atof(argv[4]);
	if (argc > 5)
		device = (unsigned int)atoi(argv[5]);
	if (argc > 6)
		offset = (unsigned int)atoi(argv[6]);


	std::cout <<"Sample rate: "<< fs <<" Hz"<<std::endl;
	std::cout <<"Buffer size for reading: "<< bufferFramesRead <<std::endl;
	std::cout <<"Callback 'input' will be called every "<< (double)bufferFramesRead/fs * 1000 << " miliseconds"<< std::endl;

	analyser = new Analyser(fs, bufferFrames);
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
	data.bufferFramesRead = bufferFramesRead;
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