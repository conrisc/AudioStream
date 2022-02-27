# Compiling a file that requires RtAduio:

example:
g++ -Wall -D__WINDOWS_WASAPI__ -I../rtaudio-5.2.0/include -o yourprogram yourprogram.cpp ../rtaudio-5.2.0RtAudio.cpp -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid

https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index