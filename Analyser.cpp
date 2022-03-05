
#include "aquila/global.h"
#include "aquila/source/generator/SineGenerator.h"
#include "aquila/tools/TextPlot.h"
#include "aquila/transform/FftFactory.h"

#include <iostream>
#include <vector>

using namespace std;

typedef signed short MY_TYPE;

struct FrequenciesData {
	double *buffer;
	unsigned long length;
};

struct LogAverages {
	char *buffer;
	unsigned long bufferBytes;
};

struct OctaveBand {
	unsigned int midFreq;
	unsigned int lowFreq;
	unsigned int highFreq;
};

class Analyser {
	bool first = true;
	vector<OctaveBand> octaveBands;
	vector<double> spectrumLog;

  public:
	Analyser() {
		calculateOctaveBands();
	}

	FrequenciesData getFrequencies(MY_TYPE *inputBuffer, unsigned long length) {
		double *fftInput = (double *)malloc(length * sizeof(double));

		for (unsigned long i = 0; i < length; i++) {
			fftInput[i] = inputBuffer[i];
		}

		auto fft = Aquila::FftFactory::getFft(length);
		Aquila::SpectrumType spectrum = fft->fft(fftInput);
		free(fftInput);

		size_t spectrumSize = spectrum.size() / 2; // Take only the first half, the second is a mirror of first (why?)
		// cout << "Fft spectrum size: " << spectrum.size() << ". Useful part size: " << spectrumSize << endl;

		double *fftOutput = (double *)malloc(spectrumSize * sizeof(double));
		for (size_t i = 0; i < spectrumSize; i++) {
			auto real = spectrum[i].real();
			auto imag = spectrum[i].imag();
			fftOutput[i] = sqrt(real * real + imag * imag);
		}

		FrequenciesData data;
		data.buffer = fftOutput;
		data.length = spectrumSize;

		return data;
	}

	LogAverages getOctaveBands(FrequenciesData *frData) {
		spectrumLog.assign(octaveBands.size(), 0);
		LogAverages averages;

		const unsigned int sampleRate = 44100;
		const unsigned int fftSize = frData->length * 2; // we take te original fft size

		double spectralLineLength = 1.0 * sampleRate / fftSize;
		if (first)
			cout << "Spectral line length: " << sampleRate << " / " << fftSize << " = " << spectralLineLength << endl;

		double *magnitude = frData->buffer;

		// *************************************************************

		for (size_t spectrumBin = 0; spectrumBin < spectrumLog.size(); spectrumBin++) {
			OctaveBand currentOctave = octaveBands[spectrumBin];
			size_t specLineLow = floor(currentOctave.lowFreq / spectralLineLength);
			size_t specLineHigh = ceil(currentOctave.highFreq / spectralLineLength); // TODO: cannot exceed spectrum

			if (specLineHigh - specLineLow == 1) {
				double part = (currentOctave.highFreq - currentOctave.lowFreq) / spectralLineLength;
				spectrumLog[spectrumBin] = magnitude[specLineLow] * part;
			} else {
				size_t specLineLowIn = specLineLow + 1;
				double lowPart = ((specLineLowIn * spectralLineLength) - currentOctave.lowFreq) / spectralLineLength;
				size_t specLineHighIn = specLineHigh - 1;
				double highPart = (currentOctave.highFreq - (specLineHighIn * spectralLineLength)) / spectralLineLength;

				spectrumLog[spectrumBin] = magnitude[specLineLow] * lowPart;
				spectrumLog[spectrumBin] += magnitude[specLineHighIn] * highPart;

				for (;specLineLowIn < specLineHighIn; specLineLowIn++) {
					spectrumLog[spectrumBin] += magnitude[specLineLowIn];
				}

				spectrumLog[spectrumBin] = spectrumLog[spectrumBin] / (specLineHighIn - specLineLow);
			}
		}

		//***************************************************************

		if (first) {
			cout << "Spectrum log: " << endl;
			for (size_t i = 0; i < spectrumLog.size(); i++) {
				cout << spectrumLog[i]<< " ";
			}
			cout << endl;
		}
		first = false;

		averages.buffer = (char *)spectrumLog.data();
		averages.bufferBytes = spectrumLog.size() * sizeof(double);

		return averages;
	}

	LogAverages getVisualization(MY_TYPE *inputBuffer, unsigned long length) {
		FrequenciesData frData = getFrequencies(inputBuffer, length);
		LogAverages averages = getOctaveBands(&frData);

		return averages;
	}

	void calculateOctaveBands(short N = 3, unsigned int lowwesMidFreq = 40) { // 1/N Octave bands
		// Middle frequency: f0
		// Lower freq. bound: f0 / (2^1/2)^1/N = f0 / 2^(1/2 * 1/N)
		// Upper freq. bound: f0 * (2^1/2)^1/N = f0 * 2^(1/2 * 1/N)

		unsigned int higgestMidFreq = 16000;
		double midFreqInterval = pow(2, 1.0 / N);
		double edgeRatio = pow(2, 0.5 / N);
		cout << "Mid frequency interval: " << midFreqInterval << endl;

		double currentFreq = (double)lowwesMidFreq;
		while (currentFreq <= higgestMidFreq) {
			OctaveBand ob;
			ob.midFreq = (unsigned int)currentFreq;
			ob.lowFreq = (unsigned int)currentFreq / edgeRatio;
			ob.highFreq = (unsigned int)currentFreq * edgeRatio;

			octaveBands.push_back(ob);
			currentFreq = midFreqInterval * currentFreq;
		}

		cout << "Octave bands: " << endl;
		for (size_t i = 0; i < octaveBands.size(); i++) {
			OctaveBand ob = octaveBands[i];
			cout << "Low: " << ob.lowFreq << " Mid: " << ob.midFreq << " High: " << ob.highFreq << endl;
		}
	}
};
