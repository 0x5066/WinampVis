i686-w64-mingw32-windres -i configbox.res -o configbox.rc
i686-w64-mingw32-windres -i configbox.rc -o configbox.res.o
i686-w64-mingw32-g++ -g3 -O0 -shared -o vis_sdl2.dll SVIS.CPP configbox.res.o -lgdi32 -mwindows -lcomctl32 -lSDL2 -lSDL2_ttf
cp vis_sdl2.dll "$HOME/.wine/drive_c/Program Files (x86)/Winamp/Plugins/vis_sdl2.dll"
# assumes you symlinked vis_sdl2 from the winamp plugins folder to your wacup plugins folder
