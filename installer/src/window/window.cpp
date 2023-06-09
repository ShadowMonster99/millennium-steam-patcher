#include <string>

#include <window/window.hpp>

#include <vendor/imgui/imgui.h>
#include <vendor/imgui/imgui_internal.h>
#include <vector>

#include <iostream>

//Task bar utility
#include <ShObjIdl_core.h>

//includes default font used by app
#include <strsafe.h>
#include <window/memory.h>

#pragma comment(lib, "winmm.lib")

void SetTheme();

static bool WindowOpen = true;
bool OverlayShowing = true;

struct OverlayWindow {
    WNDCLASSEX WindowClass;
    HWND Hwnd = NULL;
    LPCSTR Name = NULL;
}
Overlay;
struct DirectX9Interface {
    LPDIRECT3D9 IDirect3D9 = NULL;
    LPDIRECT3DDEVICE9 pDevice = NULL;
    D3DPRESENT_PARAMETERS pParameters;
    MSG Message;
}
DirectX9;
struct AppInfo {
    std::string Title;
    uint32_t width = { 1215 }, height = { 850 };
}
appinfo;

void WindowClass::WindowTitle(char* name) { 
    appinfo.Title = name;
}

void WindowClass::WindowDimensions(ImVec2 dimensions) {
    appinfo.width = dimensions.x;
    appinfo.height = dimensions.y;
}

void WindowClass::StartApplication(std::string path)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));

    si.cb = sizeof(si);

    CreateProcess(path.c_str(), 0, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void CenterModal(ImVec2 size)
{
	RECT rect;
	GetWindowRect(Overlay.Hwnd, &rect);

	ImGui::SetNextWindowPos
	(ImVec2((rect.left + (((rect.right - rect.left) / 2) - (size.x / 2))), (rect.top + (((rect.bottom - rect.top) / 2) - (size.y / 2)))));
	ImGui::SetNextWindowSize(ImVec2(size.x, size.y));
}

bool CreateDeviceD3D(HWND hWnd) {
    if ((DirectX9.IDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
        return false;
    }
    ZeroMemory(&DirectX9.pParameters, sizeof(DirectX9.pParameters));
    DirectX9.pParameters.Windowed = TRUE;
    DirectX9.pParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    DirectX9.pParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    DirectX9.pParameters.EnableAutoDepthStencil = TRUE;
    DirectX9.pParameters.AutoDepthStencilFormat = D3DFMT_D16;
    DirectX9.pParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    
    if (DirectX9.IDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DirectX9.pParameters, &DirectX9.pDevice) < 0) 
    {
        return false;
    }
    return true;
}
 
void ClearAll()
{
    if (DirectX9.pDevice) { DirectX9.pDevice->Release(); DirectX9.pDevice = NULL; }
    if (DirectX9.IDirect3D9) { DirectX9.IDirect3D9->Release(); DirectX9.IDirect3D9 = NULL; }
    UnregisterClass(Overlay.WindowClass.lpszClassName, Overlay.WindowClass.hInstance);
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    if (DirectX9.pDevice->Reset(&DirectX9.pParameters) == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

void Center(float avail_width, float element_width, float padding)
{
    ImGui::SameLine((avail_width / 2) - (element_width / 2) + padding);
}

void Center_Text(const char* format, float spacing, ImColor color) {
    Center(ImGui::GetContentRegionAvail().x, ImGui::CalcTextSize(format).x);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + spacing);
    ImGui::TextColored(color.Value, format);
}
static ImVec2 ScreenRes, WindowPos;

bool Application::Create(std::string& m_window_title, void (*Handler)(void))
{
    Overlay.Name = { appinfo.Title.c_str() };
    Overlay.WindowClass = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, Overlay.Name, NULL };

    (ATOM)RegisterClassExA(&Overlay.WindowClass);

    Overlay.Hwnd = CreateWindowA(Overlay.Name, Overlay.Name, WS_POPUP, 0, 0, appinfo.width, appinfo.height, NULL, NULL, Overlay.WindowClass.hInstance, NULL);
    if (CreateDeviceD3D(Overlay.Hwnd) == FALSE) { ClearAll(); }

    RECT rc;
    GetWindowRect(Overlay.Hwnd, &rc);

    int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

    SetWindowPos(Overlay.Hwnd, HWND_NOTOPMOST, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    ::ShowWindow(Overlay.Hwnd, SW_SHOW);
    ::UpdateWindow(Overlay.Hwnd);

    ImGui::CreateContext();

    ImGuiIO* IO = &ImGui::GetIO();
    IO->Fonts->AddFontFromMemoryTTF(Memory::Poppins, sizeof Memory::Poppins, 16);
    IO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
   
    DirectX9.pParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	DirectX9.pParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

    if (ImGui_ImplWin32_Init(Overlay.Hwnd) && ImGui_ImplDX9_Init(DirectX9.pDevice))
    {
        SetTheme();
        memset((&DirectX9.Message), 0, (sizeof(DirectX9.Message)));

        while (DirectX9.Message.message != WM_QUIT)
        {
            if (PeekMessageA(&DirectX9.Message, NULL, (UINT)0U, (UINT)0U, PM_REMOVE))
            {
                TranslateMessage(&DirectX9.Message);
                DispatchMessageA(&DirectX9.Message);
                continue;
            }

            ImGui_ImplDX9_NewFrame(); ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();

            if (OverlayShowing)
            {
                static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
                ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
                ImGui::SetNextWindowSize(ImVec2(appinfo.width, appinfo.height));

                ImGui::Begin(appinfo.Title.c_str(), &OverlayShowing, flags); Handler(); ImGui::End();
            }
            else exit(1);

            ImGui::EndFrame();

            //cleanup, exit device and close window
            DirectX9.pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

            if (DirectX9.pDevice->BeginScene() >= 0) { ImGui::Render(); ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData()); DirectX9.pDevice->EndScene(); }
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault();
            if (DirectX9.pDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST && DirectX9.pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();
        }
    }
    else return false;
}

bool mousedown = false;
POINT lastLocation;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_LBUTTONDOWN: {

        if (ImGui::IsAnyItemHovered() == false)
        {
            mousedown = true;
            GetCursorPos(&lastLocation);
            RECT rect;
            GetWindowRect(Overlay.Hwnd, &rect);

            lastLocation.x = lastLocation.x - rect.left;
            lastLocation.y = lastLocation.y - rect.top;
        }
        break;
    }
    case WM_LBUTTONUP: {
        mousedown = false;
        break;
    }
    case WM_MOUSEMOVE: {
        if (mousedown && ImGui::IsAnyItemHovered() == false) {
            POINT currentpos;
            GetCursorPos(&currentpos);
            int x = currentpos.x - lastLocation.x;
            int y = currentpos.y - lastLocation.y;

            MoveWindow(Overlay.Hwnd, x, y, appinfo.width, appinfo.height, false);
        }
        break;
    }
    case WM_SIZE:
        if (DirectX9.pDevice != NULL && wParam != SIZE_MINIMIZED) {
            DirectX9.pParameters.BackBufferWidth = LOWORD(lParam);
            DirectX9.pParameters.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void SetTheme()
{
	ImGui::GetStyle().FrameRounding = 2.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;
	ImGui::GetStyle().ItemSpacing = ImVec2(12, 8);
    ImGui::GetStyle().ScrollbarSize = 8.0f;
    ImGui::GetStyle().ScrollbarRounding = 8.0f;

	ImGuiStyle* ImGuiWindowStyle = &ImGui::GetStyle();

	ImGuiWindowStyle->SetTheme[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_Border] = ImVec4(0.18, 0.18, 0.18, 1.00f);
      
	ImGuiWindowStyle->SetTheme[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_WindowBg] = ImVec4(0.10, 0.10, 0.10, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ScrollbarBg] = ImVec4(0.12, 0.12, 0.12, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_PopupBg] = ImVec4(0.10, 0.10, 0.10, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_FrameBgHovered] = ImVec4(0.18, 0.18, 0.18, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_FrameBgActive] = ImVec4(0.18, 0.18, 0.18, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.14f, 0.75f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);

	ImGuiWindowStyle->SetTheme[ImGuiCol_Button] = ImVec4(0.14, 0.14, 0.14, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ButtonHovered] = ImVec4(0.16, 0.16, 0.16, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ButtonActive] = ImVec4(0.16, 0.16, 0.16, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_Tab] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TabHovered] = ImVec4(0.16, 0.16, 0.16, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TabActive] = ImVec4(0.16, 0.16, 0.16, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	ImGuiWindowStyle->SetTheme[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
}
