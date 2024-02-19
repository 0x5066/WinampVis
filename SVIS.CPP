// Winamp test visualization library v1.0
// Copyright (C) 1997-1998, Justin Frankel/Nullsoft
// Feel free to base any plugins on this "framework"...
// thanks justin, i owe ya.

#include "wacup/wa_ipc.h"
#include "wacup/vis.h"
#include "namedefs.h"
#include "textstuff.h"
#include "resource.h"
#include "wm_messages.h"
//#include <prsht.h>

#define WIDTH 76
#define HEIGHT 16
#define VIS_WIDTH 75

const int NUM_BARS = 75;

const wchar_t szAppName[] = L"SimpleVis"; // Our window class, etc

struct Bar {
    int height;
    double height2;
    double peak;
    double gravity;
    double bargrav;
    int vumeter;
    float vumeterpeak;
};

#ifdef _WIN64
// Code specific to 64-bit Windows
#define MY_GWL_USERDATA GWLP_USERDATA
#else
// Code specific to 32-bit Windows
#define MY_GWL_USERDATA GWL_USERDATA
#endif

std::vector<Bar> bars(NUM_BARS, { 0, 0.0, 0.0, 0.0, 0.0, 0, 0.0 });

std::ofstream debugLogFile;

// configuration declarations
int config_x = 50, config_y = 50;	// screen X position and Y position, repsectively
int config_w = 550, config_h = 153;	// screen X position and Y position, repsectively
void config_read(struct winampVisModule* this_mod);		// reads the configuration
void config_write(struct winampVisModule* this_mod);	// writes the configuration
void config_getinifn(struct winampVisModule* this_mod, char* ini_file); // makes the .ini file filename

// returns a winampVisModule when requested. Used in hdr, below
winampVisModule* getModule(int which);

// "member" functions
void config(struct winampVisModule* this_mod); // configuration dialog
int init(struct winampVisModule* this_mod);	   // initialization for module
int render1(struct winampVisModule* this_mod);  // rendering for module 1
int render2(struct winampVisModule* this_mod);  // rendering for module 2
int render3(struct winampVisModule* this_mod);  // rendering for module 3
int render4(struct winampVisModule* this_mod);  // rendering for module 4
void quit(struct winampVisModule* this_mod);   // deinitialization for module
int random_presets_flag = 0;
int is_paused = 0;
int dsize = 0;
int dsizeX = 24;
int dsizeY = 43;
int dsizeW = WIDTH;
int dsizeH = HEIGHT;

typedef struct {
    Uint8 r, g, b, a;
} Color;

Color colors[] = {
    {0, 0, 0, 255},        // color 0 = black
    {24, 33, 41, 255},     // color 1 = grey for dots
    {239, 49, 16, 255},    // color 2 = top of spec
    {206, 41, 16, 255},    // 3
    {214, 90, 0, 255},     // 4
    {214, 102, 0, 255},    // 5
    {214, 115, 0, 255},    // 6
    {198, 123, 8, 255},    // 7
    {222, 165, 24, 255},   // 8
    {214, 181, 33, 255},   // 9
    {189, 222, 41, 255},   // 10
    {148, 222, 33, 255},   // 11
    {41, 206, 16, 255},    // 12
    {50, 190, 16, 255},    // 13
    {57, 181, 16, 255},    // 14
    {49, 156, 8, 255},     // 15
    {41, 148, 0, 255},     // 16
    {24, 132, 8, 255},     // 17 = bottom of spec
    {255, 255, 255, 255},  // 18 = osc 1
    {214, 214, 222, 255},  // 19 = osc 2 (slightly dimmer)
    {181, 189, 189, 255},  // 20 = osc 3
    {160, 170, 175, 255},  // 21 = osc 4
    {148, 156, 165, 255},  // 22 = osc 5
    {150, 150, 150, 255}   // 23 = analyzer peak dots
};

Color* osccolors(const Color* colors) {
    static Color osc_colors[17];

    osc_colors[0] = colors[21];
    osc_colors[1] = colors[21];
    osc_colors[2] = colors[20];
    osc_colors[3] = colors[20];
    osc_colors[4] = colors[19];
    osc_colors[5] = colors[19];
    osc_colors[6] = colors[18];
    osc_colors[7] = colors[18];
    osc_colors[8] = colors[19];
    osc_colors[9] = colors[19];
    osc_colors[10] = colors[20];
    osc_colors[11] = colors[20];
    osc_colors[12] = colors[21];
    osc_colors[13] = colors[21];
    osc_colors[14] = colors[22];
    osc_colors[15] = colors[22];
    osc_colors[16] = colors[22];

    return osc_colors;
}

Color* vucolor(const Color* colors) {
    static Color vu_colors[75];

    vu_colors[0] = colors[17];
    vu_colors[1] = colors[17];
    vu_colors[2] = colors[17];
    vu_colors[3] = colors[17];
    vu_colors[4] = colors[17];
    vu_colors[5] = colors[16];
    vu_colors[6] = colors[16];
    vu_colors[7] = colors[16];
    vu_colors[8] = colors[16];
    vu_colors[9] = colors[16];
    vu_colors[10] = colors[15];
    vu_colors[11] = colors[15];
    vu_colors[12] = colors[15];
    vu_colors[13] = colors[15];
    vu_colors[14] = colors[14];
    vu_colors[15] = colors[14];
    vu_colors[16] = colors[14];
    vu_colors[17] = colors[14];
    vu_colors[18] = colors[14];
    vu_colors[19] = colors[13];
    vu_colors[20] = colors[13];
    vu_colors[21] = colors[13];
    vu_colors[22] = colors[13];
    vu_colors[23] = colors[13];
    vu_colors[24] = colors[12];
    vu_colors[25] = colors[12];
    vu_colors[26] = colors[12];
    vu_colors[27] = colors[12];
    vu_colors[28] = colors[12];
    vu_colors[29] = colors[11];
    vu_colors[30] = colors[11];
    vu_colors[31] = colors[11];
    vu_colors[32] = colors[11];
    vu_colors[33] = colors[10];
    vu_colors[34] = colors[10];
    vu_colors[35] = colors[10];
    vu_colors[36] = colors[10];
    vu_colors[37] = colors[10];
    vu_colors[38] = colors[9];
    vu_colors[39] = colors[9];
    vu_colors[40] = colors[9];
    vu_colors[41] = colors[9];
    vu_colors[42] = colors[9];
    vu_colors[43] = colors[8];
    vu_colors[44] = colors[8];
    vu_colors[45] = colors[8];
    vu_colors[46] = colors[8];
    vu_colors[47] = colors[8];
    vu_colors[48] = colors[7];
    vu_colors[49] = colors[7];
    vu_colors[50] = colors[7];
    vu_colors[51] = colors[7];
    vu_colors[52] = colors[6];
    vu_colors[53] = colors[6];
    vu_colors[54] = colors[6];
    vu_colors[55] = colors[6];
    vu_colors[56] = colors[6];
    vu_colors[57] = colors[5];
    vu_colors[58] = colors[5];
    vu_colors[59] = colors[5];
    vu_colors[60] = colors[5];
    vu_colors[61] = colors[5];
    vu_colors[62] = colors[4];
    vu_colors[63] = colors[4];
    vu_colors[64] = colors[4];
    vu_colors[65] = colors[4];
    vu_colors[66] = colors[4];
    vu_colors[67] = colors[3];
    vu_colors[68] = colors[3];
    vu_colors[69] = colors[3];
    vu_colors[70] = colors[3];
    vu_colors[71] = colors[2];
    vu_colors[72] = colors[2];
    vu_colors[73] = colors[2];
    vu_colors[74] = colors[2];

    return vu_colors;
}

// Global variables to store the window and renderer
SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* font;
HWND childWindow = NULL;

// Initialize SDL
static void initSDL(HWND hwnd) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    //window = SDL_CreateWindow("SDL Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * ZOOM, HEIGHT * ZOOM, SDL_WINDOW_RESIZABLE);
    window = SDL_CreateWindowFrom(hwnd);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //SetWindowPos(SDL_GetWindowHWND(window), NULL, 0, 0, 75 * ZOOM, 16 * ZOOM, SWP_NOZORDER | SWP_NOACTIVATE);

    const char* fontPath = "C:\\Windows\\Fonts\\tahoma.ttf";
    const int fontSize = 16;
    font = TTF_OpenFont(fontPath, fontSize);

    if (!font) {
        // Handle font loading error
        std::string errorMessage = "Failed to load TTF font: ";
        errorMessage += fontPath;

        // Use TTF_GetError() to get more information about the error
        errorMessage += "\nError: " + std::string(TTF_GetError());

        MessageBox(hwnd, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);

        TTF_Quit();  // Quit SDL_ttf before returning
        SDL_Quit();  // Quit SDL before returning
        return;  // Return or add appropriate error handling
    }
    /*     if (font) {
            // Handle font loading error
            std::string errorMessage = "Loaded TTF from ";
            errorMessage += fontPath;

            // Use TTF_GetError() to get more information about the error
            errorMessage += "\nError: " + std::string(TTF_GetError());

            MessageBox(hwnd, errorMessage.c_str(), "Error", MB_OK | MB_ICONERROR);
            return;  // Return or add appropriate error handling
        } */
}

// Cleanup SDL
static void cleanupSDL() {
    //UnregisterClass(szAppName,this_mod->hDllInstance); // unregister window class
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    //font = nullptr;
    TTF_Quit();
    SDL_Quit();
}

// Function to draw a rectangle
static void drawRect(SDL_Renderer* renderer, int x, int y, int zoom, Color color) {
    SDL_Rect rect = { x * zoom, y * zoom, zoom, zoom };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

static void presentRenderer() {
    SDL_RenderPresent(renderer);
}

// our window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND hMainWnd; // main window handle
HWND hwndWinamp = FindWindow("Winamp v1.x", NULL); // find winamp

// Double buffering data
/*HDC memDC;		// memory device context
HBITMAP	memBM,  // memory bitmap (for memDC)
        oldBM;  // old bitmap (from memDC)*/


        // Module header, includes version, description, and address of the module retriever function
winampVisHeader hdr = { VIS_HDRVER, WACUPHEADER, getModule };

// first module (oscilloscope)
winampVisModule mod1 =
{
    WACUPOSC,
    NULL,    // hwndParent
    NULL,    // hDllInstance
    0,       // sRate
    0,       // nCh
    0,       // latencyMS - delay between audio & video
    16,       // delayMS - winamp will make sure that at least this much time passes per frame.
    0,       // spectrumNch
    2,       // waveformNch
    { 0, },  // spectrumData
    { 0, },  // waveformData
    config,
    init,
    render1,
    quit
};

// second module (spectrum analyzer)
winampVisModule mod2 =
{
    WACUPSA,
    NULL,    // hwndParent
    NULL,    // hDllInstance
    0,       // sRate
    0,       // nCh
    0,       // latencyMS - delay between audio & video
    16,      // delayMS - winamp will make sure that at least this much time passes per frame.
    2,       // spectrumNch
    0,       // waveformNch
    { 0, },  // spectrumData
    { 0, },  // waveformData
    config,
    init,
    render2,
    quit
};

// third module (VU meter)
winampVisModule mod3 =
{
    WACUPVU,
    NULL,    // hwndParent
    NULL,    // hDllInstance
    0,       // sRate
    0,       // nCh
    0,       // latencyMS - delay between audio & video
    16,      // delayMS - winamp will make sure that at least this much time passes per frame.
    0,       // spectrumNch
    0,       // waveformNch
    { 0, },  // spectrumData
    { 0, },  // waveformData
    config,
    init,
    render3,
    quit
};

// third module (X-Y Scope)
winampVisModule mod4 =
{
    WACUPXY,
    NULL,    // hwndParent
    NULL,    // hDllInstance
    0,       // sRate
    0,       // nCh
    0,       // latencyMS - delay between audio & video
    16,      // delayMS - winamp will make sure that at least this much time passes per frame.
    0,       // spectrumNch
    2,       // waveformNch
    { 0, },  // spectrumData
    { 0, },  // waveformData
    config,
    init,
    render4,
    quit
};

// this is the only exported symbol. returns our main header.
// if you are compiling C++, the extern "C" { is necessary, so we just #ifdef it
#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) winampVisHeader* winampVisGetHeader()
    {
        return &hdr;
    }
#ifdef __cplusplus
}
#endif

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1, mod2 or mod3 depending on 'which'.
winampVisModule* getModule(int which)
{
    switch (which)
    {
    case 0: return &mod1;
    case 1: return &mod2;
    case 2: return &mod3;
    case 3: return &mod4;
    default:return NULL;
    }
}

winampVisModule* g_mod = NULL;

/* // Function to show the configuration dialog
static void ShowDialog(HINSTANCE hInstance) {
    RECT winampRect;
    GetWindowRect(hwndWinamp, &winampRect);

    // Calculate the position for the config dialog relative to Winamp
    int xPos = winampRect.left + 50;  // Adjust as needed
    int yPos = winampRect.top + 50;   // Adjust as needed

    // Use CreateDialogParam to create a modeless dialog
    HWND hDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CONFIGBOX), hwndWinamp, DialogProc, MAKELPARAM(xPos, yPos));

    // Show the modeless dialog
    ShowWindow(hDialog, SW_SHOWNORMAL);
} */

// Configuration function
void config(struct winampVisModule* this_mod) {
    // Initialize property sheet headers and pages
    PROPSHEETPAGE psp[4] = { 0 };
    PROPSHEETHEADER psh = { 0 };

    // First property sheet page
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_DEFAULT;
    psp[0].hInstance = this_mod->hDllInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_CONFIGBOX);
    psp[0].pfnDlgProc = (DLGPROC)DialogProc;

    // Second property sheet page
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_DEFAULT;
    psp[1].hInstance = this_mod->hDllInstance;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_ANALYZER);
    psp[1].pfnDlgProc = (DLGPROC)AnalyzerDialogProc;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_DEFAULT;
    psp[2].hInstance = this_mod->hDllInstance;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_OSCILLOSCOPE);
    psp[2].pfnDlgProc = (DLGPROC)OscDialogProc;

    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_DEFAULT;
    psp[3].hInstance = this_mod->hDllInstance;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_VU_METER);
    psp[3].pfnDlgProc = (DLGPROC)VUDialogProc;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPTITLE | PSH_PROPSHEETPAGE | PSH_NOCONTEXTHELP;
    psh.hwndParent = hwndWinamp;
    psh.hInstance = this_mod->hDllInstance;
    psh.pszCaption = "vis_sdl2";
    psh.nPages = 4; // Updated to reflect two pages
    psh.ppsp = psp; // Updated to point to the array of property sheet pages

    // Display property sheet
    PropertySheet(&psh);
}

embedWindowState myWindowState;

int width;
int height;

// initialization. Registers our window class, creates our window, etc. Again, this one works for
// both modules, but you could make init1() and init2()...
// returns 0 on success, 1 on failure.
int init(struct winampVisModule* this_mod)
{
    int styles;
    HWND parent = NULL;
    HWND (*e)(embedWindowState *v);
    vinit = 1;

    debugLogFile.open("debug_log.txt");

    width = 550;
    height = 153;

    g_mod = this_mod;

    dsize = SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_ISDOUBLESIZE);

    if (g_bEmbedIntoMainWindow) {
        c_zoom = dsize;
    } else {
        c_zoom = ZOOM;
    }

    config_read(this_mod);

    /* if (g_bEmbedIntoMainWindow) {
        ZOOM = 2;
    } */

    // uncomment this line if your plugin draws to the screen using directx OVERLAY mode
    myWindowState.flags = EMBED_FLAGS_SCALEABLE_WND; //new shiny thing

    myWindowState.r.left = config_x;
    myWindowState.r.top = config_y;
    myWindowState.r.right = config_w/* + width*/;
    myWindowState.r.bottom = config_h/* + height*/;

    *(void**)&e = (void *)SendMessage(this_mod->hwndParent,WM_WA_IPC,(LPARAM)0,IPC_GET_EMBEDIF);

    if (!e)
    {
        MessageBoxW(this_mod->hwndParent, L"This plugin requires Winamp 5.0+", L"blah", MB_OK);
        return 1;
    }

    parent = e(&myWindowState);
#ifdef _WIN64
    SetWindowTextW(myWindowState.me, this_mod->description); // set our window title
#else
    SetWindowText(myWindowState.me, this_mod->description); // set our window title
#endif //_WIN64

    {	// Register our window class
        WNDCLASSW wc; // Note the 'W' suffix for the Unicode version of the structure
        memset(&wc, 0, sizeof(wc));
        wc.lpfnWndProc = WndProc; // our window procedure
        wc.hInstance = this_mod->hDllInstance; // hInstance of DLL
        wc.lpszClassName = szAppName; // our window class name

        if (!RegisterClassW(&wc)) // Note the 'W' suffix for the Unicode version of RegisterClass
        {
            MessageBoxW(this_mod->hwndParent, L"Error registering window class,\nunregistering the pre-existing instance and killing SDL2...", L"blah", MB_OK);
            quit(this_mod);
            return 1;
        }
    }

    styles = WS_VISIBLE | WS_CHILDWINDOW | WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    if (g_bEmbedIntoMainWindow) {
        hMainWnd = CreateWindowExW(
            WS_EX_NOPARENTNOTIFY | WS_EX_TOPMOST, // Add WS_EX_LAYERED style for transparency
            L"Static",     // Window class name
            NULL,         // Window text
            WS_CHILD | WS_VISIBLE, // Child window styles
            dsizeX,            // X position
            dsizeY,            // Y position
            dsizeW,          // Width
            dsizeH,          // Height
            hwndWinamp,         // Parent window handle
            NULL,         // Menu handle
            this_mod->hDllInstance, // Instance handle
            NULL);        // Additional application data*/
    }
    else {
        hMainWnd = CreateWindowExW(
            0,	                                // these exstyles put a nice small frame, but also a button in the taskbar
            szAppName,							// our window class name
            NULL,				                // no title, we're a child
            styles,				                // do not make the window visible 
            config_x, config_y,					// screen position (read from config)
            config_w, config_h,					// width & height of window (need to adjust client area later)
            parent,				                // parent window (winamp main window)
            NULL,								// no menu
            this_mod->hDllInstance,				// hInstance of DLL
            0); // no window creation data
    }

    if (!hMainWnd)
    {
        MessageBoxW(this_mod->hwndParent, L"Error creating window", L"blah", MB_OK);
        quit(this_mod);
        return 1;
    }

    initSDL(hMainWnd);

#ifdef _WIN64
    SetWindowLong(hMainWnd, MY_GWL_USERDATA, (LONG_PTR)this_mod); // set our user data to a "this" pointer
#else
    SetWindowLong(hMainWnd, MY_GWL_USERDATA, (LONG)this_mod); // set our user data to a "this" pointer
#endif

#ifdef _WIN64
    SendMessageW(this_mod->hwndParent, WM_WA_IPC, (intptr_t)hMainWnd, IPC_SETVISWND);
#else
    SendMessage(this_mod->hwndParent, WM_WA_IPC, (int)hMainWnd, IPC_SETVISWND);
#endif

    // show the window
    if (!g_bEmbedIntoMainWindow) {
        ShowWindow(parent, SW_SHOWNORMAL);
    }

#ifdef _WIN64
    if (wcscmp(this_mod->description, WACUPOSC) == 0) {
        render1(this_mod);
    }
    else if (wcscmp(this_mod->description, WACUPSA) == 0) {
        render2(this_mod);
    }
    else if (wcscmp(this_mod->description, WACUPVU) == 0) {
        render3(this_mod);
    }
    else if (wcscmp(this_mod->description, WACUPXY) == 0) {
        render4(this_mod);
    }
#else
    if (strcmp(this_mod->description, WACUPOSC) == 0) {
        render1(this_mod);
    }
    else if (strcmp(this_mod->description, WACUPSA) == 0) {
        render2(this_mod);
    }
    else if (strcmp(this_mod->description, WACUPVU) == 0) {
        render3(this_mod);
    }
    else if (strcmp(this_mod->description, WACUPXY) == 0) {
        render4(this_mod);
    }
#endif //_WIN64

    return 0;
}

// Function to clear the SDL renderer with a specified color
void clearRenderer(SDL_Renderer* renderer, Color backgroundColor) {
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);
    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            if (x % 2 == 1 || y % 2 == 0) {
                drawRect(renderer, x, y, c_zoom, colors[0]);
            }
            else {
                drawRect(renderer, x, y, c_zoom, colors[1]);
            }
        }
    }
}

// render function for oscilliscope. Returns 0 if successful, 1 if visualization should end.
int render1(struct winampVisModule* this_mod)
{
    HWND winampWindow = this_mod->hwndParent;
    int x, y, newX, NEW_WIDTH;

    int last_y = 0;
    int top, bottom, intValue, scaledValueY, newY;
    char* sadata;
    Color* osc_colors = osccolors(colors);
    clearRenderer(renderer, { 0, 0, 0, 255 });

    if (!g_bEmbedIntoMainWindow) {
        c_zoom = ZOOM;
    }

    if (g_bEmbedIntoMainWindow) {
        dsize = SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_ISDOUBLESIZE) + 1;

        dsizeX = 24 * dsize;
        dsizeY = 43 * dsize;
        dsizeW = WIDTH * dsize;
        dsizeH = HEIGHT * dsize;
        c_zoom = dsize;
        SetWindowPos(hMainWnd, NULL, dsizeX, dsizeY, dsizeW, dsizeH, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    const char* infoText = GetInfoText(winampWindow);
    const std::string combinedText = CreateSongTickerText(winampWindow);

    // int targetWidth = 75; // We don't need to scale X

    // Get the SA data function pointer
/* 	char *(*export_sa_get)(char data[75*2 + 8]);
    *(void**)&export_sa_get = (void*)SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETSADATAFUNC); */

    static char* (*export_sa_get)(void) = NULL;
    static void (*export_sa_setreq)(int) = NULL;

    if (!g_bwavetrumData) {

        /* Get function pointers from Winamp */
        if (!export_sa_get)
            export_sa_get = (char* (*)(void))SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETSADATAFUNC);
        if (!export_sa_setreq)
            export_sa_setreq = (void (*)(int))SendMessage(this_mod->hwndParent, WM_WA_IPC, 1, IPC_GETSADATAFUNC);

        /* Specify, that we want both spectrum and oscilloscope data */
        export_sa_setreq(1); /* Pass 0 (zero) and get spectrum data only */
        sadata = export_sa_get(); // Visualization data
        NEW_WIDTH = VIS_WIDTH;
    }
    else {
        NEW_WIDTH = 576;
    }

    for (int x = 0; x < NEW_WIDTH; x++) {
        // Calculate the scaled x-coordinate (commented out)
        // int scaledX = static_cast<int>((static_cast<double>(x) / static_cast<double>(576)) * targetWidth);

        // Skip points if they are too close to each other in the scaled version (commented out)
        // if (x > 0 && scaledX == static_cast<int>((static_cast<double>(x - 1) / static_cast<double>(576)) * targetWidth)) {
        //    continue;
        // }

        // Get SA data for the current x position
        //char data[75 * 2 + 8];
        //char *saData = export_sa_get(data);

        // Extract the relevant value from SA data
        if (!g_bwavetrumData) {
            newX = x;
            signed char y = sadata[x + 75];
            intValue = y;
            intValue = (intValue)+7;
            intValue = intValue < 0 ? 0 : (intValue > 16 - 1 ? 16 - 1 : intValue);
            newY = intValue;
        }
        else {
            int scaledX = static_cast<int>((static_cast<double>(x) / static_cast<double>(576)) * VIS_WIDTH);
            newX = scaledX;

            // Skip points if they are too close to each other in the scaled version
            if (x > 0 && scaledX == static_cast<int>((static_cast<double>(x - 1) / static_cast<double>(576)) * VIS_WIDTH)) {
                continue;
            }

            signed char yl = static_cast<signed char>(this_mod->waveformData[0][x]);
            signed char yr = static_cast<signed char>(this_mod->waveformData[1][x]);
            intValue = static_cast<int>(yl + yr) - 6;
            intValue = (intValue)+128;
            intValue = intValue < 0 ? 0 : (intValue > 256 - 1 ? 256 - 1 : intValue);

            scaledValueY = static_cast<int>((static_cast<double>(intValue) / static_cast<double>(256)) * 16.0);
            newY = scaledValueY;
        }

        // int scaledValueY = static_cast<int>((static_cast<double>(intValue) / static_cast<double>(256)) * 16.0); // We don't need to scale Y

        if (x == 0) {
            last_y = newY; // Use intValue directly
        }

        top = newY; // Use intValue directly
        bottom = last_y;
        last_y = newY;

        if (wcscmp(oscstyle, L"lines") == 0 ) {
            if (bottom < top) {
                int temp = bottom;
                bottom = top;
                top = temp + 1;
            }
        } else if (wcscmp(oscstyle, L"solid") == 0 ) {
            if (newY >= 8) {
                top = 8;
                bottom = newY;
            } else {
                top = newY;
                bottom = 7;
            }
        } else if (wcscmp(oscstyle, L"dots") == 0 ) {
            top = newY;
            bottom = newY;
        }

        for (int dy = top; dy <= bottom; dy++) {
            int color_index = (newY);
            Color scope_color = osc_colors[color_index];
            drawRect(renderer, newX, dy, c_zoom, scope_color); // Use x directly
        }
    }

    if (g_bSDL2text || g_bEmbedIntoMainWindow) {
        renderText(renderer, infoText, 30 * ZOOM, 16 * ZOOM, font, { 255,255,255,255 });
        renderMarqueeText(renderer, combinedText.c_str(), 0, 19 * ZOOM, font, { 255, 255, 255, 255 }, 75 * ZOOM);
    }

    presentRenderer();
    return 0;
}

void SpectrumAnalyzer(const std::vector<signed char>& fft, std::vector<Bar>& bars) {
    size_t chunkSize = 1;
    size_t addTogether = 1;

    std::vector<double> bv_vec = {0.19, 0.422, 0.75, 1.0, 2.0};
    size_t bv_index = std::max(static_cast<size_t>(safalloff) - 1, static_cast<size_t>(0));
    bv_index = std::min(bv_index, bv_vec.size() - 1);
    double bv_value = bv_vec[bv_index];

    std::vector<double> pv_vec = {1.0, 1.5, 2.0, 3.0, 4.0};
    size_t pv_index = std::max(static_cast<size_t>(pfalloff) - 1, static_cast<size_t>(0));
    pv_index = std::min(pv_index, pv_vec.size() - 1);
    double pv_value = pv_vec[pv_index];

    if (wcscmp(bandwidth, L"thick") == 0) {
        chunkSize = 4;
        addTogether = 4;
    } else if (wcscmp(bandwidth, L"thin") == 0) {
        chunkSize = 1;
        addTogether = 1;
    }

    if (bars.size() < fft.size()) {
        bars.resize(fft.size());
    }

    for (size_t i = 0; i < fft.size(); i += chunkSize) {
        unsigned int sum = 0;  // Use unsigned int to avoid signed/unsigned issues
        for (size_t j = 0; j < addTogether && i + j < fft.size(); ++j) {
            sum += static_cast<unsigned char>(fft[i + j]);  // Cast to unsigned char
        }

        for (size_t j = 0; j < chunkSize && i + j < fft.size(); ++j) {
            bars[i + j].height = sum / addTogether;

            bars[i + j].height2 -= bv_value;

            // Check if bars.height is above 15 and set it to 15 if true
            if (bars[i + j].height >= 15) {
                bars[i + j].height = 15;
            }

            if (bars[i + j].height2 <= bars[i + j].height) {
                bars[i + j].height2 = bars[i + j].height;
            }
        }
    }

    for (size_t i = 0; i < NUM_BARS; ++i) {
        bars[i].peak -= pv_value / 8;
        if (bars[i].height2 > bars[i].peak) {
            bars[i].peak = bars[i].height2;
        }
        if ((bars[i].height2 >= 13.0f) && (bars[i].height2 <= 14.99f)) {
            // Offset peak by -3
            bars[i].peak = 14;
        }
    }
}

void renderBars(SDL_Renderer* renderer, Color color) {
    const int barCount = 75;
    const int lineSpacing = 4;
    for (int bar_x = 0; bar_x < 75; bar_x += lineSpacing) {
        SDL_Rect rect = {
            (bar_x - 1) * c_zoom,
            0, // Adjust this if you want the lines to start from a different Y coordinate.
            1 * c_zoom, // Set the width of the line (1 pixel for a vertical line).
            16 * c_zoom // Set the height of the line (16 pixels high).
        };

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
    }
}

// render function for analyser. Returns 0 if successful, 1 if visualization should end.
int render2(struct winampVisModule* this_mod)
{
    HWND winampWindow = this_mod->hwndParent;
    std::vector<signed char> fft;

    int x, y, newX, NEW_WIDTH;

    int last_y = 0;
    int top, bottom, color_index;
    float intValue, newY, scaledValueY;
    char* sadata;
    //Color* osc_colors = osccolors(colors);
    clearRenderer(renderer, { 0, 0, 0, 255 });

    if (!g_bEmbedIntoMainWindow) {
        c_zoom = ZOOM;
    }

    if (g_bEmbedIntoMainWindow) {
        dsize = SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_ISDOUBLESIZE) + 1;

        dsizeX = 24 * dsize;
        dsizeY = 43 * dsize;
        dsizeW = WIDTH * dsize;
        dsizeH = HEIGHT * dsize;
        c_zoom = dsize;
        SetWindowPos(hMainWnd, NULL, dsizeX, dsizeY, dsizeW, dsizeH, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    const char* infoText = GetInfoText(winampWindow);
    const std::string combinedText = CreateSongTickerText(winampWindow);

    if (g_bSDL2text || g_bEmbedIntoMainWindow){
        renderText(renderer, infoText, 30 * ZOOM, 16 * ZOOM, font, {255,255,255,255});
        renderMarqueeText(renderer, combinedText.c_str(), 0, 19 * ZOOM, font, {255, 255, 255, 255}, 75 * ZOOM);
    }

    // Get the SA data function pointer
/* 	char *(*export_sa_get)(char data[75*2 + 8]);
    *(void**)&export_sa_get = (void*)SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETSADATAFUNC); */

    static char* (*export_sa_get)(void) = NULL;
    static void (*export_sa_setreq)(int) = NULL;

    if (!g_bwavetrumData) {

        /* Get function pointers from Winamp */
        if (!export_sa_get)
            export_sa_get = (char* (*)(void))SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETSADATAFUNC);
        if (!export_sa_setreq)
            export_sa_setreq = (void (*)(int))SendMessage(this_mod->hwndParent, WM_WA_IPC, 1, IPC_GETSADATAFUNC);

        /* Specify, that we want both spectrum and oscilloscope data */
        export_sa_setreq(0); /* Pass 0 (zero) and get spectrum data only */
        sadata = export_sa_get(); // Visualization data
        NEW_WIDTH = VIS_WIDTH;
    }
    else {
        NEW_WIDTH = 576;
    }

    std::vector<float> curve;
    float multiplier = 1.045;

    // Generate the precomputed curve
    for (int i = 0; i < NEW_WIDTH; i++) {
        float value = pow(multiplier, i);
        curve.push_back(value);
    }

    for (int x = 0; x < NEW_WIDTH; x++) {
        // Get SA data for the current x position
        if (!g_bwavetrumData) {
            static char data[75 * 2 + 8];
            //char *sadata = export_sa_get();

            // Extract the relevant value from SA data
            char y = sadata[x];
            newY = y;
        }
        else {
            // Sum up every 7.65 points of spectrumData
            float sum = 0;
            float count = 0;
            int startIdx = static_cast<int>(x * 6.89);
            int endIdx = static_cast<int>((x + 1) * 6.89);
            for (int i = startIdx; i < endIdx; i++) {
                if (i < 576) {
                    sum += static_cast<float>(this_mod->spectrumData[0][i]) + static_cast<float>(this_mod->spectrumData[1][i]);
                    count++;
                }
            }

            // Calculate average value
            float averageValue = count > 0 ? sum / count : 0;

            // Scale and adjust the value
            //intValue = (averageValue * 2);
            intValue = static_cast<float>((averageValue * curve[x]));
            //intValue = static_cast<int>(averageValue * curve[x]);
            scaledValueY = static_cast<float>((static_cast<float>(intValue) / static_cast<float>(256)) * 16.0);
            newY = scaledValueY/* / 2.5*/;
            //newY = static_cast<float>((scaledValueY * curve[x]));
        }

        if (x >= 75) {
            newY = 0;
        }

        char intValue = newY;

        fft.push_back(intValue);
    }

    SpectrumAnalyzer(fft, bars);

    for (int i = 0; i < VIS_WIDTH; ++i) {
        Bar bar = bars[i];
        int barX = i;
        int barY = 15 - bar.height2;  // Adjusted for proper rendering

        top = barY + 1;
        bottom = 15;

        for (int dy = top; dy <= bottom; ++dy) {
            if (wcscmp(specdraw, L"normal") == 0) {
                color_index = dy + 2;
            } else if (wcscmp(specdraw, L"fire") == 0) {
                color_index = dy - barY + 2;
            } else if (wcscmp(specdraw, L"line") == 0) {
                color_index = barY + 2;
            }
            Color scope_color = colors[color_index];
            drawRect(renderer, barX, dy, c_zoom, scope_color);
        }
    }

    if (peaks){
        for (int i = 0; i < VIS_WIDTH; ++i) {
            Bar bar = bars[i];
            int barX = i;
            int barY = 15 - bar.peak;  // Adjusted for proper rendering

            top = barY + 1;
            bottom = 15;

            for (int dy = top; dy <= bottom; ++dy) {
                drawRect(renderer, barX, barY, c_zoom, colors[23]);
            }
        }
    }


    //debugLogFile << "intValue: " << bars[74].peak << std::endl;
    //renderBars(renderer, colors[0]);
    if (wcscmp(bandwidth, L"thick") == 0) {
        renderBars(renderer, colors[0]);
    } else if (wcscmp(bandwidth, L"thin") == 0) {
        // SORRY NOTHING
    }

    presentRenderer();
    return 0;
}

// render function for VU meter. Returns 0 if successful, 1 if visualization should end.
int render3(struct winampVisModule* this_mod)
{
    int x, y;

    int top, bottom, top2, bottom2, color_index;
    Color* vu_colors = vucolor(colors);
    clearRenderer(renderer, { 0, 0, 0, 255 });

    std::vector<float> pv_vec = {1.0f, 1.125f, 1.5f, 2.0f, 2.5f};
    size_t pv_index = std::max(static_cast<size_t>(vupfalloff) - 1, static_cast<size_t>(0));
    pv_index = std::min(pv_index, pv_vec.size() - 1);
    float pv_value = pv_vec[pv_index];

    if (!g_bEmbedIntoMainWindow) {
        c_zoom = ZOOM;
    } else {
        c_zoom = 2;
    }

    if (g_bEmbedIntoMainWindow) {
        dsize = SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_ISDOUBLESIZE) + 1;

        dsizeX = 24 * dsize;
        dsizeY = 43 * dsize;
        dsizeW = WIDTH * dsize;
        dsizeH = HEIGHT * dsize;
        c_zoom = dsize;
        SetWindowPos(hMainWnd, NULL, dsizeX, dsizeY, dsizeW, dsizeH, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    HWND winampWindow = this_mod->hwndParent;

    const char* infoText = GetInfoText(winampWindow);
    const std::string combinedText = CreateSongTickerText(winampWindow);

    // Get the VU data function pointer
    int (*export_vu_get)(int channel);
    *(void**)&export_vu_get = (void*)SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETVUDATAFUNC);

    for (int x = 0; x < 7; x++) {
        // Get VU data for the left channel (channel 0)
        int leftChannel = export_vu_get(0);
        int rightChannel = export_vu_get(1);

        // Extract the relevant value from VU data
        int intValue = leftChannel * 75 / 255; // Scale down to fit in the 16 available colors
        int intValue2 = rightChannel * 75 / 255; // Scale down to fit in the 16 available colors

        top = 0;
        bottom = intValue - 1;

        top2 = 0;
        bottom2 = intValue2 - 1;

        bars[0].vumeter = intValue;
        bars[1].vumeter = intValue2;

        for (int dy = top; dy <= bottom; dy++) {
            if (wcscmp(vudraw, L"normal") == 0) {
                color_index = (dy);          
            } else if (wcscmp(vudraw, L"line") == 0) {
                color_index = (bottom);
            } else if (wcscmp(vudraw, L"fire") == 0) {
                color_index = (dy - bottom + 74);
            }
            Color scope_color = vu_colors[color_index];
            drawRect(renderer, dy, x + 1, c_zoom, scope_color); // Use x directly
        }

        for (int dy = top2; dy <= bottom2; dy++) {
            if (wcscmp(vudraw, L"normal") == 0) {
                color_index = (dy);          
            } else if (wcscmp(vudraw, L"line") == 0) {
                color_index = (bottom2);
            } else if (wcscmp(vudraw, L"fire") == 0) {
                color_index = (dy - bottom2 + 74);
            }
            Color scope_color = vu_colors[color_index];
            drawRect(renderer, dy, x + 9, c_zoom, scope_color); // Use x directly
        }
    }

    for (int i = 0; i < NUM_BARS; ++i) {
        bars[i].vumeterpeak -= pv_value;
        if (bars[i].vumeterpeak <= bars[i].vumeter) {
            bars[i].vumeterpeak = bars[i].vumeter;
        }
    }

    if (vu_peaks) {
        for (int i = 0; i < 7; ++i) {
            int barX = i;
            int barY = bars[0].vumeterpeak;  // Adjusted for proper rendering

            top = 0;
            bottom = barY;

            for (int dy = top; dy <= bottom; ++dy) {
                color_index = barY;
                Color scope_color = vu_colors[color_index];
                drawRect(renderer, barY, barX + 1, c_zoom, colors[23]);
            }
        }

        for (int i = 0; i < 7; ++i) {
            int barX = i;
            int barY = bars[1].vumeterpeak;  // Adjusted for proper rendering

            top = 0;
            bottom = barY;

            for (int dy = top; dy <= bottom; ++dy) {
                int color_index = barY;
                Color scope_color = vu_colors[color_index];
                drawRect(renderer, barY, barX + 9, c_zoom, colors[23]);
            }
        }
    } else {
        // SORRY NOTHING
    }

    if (g_bSDL2text || g_bEmbedIntoMainWindow) {
        renderText(renderer, infoText, 30 * ZOOM, 16 * ZOOM, font, { 255,255,255,255 });
        renderMarqueeText(renderer, combinedText.c_str(), 0, 19 * ZOOM, font, { 255, 255, 255, 255 }, 75 * ZOOM);
    }
    //MessageBox(winampWindow, combinedText.c_str(), "blah", MB_OK);

    presentRenderer();
    return 0;
}

void clearRenderer2(SDL_Renderer* renderer, Color backgroundColor) {
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    // Draw a semi-transparent rectangle covering the entire renderer
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, NULL);
    //SDL_RenderClear(renderer);
}

int render4(struct winampVisModule* this_mod)
{
    int x, y;
    clearRenderer2(renderer, { 0, 0, 0, 64 });

    // Get the current window size
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    if (!g_bEmbedIntoMainWindow) {
        c_zoom = ZOOM;
    } else {
        c_zoom = 2;
    }

    if (g_bEmbedIntoMainWindow) {
        dsize = SendMessage(hwndWinamp,WM_WA_IPC,0,IPC_ISDOUBLESIZE) + 1;

        dsizeX = 24 * dsize;
        dsizeY = 43 * dsize;
        dsizeW = WIDTH * dsize;
        dsizeH = HEIGHT * dsize;
        c_zoom = dsize;
        SetWindowPos(hMainWnd, NULL, dsizeX, dsizeY, dsizeW, dsizeH, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    for (int x = 0; x < 576; x++)
    {
        signed char y = static_cast<signed char>(this_mod->waveformData[0][x]);
        int intY = static_cast<int>(y);
        intY = (intY)+128;

        signed char x2 = static_cast<signed char>(this_mod->waveformData[1][x]);
        int intX = static_cast<int>(x2);
        intX = (intX)+128;

        // Scale the coordinates based on the window size
        int scaledY = static_cast<int>((static_cast<double>(intY) / 256.0) * (windowWidth / c_zoom));
        int scaledX = static_cast<int>((static_cast<double>(-intX + 256) / 256.0) * (windowHeight / c_zoom));

        drawRect(renderer, scaledY, scaledX, c_zoom, { 124, 252, 40, 128 });
    }

    presentRenderer();
    return 0;
}

// cleanup (opposite of init()). Destroys the window, unregisters the window class
void quit(struct winampVisModule* this_mod)
{
    SendMessage(this_mod->hwndParent, WM_WA_IPC, 0, IPC_SETVISWND);
    vinit = 0;

    config_write(this_mod);		// write configuration
    // delete our window
    if (myWindowState.me)
    {
        SetForegroundWindow(g_mod->hwndParent);
        DestroyWindow(myWindowState.me);
    }
    debugLogFile.close();

    cleanupSDL();
    UnregisterClassW(szAppName, this_mod->hDllInstance); // unregister window class
}

void next_preset()
{
}

void previous_preset()
{
}

void load_random_preset()
{
}

void set_random(int r)
{
    random_presets_flag = r;
}

void go_fullscreen()
{
    if (SendMessage(g_mod->hwndParent, WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO) > 1)
    {
        MessageBoxW(g_mod->hwndParent, L"Can't go fullscreen while video is playing", L"vis_sdl2", 0);
    }
    else
    {
        SendMessage(g_mod->hwndParent, WM_WA_IPC, 1, IPC_SET_VIS_FS_FLAG);

        MessageBoxW(g_mod->hwndParent, L"Why do you want this to go fullscreen?", L"vis_sdl2", 0);

    }
}

void go_windowed()
{
    SendMessage(g_mod->hwndParent, WM_WA_IPC, 0, IPC_SET_VIS_FS_FLAG);
}

void open_configuration()
{
    config(g_mod);
}

void open_popup_menu()
{
    config(g_mod);
}

// window procedure for our window
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:		return 0;
    case WM_ERASEBKGND: return 0;
    case WM_PAINT:
    { // update from doublebuffer
        PAINTSTRUCT ps;
        RECT r;
        HDC hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &r);
        //BitBlt(hdc,0,0,r.right,r.bottom,memDC,0,0,SRCCOPY);
/*{
  RECT x={r.left+width, r.top, r.right, r.bottom};
  RECT y={r.left, r.top+height, r.right, r.bottom};
FillRect(hdc, &x, (HBRUSH)GetStockObject(WHITE_BRUSH));
FillRect(hdc, &y, (HBRUSH)GetStockObject(WHITE_BRUSH));
        }*/
        //EndPaint(hwnd,&ps);
    }
    return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
    case WM_KEYUP:
    {	// get this_mod from our window's user data
#ifdef _WIN64
        winampVisModule* this_mod = (winampVisModule*)GetWindowLongPtr(hwnd, MY_GWL_USERDATA);
#else
        winampVisModule* this_mod = (winampVisModule*)GetWindowLong(hwnd, MY_GWL_USERDATA);
#endif
        PostMessage(this_mod->hwndParent, message, wParam, lParam);
    }
    return 0;
    case WM_WINDOWPOSCHANGING:
    {
        // get config_x and config_y for configuration
        RECT r;
        GetWindowRect(myWindowState.me, &r);
        config_x = r.left;
        config_y = r.top;
        config_w = r.right;
        config_h = r.bottom;

        // tried to get the vis to respond immediately, didnt work
        //InvalidateRect(hwnd, NULL, TRUE);
    }
    return 0;
    case WM_SIZE:
        if (wParam == SIZE_RESTORED) {
            // Window has been resized
            //int newWidth = LOWORD(lParam);
            //int newHeight = HIWORD(lParam);

            // Do something with the new width and height, for example, inform SDL2
            //SDL_SetWindowSize(window, newWidth, newHeight);
        }
        return 0;
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        switch (id) {

            // user clicked on 'next' preset button
        case ID_VIS_NEXT: next_preset(); break;

            // user clicked on 'previous' preset button
        case ID_VIS_PREV: previous_preset(); break;

            // user clicked on 'random' togglebutton
        case ID_VIS_RANDOM: {
            // determine if we're switching random on or off or if Winamp is asking us about the state of our random flag
            int v = HIWORD(wParam) ? 1 : 0;

            // are we being asked about the state of our random flag ?
            if (wParam >> 16 == 0xFFFF) {
                // tell winamp about our state
                SendMessage(g_mod->hwndParent, WM_WA_IPC, random_presets_flag, IPC_CB_VISRANDOM);
                break;
            }

            // changes random_preset_flag 
            set_random(v);

            // if we are turning random on, we should switch to a new random preset right away
            if (v) load_random_preset();

            break;
        }
        case ID_VIS_FS: go_fullscreen(); break;
        case ID_VIS_CFG: open_configuration(); break;
        case ID_VIS_MENU: open_popup_menu(); break;
        }
        break;
    }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void config_getinifnW(struct winampVisModule* this_mod, wchar_t* ini_file)
{
    // makes a .ini file in the winamp directory named "plugin.ini"
    wchar_t* p;
    GetModuleFileNameW(this_mod->hDllInstance, ini_file, MAX_PATH);
    p = ini_file + wcslen(ini_file);
    while (p >= ini_file && *p != L'\\') {
        p--;
    }
    if (++p >= ini_file) {
        *p = L'\0';
    }
    wcscat(ini_file, L"vis_sdl2.ini");
}


void setDefaultIfEmpty(wchar_t* str, const wchar_t* defaultStr) {
    if (wcslen(str) == 0) {
        wcscpy(str, defaultStr);
    }
}

void config_read(struct winampVisModule* this_mod)
{
    wchar_t ini_file[MAX_PATH];
    config_getinifnW(this_mod, ini_file);

    config_x = GetPrivateProfileIntW(L"vis_sdl2 settings", L"Screen_x", config_x, ini_file);
    config_y = GetPrivateProfileIntW(L"vis_sdl2 settings", L"Screen_y", config_y, ini_file);
    config_w = GetPrivateProfileIntW(L"vis_sdl2 settings", L"Screen_w", config_w, ini_file);
    config_h = GetPrivateProfileIntW(L"vis_sdl2 settings", L"Screen_h", config_h, ini_file);
    g_bEmbedIntoMainWindow = GetPrivateProfileIntW(L"vis_sdl2 settings", L"embed2main", g_bEmbedIntoMainWindow, ini_file);
    ZOOM = GetPrivateProfileIntW(L"vis_sdl2 settings", L"Zoom", ZOOM, ini_file);
    g_bSDL2text = GetPrivateProfileIntW(L"vis_sdl2 settings", L"SDL2text", g_bSDL2text, ini_file);
    g_bwavetrumData = GetPrivateProfileIntW(L"vis_sdl2 settings", L"wavetrumData", g_bwavetrumData, ini_file);
    peaks = GetPrivateProfileIntW(L"vis_sdl2 settings", L"sa_peaks", peaks, ini_file);
    GetPrivateProfileStringW(L"vis_sdl2 settings", L"sa_draw", L"", specdraw, sizeof(specdraw), ini_file);
    GetPrivateProfileStringW(L"vis_sdl2 settings", L"sa_bandwidth", L"", bandwidth, sizeof(bandwidth), ini_file);
    GetPrivateProfileStringW(L"vis_sdl2 settings", L"oscstyle", L"", oscstyle, sizeof(oscstyle), ini_file);
    GetPrivateProfileStringW(L"vis_sdl2 settings", L"vudraw", L"", vudraw, sizeof(vudraw), ini_file);
    vu_peaks = GetPrivateProfileIntW(L"vis_sdl2 settings", L"vu_peaks", vu_peaks, ini_file);
    safalloff = GetPrivateProfileIntW(L"vis_sdl2 settings", L"safalloff", safalloff, ini_file);
    pfalloff = GetPrivateProfileIntW(L"vis_sdl2 settings", L"pfalloff", pfalloff, ini_file);
    vupfalloff = GetPrivateProfileIntW(L"vis_sdl2 settings", L"vupfalloff", pfalloff, ini_file);

    setDefaultIfEmpty(specdraw, L"normal");
    setDefaultIfEmpty(bandwidth, L"thick");
    setDefaultIfEmpty(oscstyle, L"lines");
    setDefaultIfEmpty(vudraw, L"normal");
}

void config_write(struct winampVisModule* this_mod)
{
    wchar_t string[32];
    wchar_t ini_file[MAX_PATH];
    config_getinifnW(this_mod, ini_file);

    wsprintfW(string, L"%d", config_x);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"Screen_x", string, ini_file);
    wsprintfW(string, L"%d", config_y);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"Screen_y", string, ini_file);
    wsprintfW(string, L"%d", config_w);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"Screen_w", string, ini_file);
    wsprintfW(string, L"%d", config_h);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"Screen_h", string, ini_file);
    wsprintfW(string, L"%d", g_bEmbedIntoMainWindow);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"embed2main", string, ini_file);
    wsprintfW(string, L"%d", ZOOM);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"Zoom", string, ini_file);
    wsprintfW(string, L"%d", g_bSDL2text);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"SDL2text", string, ini_file);
    wsprintfW(string, L"%d", g_bwavetrumData);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"wavetrumData", string, ini_file);
    wsprintfW(string, L"%d", peaks);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"sa_peaks", string, ini_file);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"sa_draw", specdraw, ini_file);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"sa_bandwidth", bandwidth, ini_file);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"oscstyle", oscstyle, ini_file);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"vudraw", vudraw, ini_file);
    wsprintfW(string, L"%d", vu_peaks);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"vu_peaks", string, ini_file);
    wsprintfW(string, L"%d", safalloff);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"safalloff", string, ini_file);
    wsprintfW(string, L"%d", pfalloff);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"pfalloff", string, ini_file);
    wsprintfW(string, L"%d", vupfalloff);
    WritePrivateProfileStringW(L"vis_sdl2 settings", L"vupfalloff", string, ini_file);
}