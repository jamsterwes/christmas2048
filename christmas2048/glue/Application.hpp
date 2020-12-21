#pragma once
#include <string>
#include <functional>
#include "UIContext.hpp"

#define GLUE_GL_MAJOR 4
#define GLUE_GL_MINOR 4

#ifdef GLUE_EXPORTS
#define GLUE_API __declspec(dllexport)
#else
#define GLUE_API __declspec(dllimport)
#endif

namespace glue
{
    // -- Application interface
    class IApplication
    {
    public:
        // Window properties

        virtual void setSize(unsigned int width, unsigned int height) = 0;
        virtual void setTitle(std::string title) = 0;
        
        // Style properties
        virtual void setBackground(float r, float g, float b) = 0;

        // Render loop functions

        virtual bool running() = 0;
        virtual void update(std::function<void(IUIContext*)> userFunc = [](IUIContext*){}) = 0;
    };

    // -- Application factory
    extern "C" GLUE_API IApplication* CreateApplication(unsigned int width, unsigned int height, std::string title);
}