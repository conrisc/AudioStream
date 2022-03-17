#define COLUMNS 32
#define ROWS 8

typedef uint8_t PIXEL;

enum MessageType : PIXEL {
  setBrightness = 1,
  lightMatrixColumns
};

class CommunicationController {

  public:
	CommunicationController() {}

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
};
