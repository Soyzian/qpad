// winblur.h
#pragma once
#include <Windows.h>
#include <QColor>

enum class AccentState : int {
    Disabled                     = 0,
    EnableBlurBehind             = 3,
    EnableAcrylicBlurBehind      = 4,  // Windows 11
};

struct AccentPolicy {
    AccentState  AccentState_;
    DWORD        AccentFlags;
    DWORD        GradientColor;    // 0xAABBGGRR
    DWORD        AnimationId;
};

struct WindowCompositionAttributeData {
    DWORD  Attribute;
    PVOID  Data;
    SIZE_T SizeOfData;
};

bool enableAcrylicBlur(HWND hwnd, QColor backdropColor = QColor(255,255,255, 60));
