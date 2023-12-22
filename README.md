# Audiostream

Application which captures the audio stream that is being played in real-time. Then the frames are streamed to the analyser. The output (e.g. audio spectrum) from the analyser is send to the target device via udp client.

Used libaries:
* RtAudio - to capture audio stream from the host device
* Aquila (https://aquila-dsp.org/) - to analyse raw audio stream. This library must be build from source code before it can be used.


## Requirements

To build or run this application you need to install mingw64. You can install MSYS2 (https://www.msys2.org/) first and then MINGW. Remember to add mingw's bin folder to the env variable PATH.

## Compile

g++ -Wall -D__WINDOWS_WASAPI__ -I../rtaudio-5.2.0/include -I../Aquila/include -o main main.cpp UdpClient.cpp RtAudio.cpp -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid -lws2_32 -L../Aquila/lib -lAquila -L../aquila-src/build/lib -lOoura_fft -std=c++20


### Compiling a file that requires RtAduio:

example:
g++ -Wall -D__WINDOWS_WASAPI__ -I../rtaudio-5.2.0/include -o yourprogram yourprogram.cpp ../rtaudio-5.2.0RtAudio.cpp -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid

https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index


### Compiling a file that requires Aquile:

g++ -Wall -I../Aquila/include -o test_aquila test_aquila.cpp -L../Aquila/lib -lAquila -L../aquila-src/build/lib -lOoura_fft

