#include "GR_3/Graphic_3.hpp"
#include <sstream>

Graphic_3* pRenderer_3 = nullptr;

HWND g_hGraphic_3Window = nullptr;

LRESULT CALLBACK InputWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Graphic_3WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
    wc.lpfnWndProc = InputWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("InputWindowClass");
    RegisterClassEx(&wc);

    HWND hInputWnd = CreateWindow(
        TEXT("InputWindowClass"),
        TEXT("Choose graphic's number"),
        WS_OVERLAPPEDWINDOW,
        100, 100,
        350, 200,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    CreateWindow(
        TEXT("STATIC"),
        TEXT("Enter graphic's number"),
        WS_CHILD | WS_VISIBLE,
        10, 10,
        300, 20,
        hInputWnd,
        NULL,
        hInstance,
        NULL
    );

    CreateWindow(
        TEXT("EDIT"),
        TEXT("1"),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        10, 40,
        300, 30,
        hInputWnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    CreateWindow(
        TEXT("BUTTON"),
        TEXT("Build a graph"),
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 80,
        300, 30,
        hInputWnd,
        (HMENU)2,
        hInstance,
        NULL
    );

    ShowWindow(hInputWnd, nCmdShow);
    UpdateWindow(hInputWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (pRenderer_3) delete pRenderer_3;
    return (int)msg.wParam;
}

void CreateGraphic_3Window(HINSTANCE hInstance, double a, double b) {
    if (g_hGraphic_3Window) {
        DestroyWindow(g_hGraphic_3Window);
        g_hGraphic_3Window = nullptr;
    }

    if (pRenderer_3) {
        delete pRenderer_3;
        pRenderer_3 = nullptr;
    }

    WNDCLASSEX wc = {sizeof(WNDCLASSEX)};
    wc.lpfnWndProc = Graphic_3WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("Graphic_3WindowClass");
    RegisterClassEx(&wc);

    g_hGraphic_3Window = CreateWindow(
        TEXT("Graphic_3WindowClass"),
        TEXT("3D visualization of the torus"),
        WS_OVERLAPPEDWINDOW,
        450, 100,
        800, 800,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    pRenderer_3 = new Graphic_3(800, 800);
    pRenderer_3->ChangeParameters(std::max(2.0, a), std::max(0.5, b));

    char* temp_title = new char[100];
    sprintf(temp_title, "Torus: a = %.1lf, b = %.1lf", pRenderer_3->GetA(), pRenderer_3->GetB());
    int lenght = MultiByteToWideChar(CP_UTF8, 0, temp_title, -1, NULL, 0);
    wchar_t* title = new wchar_t[lenght];
    MultiByteToWideChar(CP_UTF8, 0, temp_title, -1, title, lenght);
    SetWindowText(g_hGraphic_3Window, title);

    ShowWindow(g_hGraphic_3Window, SW_SHOW);
    UpdateWindow(g_hGraphic_3Window);

    delete temp_title;
    delete title;
    return;
}

LRESULT CALLBACK InputWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            if (LOWORD(wParam) == 2) {
                wchar_t* buffer = new wchar_t[256];
                GetWindowText(GetDlgItem(hWnd, 1), buffer, 256);
                int number = _wtoi(buffer);
                switch (number) {
                    case 3: {
                        CreateGraphic_3Window((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), 2.0, 0.5);
                        break;
                    }

                    default: {
                        MessageBox(hWnd, TEXT("Incorrect input! Enter a number 3. While..."), TEXT("Error"), MB_OK | MB_ICONERROR);
                        break;
                    }
                }
                delete buffer;
            }
            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }

        default: {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }
    return 0;
}

LRESULT CALLBACK Graphic_3WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rect;
            GetClientRect(hWnd, &rect);
            HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rect, whiteBrush);
            DeleteObject(whiteBrush);

            pRenderer_3->Draw(hdc);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_LBUTTONDOWN: {
            pRenderer_3->StartDrag(LOWORD(lParam), HIWORD(lParam));
            SetCapture(hWnd);
            break;
        }

        case WM_LBUTTONUP: {
            pRenderer_3->EndDrag();
            ReleaseCapture();
            break;
        }

        case WM_MOUSEMOVE: {
            pRenderer_3->UpdateDrag(LOWORD(lParam), HIWORD(lParam));
            if (pRenderer_3->IsDragging()) {
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }

        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            double newScale = pRenderer_3->GetScale() * (delta > 0 ? 1.1 : 0.9);
            newScale = std::max(10.0, std::min(200.0, newScale));
            pRenderer_3->SetScale(newScale);
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        case WM_KEYDOWN: {
            bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;
            double currentA = pRenderer_3->GetA();
            double currentB = pRenderer_3->GetB();

            if (wParam == VK_ADD) {
                if (shiftPressed) {
                    if (currentB < 2.0) currentB += 0.1;
                }
                else if (currentA < 4.9) currentA += 0.3;
                pRenderer_3->ChangeParameters(currentA, currentB);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else if (wParam == VK_SUBTRACT) {
                if (shiftPressed) {
                    if (currentB > 0.5) currentB -= 0.1;
                }
                else if (currentA > 2.0) currentA -= 0.3;
                pRenderer_3->ChangeParameters(currentA, currentB);
                InvalidateRect(hWnd, NULL, TRUE);
            }

            char* temp_title = new char[100];
            sprintf(temp_title, "Torus: a = %.1lf, b = %.1lf", pRenderer_3->GetA(), pRenderer_3->GetB());
            int lenght = MultiByteToWideChar(CP_UTF8, 0, temp_title, -1, NULL, 0);
            wchar_t* title = new wchar_t[lenght];
            MultiByteToWideChar(CP_UTF8, 0, temp_title, -1, title, lenght);
            SetWindowText(g_hGraphic_3Window, title);
            delete temp_title;
            delete title;
            break;
        }

        case WM_DESTROY: {
            if (hWnd == g_hGraphic_3Window) {
                g_hGraphic_3Window = nullptr;
                delete pRenderer_3;
                pRenderer_3 = nullptr;
            }
            break;
        }

        default: {
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }
    return 0;
}