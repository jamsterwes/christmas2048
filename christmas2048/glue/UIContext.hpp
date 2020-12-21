#pragma once
#include <imgui.h>

#define GLUE_HOOK_UI(ctx) ImGui::SetCurrentContext(ctx->GetContext())

namespace glue
{
    class IUIContext
    {
    public:
        virtual void NewFrame() = 0;
        virtual void EndAndRenderFrame() = 0;
        virtual ImGuiContext* GetContext() = 0;
        virtual void PushBaseFont() = 0;
        virtual void PushCellFont() = 0;
    };
}