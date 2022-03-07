
#include "aquila/global.h"
#include "aquila/source/generator/SineGenerator.h"
#include "aquila/tools/TextPlot.h"
#include "aquila/transform/FftFactory.h"

#include <iostream>
#include <vector>

using namespace std;

typedef signed short MY_TYPE;
typedef vector<double> FrequenciesData;

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

	FrequenciesData getFrequencies(vector<MY_TYPE> inputSignal) {
		unsigned int size = inputSignal.size();
		vector<double> fftInput(size);

		for (unsigned long i = 0; i < size; i++) {
			fftInput[i] = inputSignal[i];
		}

		auto fft = Aquila::FftFactory::getFft(size);
		Aquila::SpectrumType spectrum = fft->fft(fftInput.data());

		size_t spectrumSize = spectrum.size() / 2; // Take only the first half, the second is a mirror of first (why?)
		// cout << "Fft spectrum size: " << spectrum.size() << ". Useful part size: " << spectrumSize << endl;
		vector<double> fftOutput (spectrumSize);

		for (size_t i = 0; i < spectrumSize; i++) {
			auto real = spectrum[i].real();
			auto imag = spectrum[i].imag();
			fftOutput[i] = sqrt(real * real + imag * imag);
		}

		FrequenciesData frData = fftOutput;
		return frData;
	}

	LogAverages getOctaveBands(FrequenciesData frData) {
		spectrumLog.assign(octaveBands.size(), 0);
		LogAverages averages;

		const unsigned int sampleRate = 32000;
		const unsigned int fftSize = frData.size() * 2; // we take te original fft size

		double frequencyResolution = 1.0 * sampleRate / fftSize;
		if (first)
			cout << "Frequency resolution: " << sampleRate << " / " << fftSize << " = " << frequencyResolution << endl;

		// *************************************************************

		for (size_t spectrumBin = 0; spectrumBin < spectrumLog.size(); spectrumBin++) {
			OctaveBand currentOctave = octaveBands[spectrumBin];
			size_t specLineLow = round(currentOctave.lowFreq / frequencyResolution);
			size_t specLineHigh = round(currentOctave.highFreq / frequencyResolution); // Not part of current spectrum bin // TODO: cannot exceed spectrum

				for (size_t i = specLineLow;i < specLineHigh; i++) {
					spectrumLog[spectrumBin] += frData[i];
				}

				spectrumLog[spectrumBin] = spectrumLog[spectrumBin] / (specLineHigh - specLineLow);
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

	LogAverages getVisualization(vector<MY_TYPE> inputSignal) {
		FrequenciesData frData = getFrequencies(inputSignal);
		LogAverages averages = getOctaveBands(frData);

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
