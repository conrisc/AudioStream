#define COLUMNS 16
#define ROWS 8

typedef uint8_t PIXEL;

enum MessageType : PIXEL {
	setBrightness = 1,
	setMaxBrightness,
	lightMatrixColumns,
	frequencySpectrum,
};

class CommunicationController {
  public:
	CommunicationController() {}

	/*
	  brightness - led brightness, value range [0, 255]
	*/
	vector<PIXEL> getBrightnessMsg(uint8_t brightness) {
		vector<PIXEL> msgBuffer { setBrightness, brightness };
		return msgBuffer;
	}

	vector<PIXEL> getMaxBrightnessMsg(uint8_t brightness) {
		vector<PIXEL> msgBuffer { setMaxBrightness, brightness };
		return msgBuffer;
	}

	vector<PIXEL> getLightMatrixColumnMsg(vector<double> input, double max) {
		double scale = (double)ROWS / max;

		vector<PIXEL> msgBuffer(COLUMNS + 1, 0);
		msgBuffer[0] = lightMatrixColumns;

		unsigned int i = 0;
		for (auto it = msgBuffer.begin() + 1; it != msgBuffer.end(); it++) {
			double scaledValue = 0;

			if (i < (unsigned int)input.size()) {
				scaledValue = input[i++] * scale;
			}

			if (scaledValue > ROWS) {
				*it = (PIXEL)ROWS + 1; // (+1 explanation) Arduino treats 0 as an end of read buffer
			} else {
				*it = (PIXEL)round(scaledValue) + 1;
			}
		}

		return msgBuffer;
	}

	vector<PIXEL> getFrequencySpectrumMsg(vector<double> input, double max) {
		// TODO? It only differs from getLightMatrixColumnMsg by scale
		const int outputMax = 100;
		double scale = (double)outputMax / max;

		vector<PIXEL> msgBuffer(input.size() + 1, 0);
		msgBuffer[0] = frequencySpectrum;

		unsigned int i = 0;
		for (auto it = msgBuffer.begin() + 1; it != msgBuffer.end(); it++) {
			double scaledValue = 0;

			if (i < (unsigned int)input.size()) {
				scaledValue = input[i++] * scale;
			}

			if (scaledValue > outputMax) {
				*it = (PIXEL)outputMax + 1; // (+1 explanation) Arduino treats 0 as an end of read buffer
			} else {
				*it = (PIXEL)round(scaledValue) + 1;
			}
		}

		return msgBuffer;
	}
};
