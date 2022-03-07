# Compiling a file that requires RtAduio:

example:
g++ -Wall -D__WINDOWS_WASAPI__ -I../rtaudio-5.2.0/include -o yourprogram yourprogram.cpp ../rtaudio-5.2.0RtAudio.cpp -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid

https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index


# Compiling a file that requires Aquile:

g++ -Wall -I../Aquila/include -o test_aquila test_aquila.cpp -L../Aquila/lib -lAquila -L../aquila-src/build/lib -lOoura_fft


# Compile

g++ -Wall -D__WINDOWS_WASAPI__ -I../rtaudio-5.2.0/include -I../Aquila/include -o main main.cpp UdpClient.cpp Analyser.cpp RtAudio.cpp -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid -lws2_32 -L../Aquila/lib -lAquila -L../aquila-src/build/lib -lOoura_fft -std=c++20
