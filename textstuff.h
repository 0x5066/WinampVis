#include <windows.h>
#include <commctrl.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <vector>
#include <iterator>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#define WIN32_LEAN_AND_MEAN

void renderText(SDL_Renderer* renderer, const char* text_t, int x, int y, TTF_Font* font, SDL_Color color) {

    SDL_Surface* text;

    text = TTF_RenderText_Blended(font, text_t, color);
    if (!text) {
        std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
    }

    SDL_Texture* text_texture;

    text_texture = SDL_CreateTextureFromSurface(renderer, text);

    SDL_Rect srcRect = {0, 0, text->w, text->h};  // Source rectangle (entire text surface)
    SDL_Rect destRect = {x, y, text->w, text->h}; // Destination rectangle (entire window)

    SDL_RenderCopy(renderer, text_texture, &srcRect, &destRect);

    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text);
}

std::string formatTime(int milliseconds) {
    // Convert milliseconds to seconds
    int totalSeconds = milliseconds / 1000;

    // Extract minutes and seconds
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    // Format as MM:SS
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;

    return ss.str();
}

const char* GetInfoText(HWND winampWindow) {
    // Send the WM_WA_IPC message to get the current output time
    LRESULT waOutputTime = SendMessage(winampWindow, WM_WA_IPC, 0, IPC_GETOUTPUTTIME);
    LRESULT waOutputTime2 = SendMessage(winampWindow, WM_WA_IPC, 2, IPC_GETOUTPUTTIME);

    // Convert the time to MM:SS format
    std::string outputTimeString = formatTime(static_cast<int>(waOutputTime));
    std::string outputTimeString2 = formatTime(static_cast<int>(waOutputTime2));

    // Combine the strings
    std::string combinedOutputTime = outputTimeString + " / " + outputTimeString2;

    // Duplicate the string to return a const char*
    char* infoText = strdup(combinedOutputTime.c_str());
    return infoText;
}

const char* GetSongLength(HWND winampWindow) {
    // Send the WM_WA_IPC message to get the current output time
    LRESULT waOutputTime2 = SendMessage(winampWindow, WM_WA_IPC, 2, IPC_GETOUTPUTTIME);

    // Convert the time to MM:SS format
    std::string outputTimeString2 = formatTime(static_cast<int>(waOutputTime2));

    // Duplicate the string to return a const char*
    char* infoText = strdup(outputTimeString2.c_str());
    return infoText;
}

// Function to convert a wide character string to a narrow character string
std::string ConvertWideStringToNarrow(const wchar_t* wideString) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);

    if (bufferSize == 0) {
        MessageBox(nullptr, "Error converting wide string to narrow string", "Error", MB_OK | MB_ICONERROR);
        return std::string();
    }

    std::string narrowString(bufferSize - 1, 0);  // Decrease bufferSize by 1 to avoid including the null terminator
    WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &narrowString[0], bufferSize, nullptr, nullptr);

    return narrowString;
}

// Function to render marquee text
void renderMarqueeText(SDL_Renderer* renderer, const char* text, int x, int y, TTF_Font* font, SDL_Color color, int screenWidth) {
    // Calculate text width
    int textWidth, textHeight;
    TTF_SizeText(font, text, &textWidth, &textHeight);

    // Check if the text is longer than the screen width
    if (textWidth <= screenWidth) {
        // If not, render the text without the marquee effect
        renderText(renderer, text, x, y, font, color);
        return;
    }

    // Repeat the title for the marquee effect
    int repeatCount = (screenWidth / textWidth) + 6;
    std::string repeatedText;
    for (int i = 0; i < repeatCount; ++i) {
        repeatedText += text;
        if (i < repeatCount - 1) {
            repeatedText += " *** ";
        }
    }

    // Create a surface from the repeated text
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, repeatedText.c_str(), color);

    // Create a texture from the surface
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Set the clip rectangle
    SDL_Rect clipRect = { x, y, screenWidth, textHeight };

    // Render the text to create the marquee effect
    int offset = (SDL_GetTicks() / 20) % (textWidth + screenWidth);
    SDL_Rect srcRect = { offset, 0, screenWidth, textHeight };
    SDL_Rect destRect = { x, y, screenWidth, textHeight };

    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

    // Cleanup
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
}

std::string CreateSongTickerText(HWND winampWindow) {
    int pos = SendMessage(winampWindow, WM_WA_IPC, 0, IPC_GETLISTPOS);

    wchar_t* title = (wchar_t*)SendMessage(winampWindow, WM_WA_IPC, 0, IPC_GET_PLAYING_TITLE);
    std::string narrowTitle = ConvertWideStringToNarrow(title);

    const char* getSongTime = GetSongLength(winampWindow);

    // Combine the strings
    return std::to_string(pos + 1) + ". " + narrowTitle + " (" + getSongTime + ")";
}