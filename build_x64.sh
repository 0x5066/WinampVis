x86_64-w64-mingw32-windres -i configbox.res -o configbox.rc
x86_64-w64-mingw32-windres -i configbox.rc -o configbox.res.o
x86_64-w64-mingw32-g++ -g3 -O0 -shared -o vis_sdl2_x64.dll SVIS.CPP configbox.res.o -lgdi32 -mwindows -lcomctl32 -lSDL2 -lSDL2_ttf
cp vis_sdl2_x64.dll "$HOME/.wine/drive_c/Program Files/WACUP/Plugins/vis_sdl2.dll"
