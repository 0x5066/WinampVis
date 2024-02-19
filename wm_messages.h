int ZOOM = 7;
bool g_bEmbedIntoMainWindow = false;  // Global variable to store checkbox state
bool g_bSDL2text = true;
bool g_bwavetrumData = false;
int vinit = 0;
int c_zoom = ZOOM;
bool peaks = true;
wchar_t specdraw[] = L"normal";
wchar_t bandwidth[] = L"thick";
int safalloff = 3;
int pfalloff = 4;

wchar_t oscstyle[] = L"lines";

wchar_t vudraw[] = L"normal";
bool vu_peaks = true;
int vupfalloff = 3;

// Declaration of DialogProc function
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void EnableControls(HWND hwndDlg) {
    HWND hSlider = GetDlgItem(hwndDlg, IDC_ZOOM_TRACKBAR);
    // Enable/disable controls based on embedding and text checkbox state
    EnableWindow(hSlider, !g_bEmbedIntoMainWindow);
    EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT_CHECKBOX), !g_bEmbedIntoMainWindow);
    EnableWindow(GetDlgItem(hwndDlg, IDC_ZOOM_LABEL), !g_bEmbedIntoMainWindow);
}

// Dialog procedure
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hTrackBar;

    switch (uMsg) {
    case WM_INITDIALOG:
        // Initialize controls and variables
        hTrackBar = GetDlgItem(hwndDlg, IDC_ZOOM_TRACKBAR);
        SendMessage(hTrackBar, TBM_SETRANGE, FALSE, MAKELPARAM(1, 8));
        SendMessage(hTrackBar, TBM_SETPOS, TRUE, ZOOM);

        // Initialize checkbox states
        CheckDlgButton(hwndDlg, IDC_EMBED_CHECKBOX, g_bEmbedIntoMainWindow ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_TEXT_CHECKBOX, g_bSDL2text ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_WAVETRUM_CHECKBOX, g_bwavetrumData ? BST_CHECKED : BST_UNCHECKED);

        // Initialize zoom label
        wchar_t labelText[128];
        swprintf(labelText, L"Zoom Factor: %d", ZOOM);
        SetDlgItemTextW(hwndDlg, IDC_ZOOM_LABEL, labelText);

        // Handle initialization state
        if (vinit == 0) {
            // Disable controls and display error message
            EnableWindow(hTrackBar, FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_EMBED_CHECKBOX), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT_CHECKBOX), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ZOOM_LABEL), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ZOOM_TRACKBAR), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_WAVETRUM_CHECKBOX), FALSE);
            MessageBoxW(hwndDlg, L"Plugin not running, configuration disabled...", L"vis_sdl2", MB_OK | MB_ICONERROR);
            EndDialog(hwndDlg, TRUE);
        }
        if (g_bEmbedIntoMainWindow) {
            EnableWindow(GetDlgItem(hwndDlg, IDC_ZOOM_LABEL), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ZOOM_TRACKBAR), FALSE);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OK_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED) {
                // "OK" button clicked, close the dialog
                EndDialog(hwndDlg, TRUE);
                return TRUE;
            }
            break;

        case IDC_EMBED_CHECKBOX:
            // Checkbox state changed, update global variable and control state
            g_bEmbedIntoMainWindow = IsDlgButtonChecked(hwndDlg, IDC_EMBED_CHECKBOX) == BST_CHECKED;
            EnableWindow(hTrackBar, !g_bEmbedIntoMainWindow);
            return TRUE;

        case IDC_TEXT_CHECKBOX:
            // Checkbox state changed, update global variable
            g_bSDL2text = IsDlgButtonChecked(hwndDlg, IDC_TEXT_CHECKBOX) == BST_CHECKED;
            return TRUE;

        case IDC_WAVETRUM_CHECKBOX:
            // Checkbox state changed, update global variable
            g_bwavetrumData = IsDlgButtonChecked(hwndDlg, IDC_WAVETRUM_CHECKBOX) == BST_CHECKED;
            return TRUE;
        }
        break;

    case WM_HSCROLL:
        if ((HWND)lParam == hTrackBar && !g_bEmbedIntoMainWindow) {
            // Slider position changed, update zoom
            int pos = SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
            ZOOM = pos;

            // Update zoom label
            swprintf(labelText, L"Zoom Factor: %d", ZOOM);
            SetDlgItemTextW(hwndDlg, IDC_ZOOM_LABEL, labelText);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        // Handle close events
        EndDialog(hwndDlg, TRUE);
        return TRUE;

        // Handle other messages if needed

    default:
        break;
    }

    return FALSE;
}

INT_PTR CALLBACK AnalyzerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hSAFalloffBar;
    static HWND hPFalloffBar;
    switch (uMsg) {
        case WM_INITDIALOG:
            // Initialize the peaks checkbox
            CheckDlgButton(hwndDlg, IDC_SHOW_PEAKS_CHECKBOX, peaks ? BST_CHECKED : BST_UNCHECKED);
            
            // Initialize the specdraw radio buttons
            if (wcscmp(specdraw, L"normal") == 0) {
                CheckRadioButton(hwndDlg, IDC_NORMAL_RADIO, IDC_LINE_RADIO, IDC_NORMAL_RADIO);
            } else if (wcscmp(specdraw, L"fire") == 0) {
                CheckRadioButton(hwndDlg, IDC_NORMAL_RADIO, IDC_LINE_RADIO, IDC_FIRE_RADIO);
            } else if (wcscmp(specdraw, L"line") == 0) {
                CheckRadioButton(hwndDlg, IDC_NORMAL_RADIO, IDC_LINE_RADIO, IDC_LINE_RADIO);
            }

            // Initialize the bandwidth radio buttons
            if (wcscmp(bandwidth, L"thick") == 0) {
                CheckRadioButton(hwndDlg, IDC_THICK_RADIO, IDC_THICK_RADIO, IDC_THICK_RADIO);
            } else if (wcscmp(bandwidth, L"thin") == 0) {
                CheckRadioButton(hwndDlg, IDC_THIN_RADIO, IDC_THIN_RADIO, IDC_THIN_RADIO);
            }

            hSAFalloffBar = GetDlgItem(hwndDlg, IDC_FALLOFF_SPEED_SLIDER);
            SendMessage(hSAFalloffBar, TBM_SETRANGE, FALSE, MAKELPARAM(1, 5));
            SendMessage(hSAFalloffBar, TBM_SETPOS, TRUE, safalloff);

            hPFalloffBar = GetDlgItem(hwndDlg, IDC_PEAK_FALLOFF_SPEED_SLIDER);
            SendMessage(hPFalloffBar, TBM_SETRANGE, FALSE, MAKELPARAM(1, 5));
            SendMessage(hPFalloffBar, TBM_SETPOS, TRUE, pfalloff);
            return TRUE;

        case WM_COMMAND:
            // Handle button clicks, etc.
            switch (LOWORD(wParam)) {
                case IDC_SHOW_PEAKS_CHECKBOX:
                    // Checkbox state changed, update global variable
                    peaks = IsDlgButtonChecked(hwndDlg, IDC_SHOW_PEAKS_CHECKBOX) == BST_CHECKED;
                    return TRUE;
                case IDC_NORMAL_RADIO:
                    // User clicked on 'normal' radio button
                    wcscpy(specdraw, L"normal");
                    break;
                case IDC_FIRE_RADIO:
                    // User clicked on 'fire' radio button
                    wcscpy(specdraw, L"fire");
                    break;
                case IDC_LINE_RADIO:
                    // User clicked on 'line' radio button
                    wcscpy(specdraw, L"line");
                    break;
                case IDC_THIN_RADIO:
                    // User clicked on 'thin' radio button
                    wcscpy(bandwidth, L"thin");
                    break;
                case IDC_THICK_RADIO:
                    // User clicked on 'thick' radio button
                    wcscpy(bandwidth, L"thick");
                    break;
            }
            break;

        case WM_NOTIFY:
            // Handle notifications from child controls, such as tab changes
            break;

        case WM_HSCROLL:
            if ((HWND)lParam == hSAFalloffBar) {
                // Slider position changed, update zoom
                int pos = SendMessage(hSAFalloffBar, TBM_GETPOS, 0, 0);
                safalloff = pos;
                return TRUE;
            }
            if ((HWND)lParam == hPFalloffBar) {
                // Slider position changed, update zoom
                int pos = SendMessage(hPFalloffBar, TBM_GETPOS, 0, 0);
                pfalloff = pos;
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;

        default:
            return FALSE;
    }
    return FALSE;
}

INT_PTR CALLBACK OscDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hVUPFalloffBar;
    switch (uMsg) {
        case WM_INITDIALOG:
            // Initialize the oscstyle radio buttons
            if (wcscmp(oscstyle, L"lines") == 0) {
                CheckDlgButton(hwndDlg, IDC_OSC_LINES_RADIO, BST_CHECKED);
            } else if (wcscmp(oscstyle, L"solid") == 0) {
                CheckDlgButton(hwndDlg, IDC_OSC_SOLID_RADIO, BST_CHECKED);
            } else if (wcscmp(oscstyle, L"dots") == 0) {
                CheckDlgButton(hwndDlg, IDC_OSC_DOTS_RADIO, BST_CHECKED);
            }
            return TRUE;

        case WM_COMMAND:
            // Handle button clicks, etc.
            switch (LOWORD(wParam)) {
                case IDC_OSC_LINES_RADIO:
                    wcscpy(oscstyle, L"lines");
                    break;
                case IDC_OSC_SOLID_RADIO:
                    wcscpy(oscstyle, L"solid");
                    break;
                case IDC_OSC_DOTS_RADIO:
                    wcscpy(oscstyle, L"dots");
                    break;
            }
            break;

        case WM_NOTIFY:
            // Handle notifications from child controls, such as tab changes
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;

        default:
            return FALSE;
    }
    return FALSE;
}

INT_PTR CALLBACK VUDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hVUPFalloffBar;
    switch (uMsg) {
        case WM_INITDIALOG:
            // Initialize the peaks checkbox
            CheckDlgButton(hwndDlg, IDC_SHOW_VUPEAKS_CHECKBOX, vu_peaks ? BST_CHECKED : BST_UNCHECKED);
            
            // Initialize the specdraw radio buttons
            if (wcscmp(vudraw, L"normal") == 0) {
                CheckRadioButton(hwndDlg, IDC_COLOURING_NORMAL_RADIO, IDC_COLOURING_LINE_RADIO, IDC_COLOURING_NORMAL_RADIO);
            } else if (wcscmp(vudraw, L"fire") == 0) {
                CheckRadioButton(hwndDlg, IDC_COLOURING_NORMAL_RADIO, IDC_COLOURING_LINE_RADIO, IDC_COLOURING_FIRE_RADIO);
            } else if (wcscmp(vudraw, L"line") == 0) {
                CheckRadioButton(hwndDlg, IDC_COLOURING_NORMAL_RADIO, IDC_COLOURING_LINE_RADIO, IDC_COLOURING_LINE_RADIO);
            }

            hVUPFalloffBar = GetDlgItem(hwndDlg, IDC_PEAK_FALLOFF_TRACKBAR);
            SendMessage(hVUPFalloffBar, TBM_SETRANGE, FALSE, MAKELPARAM(1, 5));
            SendMessage(hVUPFalloffBar, TBM_SETPOS, TRUE, vupfalloff);
            return TRUE;

        case WM_COMMAND:
            // Handle button clicks, etc.
            switch (LOWORD(wParam)) {
                case IDC_SHOW_VUPEAKS_CHECKBOX:
                    // Checkbox state changed, update global variable
                    vu_peaks = IsDlgButtonChecked(hwndDlg, IDC_SHOW_VUPEAKS_CHECKBOX) == BST_CHECKED;
                    return TRUE;
                case IDC_COLOURING_NORMAL_RADIO:
                    // User clicked on 'normal' radio button
                    wcscpy(vudraw, L"normal");
                    break;
                case IDC_COLOURING_FIRE_RADIO:
                    // User clicked on 'fire' radio button
                    wcscpy(vudraw, L"fire");
                    break;
                case IDC_COLOURING_LINE_RADIO:
                    // User clicked on 'line' radio button
                    wcscpy(vudraw, L"line");
                    break;
            }
            break;

        case WM_HSCROLL:
            if ((HWND)lParam == hVUPFalloffBar) {
                int pos = SendMessage(hVUPFalloffBar, TBM_GETPOS, 0, 0);
                vupfalloff = pos;
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;

        default:
            return FALSE;
    }
    return FALSE;
}