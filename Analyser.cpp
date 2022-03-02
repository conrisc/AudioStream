
#include "aquila/global.h"
#include "aquila/source/generator/SineGenerator.h"
#include "aquila/tools/TextPlot.h"
#include "aquila/transform/FftFactory.h"

#include <iostream>

using namespace std;

typedef signed short MY_TYPE;

struct FrequenciesData {
	char *buffer;
	unsigned long bufferBytes;
};

class Analyser {
  public:
	Analyser() {}

	FrequenciesData getFrequencies(MY_TYPE *inputBuffer, unsigned long length) {
		double *fftInput = (double *)malloc(length * sizeof(double));

		for (unsigned long i = 0; i < length; i++) {
			fftInput[i] = inputBuffer[i];
		}

		auto fft = Aquila::FftFactory::getFft(length);
		Aquila::SpectrumType spectrum = fft->fft(fftInput);
		free(fftInput);

		size_t spectrumSize = spectrum.size();
		cout << "fft spectrum size: " << spectrumSize << endl;

		double *fftOutput = (double *)malloc(spectrumSize * sizeof(double));
		for (size_t i = 0; i < spectrumSize; i++)
			fftOutput[i] = spectrum[i].real();

		FrequenciesData data;
		data.buffer = (char *)fftOutput;
		data.bufferBytes = spectrumSize * sizeof(double);

		return data;
	}
};
