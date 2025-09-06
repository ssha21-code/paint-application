#include <windows.h>
#include <string>

#define WS_NONRESIZEABLEMAINWINDOW (WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define WS_CENTEREDSTATIC (WS_VISIBLE | WS_CHILD | SS_CENTER)
#define WS_NORMALSTATIC (WS_VISIBLE | WS_CHILD)

#define ID_PAINTAREA 1001
#define ID_STATIC1 101
#define ID_STATIC2 102
#define ID_STATIC3 103
#define ID_STATIC4 104
#define ID_EXIT 2000
#define ID_HELP 2001
#define ID_ABOUT 2002
#define ID_INSTRUCTIONSBOX 201

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PaintAreaWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK InstructionsDialogProc(HWND, UINT, WPARAM, LPARAM);

void RegisterPaintAreaClass(HINSTANCE hInst);
void RegisterInstructionsDialogClass(HINSTANCE hInst);
void AddControls(HWND, HINSTANCE);
void AddMenus(HWND, HINSTANCE); 
void AddControlsDialog(HWND, HINSTANCE);

constexpr int WINDOW_WIDTH = 900;
constexpr int WINDOW_HEIGHT = 720;

bool is_drawing = false;
bool eraser_in_use = false;
int last_x = 0;
int last_y = 0;
int pen_size = 1;

int nCmdShow;

static bool was_outside = false;

int color_index = 0;
int colorbackground_index = 9;
const COLORREF colors[] = {RGB(0, 0, 0), RGB(255, 0, 0), RGB(255, 127, 39), RGB(255, 242, 0), RGB(0, 255, 0), RGB(0, 162, 255), RGB(0, 0, 255), RGB(163, 73, 164), RGB(255, 255, 255)};
int NUM_COLORS = sizeof(colors) / sizeof(colors[0]);

const COLORREF colors_background[] = {RGB(255, 255, 255), RGB(0, 0, 0), RGB(255, 0, 0), RGB(255, 127, 39), RGB(255, 242, 0), RGB(0, 255, 0), RGB(0, 162, 255), RGB(0, 0, 255), RGB(163, 73, 164), RGB(230, 230, 230)};
int NUM_COLORSBACKGROUND = sizeof(colors_background) / sizeof(colors_background[0]);

COLORREF eraser_color = colors_background[NUM_COLORSBACKGROUND-1];

COLORREF textbackground_color = RGB(112, 128, 144);
HBRUSH hBrushTextBackground = CreateSolidBrush(textbackground_color);

HWND hWnd, hPaintArea, hStatic1, hStatic2, hStatic3, hStatic4;
HFONT hFont1, hFont2;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int nCmdShow) {
    SetProcessDPIAware();   
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hInstance = hInst;
    wc.lpszClassName = L"WINDOW";
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&wc)) {
        return -1;
    }

    RegisterPaintAreaClass(hInst);
    RegisterInstructionsDialogClass(hInst);

    hWnd = CreateWindowW(L"WINDOW", L"Paint Application 1.0", WS_NONRESIZEABLEMAINWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, NULL, NULL
    );

    AddControls(hWnd, hInst);
    AddMenus(hWnd, hInst);

    hPaintArea = CreateWindowW(L"PAINTAREA", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 
        250, 50, 600, 560, hWnd, (HMENU)ID_PAINTAREA, hInst, NULL
    );
    ::nCmdShow = nCmdShow;

    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wp;
            HWND hStatic = (HWND)lp;
            int id = GetDlgCtrlID(hStatic);
            switch (id) {
                case ID_PAINTAREA: {
                    SetTextColor(hdcStatic, colors[0]);
                }
                case ID_STATIC1: {
                    SetBkMode(hdcStatic, TRANSPARENT);
                    return (INT_PTR)GetStockObject(NULL_BRUSH);
                }
                case ID_STATIC2: {
                    SetBkMode(hdcStatic, OPAQUE);
                    SetBkColor(hdcStatic, colors[color_index]);
                    SetTextColor(hdcStatic, colors[color_index] != colors[0] ? RGB(0, 0, 0) : RGB(255, 255, 255));
                    return (INT_PTR)CreateSolidBrush(colors[color_index]);
                }
                case ID_STATIC3: {
                    SetBkMode(hdcStatic, TRANSPARENT);
                    wchar_t buffer[64];
                    swprintf(buffer, 64, L"Pen Size: %d", pen_size);
                    SetWindowTextW(hStatic, buffer);
                    return (INT_PTR)GetStockObject(NULL_BRUSH);
                }
                case ID_STATIC4: {
                    SetBkMode(hdcStatic, TRANSPARENT);
                    wchar_t buffer[64] = L"Eraser in Use: ";
                    wcscat(buffer, !eraser_in_use ? L"False" : L"True");
                    SetWindowTextW(hStatic, buffer);
                    return (INT_PTR)GetStockObject(NULL_BRUSH);
                }
                
            }
            return 0;
        }
        case WM_KEYDOWN: {
            if (wp == 'A') {
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (wp == 'S') {
                if (color_index < NUM_COLORS-1) {
                    color_index++;

                } else {
                    color_index = 0;
                    
                }
                InvalidateRect(hStatic2, NULL, TRUE);
                UpdateWindow(hStatic2);
                
            } else if (wp == 'D') {
                pen_size++;
                InvalidateRect(hStatic3, NULL, TRUE);
                UpdateWindow(hStatic3);
            } else if (wp == 'F') {
                pen_size--;
                if (pen_size == 0) {
                    pen_size++;
                }
                InvalidateRect(hStatic3, NULL, TRUE);
                UpdateWindow(hStatic3);
            } else if (wp == 'Q') {
                colorbackground_index++;
                if (colorbackground_index >= NUM_COLORSBACKGROUND){
                    colorbackground_index = 0;
                }
                eraser_color = colors_background[colorbackground_index];
                InvalidateRect(hWnd, NULL, TRUE);
            } else if (wp == 'W') {
                if (!eraser_in_use) {
                    eraser_color = colors_background[colorbackground_index];
                    eraser_in_use = true;
                } else {
                    eraser_in_use = false;
                }
                InvalidateRect(hStatic4, NULL, TRUE);
                UpdateWindow(hStatic4);
            }
            return 0;
        }
        case WM_COMMAND: {
            switch (LOWORD(wp)) {
                case ID_EXIT: {
                    PostQuitMessage(0);
                    return 0;
                }
                case ID_HELP: {
                    HWND hDialog = CreateWindowW(L"INSTRUCTIONS_DIALOG", L"Instructions . . .", WS_NONRESIZEABLEMAINWINDOW, 
                        CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, NULL, NULL, NULL, NULL
                    );
                    AddControlsDialog(hDialog, NULL);

                    return 0;
                }
            }
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default: 
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}

void RegisterPaintAreaClass(HINSTANCE hInst) {
    WNDCLASSW wc = {0};
    wc.hbrBackground = CreateSolidBrush(colors_background[colorbackground_index]);
    wc.hInstance = hInst;
    wc.lpszClassName = L"PAINTAREA";
    wc.lpfnWndProc = PaintAreaWndProc;
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    
    RegisterClassW(&wc);
}

LRESULT CALLBACK PaintAreaWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT fill_rect = {0, 0, 600, 560};
            FillRect(hdc, &fill_rect, CreateSolidBrush(colors_background[colorbackground_index]));

            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            is_drawing = true;
            last_x = LOWORD(lp);
            last_y = HIWORD(lp);
            SetCapture(hWnd);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (is_drawing) {
                POINT pt = { LOWORD(lp), HIWORD(lp) };

                // get paint area rect in parent coords
                RECT rc;
                GetWindowRect(hPaintArea, &rc);
                ScreenToClient(hWnd, (LPPOINT)&rc.left);
                ScreenToClient(hWnd, (LPPOINT)&rc.right);

                bool outside = !(pt.x >= rc.left && pt.x < rc.right &&
                                 pt.y >= rc.top && pt.y < rc.bottom);

                if (!outside) {
                    if (was_outside) {
                        
                        last_x = pt.x;
                        last_y = pt.y;
                    } else {
                        // normal drawing
                        HDC hdc = GetDC(hWnd);
                        HPEN hPen = CreatePen(PS_SOLID, pen_size, !eraser_in_use ? colors[color_index] : eraser_color);
                        HGDIOBJ oldpen = SelectObject(hdc, hPen);

                        MoveToEx(hdc, last_x, last_y, NULL);
                        LineTo(hdc, pt.x, pt.y);

                        SelectObject(hdc, oldpen);
                        DeleteObject(hPen);
                        ReleaseDC(hWnd, hdc);
                    }
                    last_x = pt.x;
                    last_y = pt.y;
                }

                was_outside = outside;
                return 0;
            }
            break;
        }
        case WM_LBUTTONUP: {
            is_drawing = false;
            ReleaseCapture();
            return 0;
        }
        
        
        case WM_DESTROY: {
            return 1;
        }
        default: {
            return DefWindowProcW(hWnd, msg, wp, lp);
        }
    }
    return 0;
}

void AddControls(HWND hWnd, HINSTANCE hInst) {
    hFont1 = CreateFontW(40, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
    hFont2 = CreateFontW(30, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
    hStatic1 = CreateWindowW(L"STATIC", L"Controls: ", WS_CENTEREDSTATIC, 
        25, 50, 200, 50, hWnd, (HMENU)ID_STATIC1, hInst, NULL
    );
    SendMessageW(hStatic1, WM_SETFONT, (WPARAM)hFont1, (LPARAM)TRUE);
    hStatic2 = CreateWindowW(L"STATIC", L"Brush Color", WS_CENTEREDSTATIC | WS_BORDER, 
        25, 150, 200, 50, hWnd, (HMENU)ID_STATIC2, hInst, NULL
    );
    SendMessageW(hStatic2, WM_SETFONT, (WPARAM)hFont2, (LPARAM)TRUE);
    hStatic3 = CreateWindowW(L"STATIC", L"Pen Size: ", WS_NORMALSTATIC, 
        25, 220, 200, 50, hWnd, (HMENU)ID_STATIC3, hInst, NULL
    );
    SendMessageW(hStatic3, WM_SETFONT, (WPARAM)hFont2, (LPARAM)TRUE);
    hStatic4 = CreateWindowW(L"STATIC", L"Eraser in Use: ", WS_NORMALSTATIC, 
        25, 270, 200, 50, hWnd, (HMENU)ID_STATIC4, hInst, NULL
    );
    SendMessageW(hStatic4, WM_SETFONT, (WPARAM)hFont2, (LPARAM)TRUE);

}

void AddMenus(HWND hWnd, HINSTANCE hInst) {
    HMENU hMenuBar = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hHelpMenu = CreatePopupMenu();
    
    AppendMenuW(hFileMenu, MF_STRING, ID_EXIT, L"Exit");
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP, L"Instructions...");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"Help");
    SetMenu(hWnd, hMenuBar);
}

LRESULT CALLBACK InstructionsDialogProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CLOSE: {
            DestroyWindow(hWnd);
            return 0;
        }
        default:
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}

void RegisterInstructionsDialogClass(HINSTANCE hInst) {
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hInstance = hInst;
    wc.lpfnWndProc = InstructionsDialogProc;
    wc.lpszClassName = L"INSTRUCTIONS_DIALOG";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassW(&wc);
}

void AddControlsDialog(HWND hWnd, HINSTANCE hInst) {
    HFONT hFont1 = CreateFontW(22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
    wchar_t info[] = L"Instructions for using paint-application\r\n\r\n\
    Keep your left hand on left side of keyboard and right hand on mouse for drawing.\r\n\r\n\
    A -> Clears screen\r\n\
    S -> Changes Brush Color\r\n\
    D -> Increases Brush Width\r\n\
    F -> Decreases Brush Width\r\n\
    Q -> Changes background color of drawing window (Also clears the screen)\r\n\
    W -> Toggles Eraser\r\n\
    \r\n\r\n\
    Provides ease of use for touch typists\r\n\
    ";
    HWND hInformationBox = CreateWindowW(L"EDIT", info, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL | WS_HSCROLL | WS_BORDER | ES_READONLY, 
        0, 0, 600, 570, hWnd, NULL, hInst, NULL
    );
    SendMessageW(hInformationBox, WM_SETFONT, (WPARAM)hFont1, TRUE);
}
