
#include "aquila/global.h"
#include "aquila/source/generator/SineGenerator.h"
#include "aquila/source/window/HannWindow.h"
// #include "aquila/source/window/RectangularWindow.h"
#include "aquila/tools/TextPlot.h"
#include "aquila/transform/FftFactory.h"

#include <iostream>
#include <math.h>
#include <vector>
#include <time.h>
#include <stdexcept>

using namespace std;

// typedef signed short MY_TYPE;
typedef double MY_TYPE;
typedef vector<double> FrequenciesData;

struct AWeight {
	double freq;
	double weight;
};

vector<AWeight> aWeights = {
    {6.3, -85.4}, {8, -77.6},  {10, -70.4}, {12.5, -63.6}, {16, -56.4},  {20, -50.4},   {25, -44.8},   {31.5, -39.5}, {40, -34.5},
    {50, -30.3},  {63, -26.2}, {80, -22.4}, {100, -19.1},  {125, -16.2}, {160, -13.2},  {200, -10.8},  {250, -8.7},   {315, -6.6},
    {400, -4.8},  {500, -3.2}, {630, -1.9}, {800, -0.8},   {1000, 0.0},  {1250, 0.6},   {1600, 1.0},   {2000, 1.2},   {2500, 1.3},
    {3150, 1.2},  {4000, 1.0}, {5000, 0.6}, {6300, -0.1},  {8000, -1.1}, {10000, -2.5}, {12500, -4.3}, {16000, -6.7}, {20000, -9.3}};

struct VisualizationData {
	vector<double> spectrum;
	double max;
};

struct OctaveBand {
	unsigned int midFreq;
	unsigned int lowFreq;
	unsigned int highFreq;
};

enum SpectrumScale {
	linear,
	logarithmic
};

class Analyser {
	bool first = true; 					// control cout for debug
	unsigned int fftSize; 				// must be the power of 2
	unsigned int sampleRate;
	double frequencyResolution;
	vector<OctaveBand> octaveBands;
	vector<double> spectrumLog;
	Aquila::HannWindow waveformWindow;
	SpectrumScale scale = linear;

	vector<double> lastPeeks;
	vector<double>::iterator currentPeekIter;
	unsigned int currentPeek = 1;
	clock_t lastCheckedInc = clock();
	clock_t lastCheckedDec = clock();


  public:
	Analyser(unsigned int sampleRateP, unsigned int fftSizeP): waveformWindow(fftSizeP) {
		fftSize = fftSizeP;
		sampleRate = sampleRateP;
		frequencyResolution = (double)sampleRate / fftSize;
		cout << "Frequency resolution: " << sampleRate << " / " << fftSize << " = " << frequencyResolution << " Hz"<< endl;

		calculateOctaveBands(4);
		spectrumLog.assign(octaveBands.size(), 0);
		lastPeeks.assign(30, 1);
		currentPeekIter = lastPeeks.begin();
	}

	VisualizationData getVisualization(vector<MY_TYPE> inputSignal) {
		if (fftSize != inputSignal.size()) {
			throw invalid_argument("Size inputSignal has to be equal to the predefined fftSize.");
		}
		FrequenciesData frData = getFrequencies(inputSignal);
		executeSpectrumAnalysis(frData);
		calculateCurrentPeek();

		first = false;
		return VisualizationData { spectrumLog, (double)currentPeek };
	}

  private:
	FrequenciesData getFrequencies(vector<MY_TYPE> inputSignal) {
		Aquila::SignalSource fftInputWindowed;

		// if (is_same<double, MY_TYPE>::value) {
		fftInputWindowed = inputSignal * waveformWindow;
		// } else {
		// 	vector<double> fftInput(fftSize);

		// 	for (unsigned long i = 0; i < fftSize; i++) {
		// 		fftInput[i] = inputSignal[i];
		// 	}

		// 	fftInputWindowed = fftInput * waveformWindow;
		// }

		auto fft = Aquila::FftFactory::getFft(fftSize);
		Aquila::SpectrumType spectrum = fft->fft(fftInputWindowed.toArray());

		size_t spectrumSize = spectrum.size() / 2; // Take only the first half, the second is a mirror of first (why?)
		vector<double> fftOutput(spectrumSize);

		for (size_t i = 0; i < spectrumSize; i++) {
			auto real = spectrum[i].real();
			auto imag = spectrum[i].imag();

			double magnitude = sqrt(real * real + imag * imag);
			fftOutput[i] = magnitude;
		}

		return fftOutput;
	}

	void executeSpectrumAnalysis(FrequenciesData frData) {
		for (size_t spectrumBin = 0; spectrumBin < spectrumLog.size(); spectrumBin++) {
			OctaveBand currentOctave = octaveBands[spectrumBin];
			double spectrumBinValue = 0;
			size_t specLineLow = round(currentOctave.lowFreq / frequencyResolution);
			size_t specLineHigh = round(currentOctave.highFreq / frequencyResolution); // Not part of current spectrum bin
			if (specLineHigh > frData.size()) specLineHigh = frData.size(); // Do not exceed frData size;

			if (first) cout <<"Octave range: "<<currentOctave.lowFreq<< ": " << specLineLow<<" - "<<specLineHigh-1<<": "<<currentOctave.highFreq << " | ";

			for (size_t i = specLineLow; i < specLineHigh; i++) {
				spectrumBinValue = max(frData[i], spectrumBinValue);
			}
			if (scale == linear) {
				spectrumBinValue = applyWeightMagnitude(currentOctave.midFreq, spectrumBinValue);
			} else {
				spectrumBinValue = applyWeightDb(currentOctave.midFreq, spectrumBinValue);
			}
			const double factorOfPrev = 0.5; // value between [0,1]
			spectrumLog[spectrumBin] = spectrumLog[spectrumBin] * factorOfPrev + (1 - factorOfPrev) * spectrumBinValue;
		}
	}

	void calculateOctaveBands(short N = 3, unsigned int lowwesMidFreq = 40) { // 1/N Octave bands
		// Middle frequency: f0
		// Lower freq. bound: f0 / (2^1/2)^1/N = f0 / 2^(1/2 * 1/N)
		// Upper freq. bound: f0 * (2^1/2)^1/N = f0 * 2^(1/2 * 1/N)

		unsigned int higgestMidFreq = 16000;
		double midFreqInterval = pow(2, 1.0 / N);
		double edgeRatio = pow(2, 0.5 / N);

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

	double getWeight(unsigned int freq, double magnitude) {
		AWeight lowWeight = aWeights[0];
		AWeight highWeight = aWeights[0];
		for (auto a = aWeights.begin(); a != aWeights.end(); a++) {
			if (a -> freq == freq) {
				lowWeight = *a;
				highWeight = *a;
			}
			else if (a -> freq < freq) {
				lowWeight = *a;
			} else if (highWeight.freq < freq && freq < a -> freq ) {
				highWeight = *a;
			}
		}

		double weight;
		if (lowWeight.freq == highWeight.freq) {
			weight = lowWeight.weight;
		} else {
			double ratio = (freq - lowWeight.freq) / (highWeight.freq - lowWeight.freq);
			weight = (highWeight.weight - lowWeight.weight) * ratio + lowWeight.weight;
		}

		return weight;
	}

	double applyWeightDb(unsigned int freq, double magnitude) {
		double weight = getWeight(freq, magnitude);
		double db = 0;
		if (magnitude != 0) {
			db =  20 * log10(magnitude);	// logarithmic (power)
		}

		return db + weight;
	}

	double applyWeightMagnitude(unsigned int freq, double magnitude) {
		double weight = getWeight(freq, magnitude);
		double db = 0;
		if (magnitude != 0) {
			db =  20 * log10(magnitude);
		}

		if (first) cout<<"Freq: "<<freq<< "\tDb: "<<db<<"\tWeight: "<<weight<<endl;
		const double weightFactor = 0.35;
		double magnitudeWithWeight = pow(10, (db + weight * weightFactor)/20); // change back from db (power) to magnitude

		return magnitudeWithWeight;
	}

	void calculateCurrentPeek() {
		// Update current peek
		const double intervalInSeconds = 0.05;
		if ((clock() - lastCheckedInc) / (double)CLOCKS_PER_SEC >= intervalInSeconds) {
			lastCheckedInc = clock();

			auto maxIter = max_element(spectrumLog.begin(), spectrumLog.end());
			*currentPeekIter = *maxIter;
			currentPeekIter++;
			if (currentPeekIter == lastPeeks.end()) {
				currentPeekIter = lastPeeks.begin();
			}

			double sum = 0;
			for (double peek : lastPeeks) {
				sum += peek;
			}

			unsigned int avg = round(sum / lastPeeks.size());
			if (avg > currentPeek) {
				currentPeek = min(avg, (unsigned int)120);
			}
		}

		// Slowly reduce current peek
		if (currentPeek > 1 && (clock() - lastCheckedDec) / (double)CLOCKS_PER_SEC >= 0.7) {
			lastCheckedDec = clock();
			currentPeek--;
		}
	}
};
