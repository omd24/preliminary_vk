#ifndef UNICODE
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS

// turn off specific warnings
#pragma warning(disable : 5105)     // macro expansion producing 'defined' has undefined behavior
#pragma warning(disable : 4477)     // vk types used in fmt str
#pragma warning(disable : 6302)     // vk types used in fmt str

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

// _ASSERT_EXPR macro
#include <crtdbg.h>
#pragma comment(lib, "ucrtd.lib")

// Linker subsystem option
#pragma comment(linker, "/subsystem:windows")

// Global variables
bool global_suppress_popups = false;
static int global_validation_error = 0;

// Debugging tools
#if !defined(NDEBUG) && !defined(_DEBUG)
#error "Define at least one."
#elif defined(NDEBUG) && defined(_DEBUG)
#error "Define at most one."
#endif

#define ERROR_EXIT(err_msg, err_class)                      \
    do {                                                    \
        if (global_suppress_popups)                         \
            MessageBox(NULL, err_msg, err_class, MB_OK);    \
    } while (0)

// Vulkan
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

// Helpers
#include "utils.h"

typedef struct {
    VkInstance inst;

    // function pointers
    PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger_ext;

    VkDebugUtilsMessengerEXT dbg_messenger;
} Demo;

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_messenger_callback (
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData,
    void * pUserData
) {
    TCHAR prefix[64] = _T("");
    TCHAR * message = (TCHAR *)malloc(strlen(pCallbackData->pMessage) + 5000);
    assert(message);
    Demo * demo = (Demo *)pUserData;
    UNREFERENCED_PARAMETER(demo);

    DebugBreak();


    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        _tcscat(prefix, _T("VERBOSE : "));
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        _tcscat(prefix, _T("INFO : "));
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        _tcscat(prefix, _T("WARNING : "));
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        _tcscat(prefix, _T("ERROR : "));
    }

    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        _tcscat(prefix, _T("GENERAL"));
    } else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            _tcscat(prefix, _T("VALIDATION"));
            global_validation_error = 1;
        }
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                _tcscat(prefix, _T("|"));
            }
            _tcscat(prefix, _T("PERFORMANCE"));
        }
    }

    _stprintf(message, _T("%s - Message Id Number: %d | Message Id Name: %s\n\t%s\n"), prefix, pCallbackData->messageIdNumber,
            pCallbackData->pMessageIdName, pCallbackData->pMessage);
    if (pCallbackData->objectCount > 0) {
        TCHAR tmp_message[500];
        _stprintf(tmp_message, _T("\n\tObjects - %d\n"), pCallbackData->objectCount);
        _tcscat(message, tmp_message);
        for (uint32_t object = 0; object < pCallbackData->objectCount; ++object) {
            if (NULL != pCallbackData->pObjects[object].pObjectName && strlen(pCallbackData->pObjects[object].pObjectName) > 0) {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %s, Handle %p, Name \"%s\"\n"), object,
                        string_VkObjectType(pCallbackData->pObjects[object].objectType),
                        (void *)(pCallbackData->pObjects[object].objectHandle), pCallbackData->pObjects[object].pObjectName);
            } else {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %s, Handle %p\n"), object,
                        string_VkObjectType(pCallbackData->pObjects[object].objectType),
                        (void *)(pCallbackData->pObjects[object].objectHandle));
            }
            _tcscat(message, tmp_message);
        }
    }
    if (pCallbackData->cmdBufLabelCount > 0) {
        TCHAR tmp_message[500];
        _stprintf(tmp_message, _T("\n\tCommand Buffer Labels - %d\n"), pCallbackData->cmdBufLabelCount);
        _tcscat(message, tmp_message);
        for (uint32_t cmd_buf_label = 0; cmd_buf_label < pCallbackData->cmdBufLabelCount; ++cmd_buf_label) {
            _stprintf(tmp_message, _T("\t\tLabel[%d] - %s { %f, %f, %f, %f}\n"), cmd_buf_label,
                    pCallbackData->pCmdBufLabels[cmd_buf_label].pLabelName, pCallbackData->pCmdBufLabels[cmd_buf_label].color[0],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[1], pCallbackData->pCmdBufLabels[cmd_buf_label].color[2],
                    pCallbackData->pCmdBufLabels[cmd_buf_label].color[3]);
            _tcscat(message, tmp_message);
        }
    }

    if (!global_suppress_popups) MessageBox(NULL, message, _T("Alert"), MB_OK);

    free(message);

    // Don't bail out, but keep going.
    return false;
}

LRESULT CALLBACK
WindowProc (HWND hwnd, UINT msg_code, WPARAM wparam, LPARAM lparam) {
    LRESULT result = -1;
    switch (msg_code) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        result = 0;
    }break;
    case WM_PAINT: {
        result = 0;
    } break;
    default: {
        result = DefWindowProc(hwnd, msg_code, wparam, lparam);
    } break;
    }
    return result;
}
int WINAPI
WinMain (
    _In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance,
    _In_ LPSTR cmd_line, _In_ int cmd_show
) {
    UNREFERENCED_PARAMETER(prev_instance);
    UNREFERENCED_PARAMETER(cmd_line);

#pragma region Window Setup
    // Register the window class.
    TCHAR classname []  = _T("Vulkan Samples");
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = instance;
    wc.lpszClassName = classname;
    RegisterClass(&wc);

    // Create the window.
    HWND hwnd = CreateWindowEx(
        0,                          // Optional window styles.
        classname,                  // Window class
        _T("Vulkan Basics"),        // Window text
        WS_OVERLAPPEDWINDOW,        // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window
        NULL,       // Menu
        instance,   // Instance handle
        NULL        // Additional application data
    );
    _ASSERT_EXPR(hwnd, _T("Invalid window!"));
    ShowWindow(hwnd, cmd_show);
#pragma endregion

#pragma region Vulkan Initial Setup
    Demo demo = {0};

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "VK Preliminary";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "VK Preliminary";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo inst_info = {0};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledLayerCount = 0;
    inst_info.ppEnabledLayerNames = NULL;
    // NOTE(omid): Enabling "VK_EXT_debug_utils" extention 
    /*inst_info.enabledExtensionCount = 1;
    inst_info.ppEnabledExtensionNames = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;*/
    inst_info.enabledExtensionCount = 0;
    inst_info.ppEnabledExtensionNames = NULL;

    VkResult err = VK_NOT_READY;

    VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
    // VK_EXT_debug_utils style
    dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dbg_messenger_create_info.pNext = NULL;
    dbg_messenger_create_info.flags = 0;
    dbg_messenger_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dbg_messenger_create_info.pfnUserCallback = debug_messenger_callback;
    dbg_messenger_create_info.pUserData = &demo;


    inst_info.pNext = &dbg_messenger_create_info;
    err = vkCreateInstance(&inst_info, NULL, &demo.inst);

   // NOTE(omid): VK_EXT_debug_utils extension not enabled yet 
    /*demo.create_debug_utils_messenger_ext =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo.inst, "vkCreateDebugUtilsMessengerEXT");
    err = demo.create_debug_utils_messenger_ext(demo.inst, &dbg_messenger_create_info, NULL, &demo.dbg_messenger);*/

    ERROR_EXIT(
        _T("Cannot find a compatible Vulkan installable client driver (ICD)\n")
        _T("Refer to specs for more info\n"),
        _T("vkCreateInstance failed)")
    );
#pragma endregion

#pragma region Main Loop
    // Run the message loop.
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#pragma endregion


    return(0);
}

