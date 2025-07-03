// winblur.cpp
#include "winblur.h"

using FnSetWindowCompositionAttribute = BOOL (WINAPI*)(HWND, WindowCompositionAttributeData*);

bool enableAcrylicBlur(HWND hwnd, QColor color)
{
    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser) return false;

    auto fn = reinterpret_cast<FnSetWindowCompositionAttribute>(
        GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    if (!fn) return false;

    AccentPolicy policy = {};
    policy.AccentState_  = AccentState::EnableAcrylicBlurBehind;
    policy.GradientColor = (color.alpha() << 24) |
                           (color.blue()  << 16) |
                           (color.green() <<  8) |
                           (color.red());
    policy.AccentFlags  = 0;  // opcionalmente puedes habilitar corner rounding

    WindowCompositionAttributeData data = {};
    data.Attribute = 19;  // WCA_ACCENT_POLICY
    data.Data      = &policy;
    data.SizeOfData= sizeof(policy);

    return fn(hwnd, &data);
}
