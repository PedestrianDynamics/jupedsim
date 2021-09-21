/// This file provides wrapper functions around breakage in VTK API versions.
/// If you find yourself in the situation that you need to call different
/// functions for VTK8/9 please add them here.
#pragma once
#include <vtkRenderWindow.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif

template <typename T>
vtkRenderWindow* GetRenderWindow(T* obj) {
#if VTK890
    return obj->renderWindow();
#else
    return obj->GetRenderWindow();
#endif
}
