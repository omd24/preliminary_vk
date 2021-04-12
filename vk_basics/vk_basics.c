#ifndef UNICODE
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS

// turn off specific warnings
#pragma warning(disable : 5105)     // macro expansion producing 'defined' has undefined behavior
#pragma warning(disable : 4477)     // vk types used in fmt str
#pragma warning(disable : 6302)     // vk types used in fmt str
#pragma warning(disable : 6011)     // derefrencing null pointer?

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

#define ERREXIT(err_msg, err_class)                      \
    do {                                                    \
        if (!global_suppress_popups)                        \
            MessageBox(NULL, _T(err_msg), _T(err_class), MB_OK);    \
    } while (0)

// Vulkan
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>

// Helpers
#include "utils.h"

typedef struct {
    VkInstance                  inst;
    VkDevice                    device;

    int                         width;
    int                         height;

    int                         gpu_number;
    VkPhysicalDevice            gpu;
    UINT                        queue_family_count;
    VkQueueFamilyProperties *   queue_props;
    UINT                        graphics_queue_family_index;

    // function pointers
    PFN_vkCreateDebugUtilsMessengerEXT  create_debug_utils_messenger_ext;

    VkDebugUtilsMessengerEXT dbg_messenger;
} Demo;

static VkBool32
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

    _stprintf(
        message, _T("%ls - Message Id Number: %d | Message Id Name: %hs\n\t%hs\n"),
        prefix, pCallbackData->messageIdNumber,
        pCallbackData->pMessageIdName, pCallbackData->pMessage
    );
    if (pCallbackData->objectCount > 0) {
        TCHAR tmp_message[500];
        _stprintf(tmp_message, _T("\n\tObjects - %d\n"), pCallbackData->objectCount);
        _tcscat(message, tmp_message);
        for (uint32_t object = 0; object < pCallbackData->objectCount; ++object) {
            if (NULL != pCallbackData->pObjects[object].pObjectName && strlen(pCallbackData->pObjects[object].pObjectName) > 0) {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %hs, Handle %p, Name \"%hs\"\n"), object,
                          string_VkObjectType(pCallbackData->pObjects[object].objectType),
                          (void *)(pCallbackData->pObjects[object].objectHandle), pCallbackData->pObjects[object].pObjectName);
            } else {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %hs, Handle %p\n"), object,
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
            _stprintf(tmp_message, _T("\t\tLabel[%d] - %hs { %f, %f, %f, %f}\n"), cmd_buf_label,
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
static void
demo_init (Demo * demo, int w, int h) {
    _ASSERT_EXPR(demo, _T("Invalid demo pointer"));
    memset(demo, 0, sizeof(demo));
    demo->width = w;
    demo->height = h;
    demo->gpu_number = -1;
}
LRESULT CALLBACK
WindowProc (HWND hwnd, UINT msg_code, WPARAM wparam, LPARAM lparam) {
    LRESULT result = -1;
    switch (msg_code) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        result = 0;
    }break;
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
    demo_init(&demo, 500, 500);
    //
    // Init instance
    //
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

    //VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
    //// VK_EXT_debug_utils style
    //dbg_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //dbg_messenger_create_info.pNext = NULL;
    //dbg_messenger_create_info.flags = 0;
    //dbg_messenger_create_info.messageSeverity =
    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //dbg_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    //    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    //    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    //dbg_messenger_create_info.pfnUserCallback = debug_messenger_callback;
    //dbg_messenger_create_info.pUserData = &demo;
    //inst_info.pNext = &dbg_messenger_create_info;

    err = vkCreateInstance(&inst_info, NULL, &demo.inst);

    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERREXIT(
            "Cannot find a compatible Vulkan installable client driver (ICD)\n"
            "Refer to specs for more info\n",
            "vkCreateInstance failed)"
        );
    } else if (err) {
        ERREXIT(
            "Cannot find a specified extension library.\n"
            "Make sure layer paths are correct\n",
            "vkCreateInstance failed"
        );
    } else if (err) {
        ERREXIT(
            "Something went wrong.\n"
            "Do you have a vulkan installabed ICD?\n",
            "vkCreateInstance failed"
        );
    }
// NOTE(omid): VK_EXT_debug_utils extension not enabled yet 
 /*demo.create_debug_utils_messenger_ext =
     (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo.inst, "vkCreateDebugUtilsMessengerEXT");
 err = demo.create_debug_utils_messenger_ext(demo.inst, &dbg_messenger_create_info, NULL, &demo.dbg_messenger);*/

    //
    // Enumerate physical devices
    //
    UINT gpu_count = 0;
    err = vkEnumeratePhysicalDevices(demo.inst, &gpu_count, NULL);
    if (gpu_count < 1)
        ERREXIT("vkEnumeratePhysicalDevices reported zero device??\n", "vkEnumeratePhysicalDevices failed");
    VkPhysicalDevice * physical_devices = (VkPhysicalDevice *)calloc(gpu_count, sizeof(VkPhysicalDevice));
    err = vkEnumeratePhysicalDevices(demo.inst, &gpu_count, physical_devices);
    _ASSERT_EXPR(!err, _T("Filling physical_devices array failed"));

    //
    // Auto-select suitable device
    //
    if (demo.gpu_number == -1) {
        UINT count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU + 1] = {0};
        VkPhysicalDeviceProperties device_props = {0};
        for (UINT i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &device_props);
            ++count_device_type[device_props.deviceType];
        }
        VkPhysicalDeviceType search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_CPU]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_CPU;
        } else if (count_device_type[VK_PHYSICAL_DEVICE_TYPE_OTHER]) {
            search_for_device_type = VK_PHYSICAL_DEVICE_TYPE_OTHER;
        }
        for (UINT i = 0; i < gpu_count; i++) {
            vkGetPhysicalDeviceProperties(physical_devices[i], &device_props);
            if (device_props.deviceType == search_for_device_type) {
                demo.gpu_number = i;
                break;
            }
        }
    }
    _ASSERT_EXPR(demo.gpu_number >= 0, _T("Could not select device"));
    if (physical_devices)
        demo.gpu = physical_devices[demo.gpu_number];
    {
        VkPhysicalDeviceProperties device_props = {0};
        vkGetPhysicalDeviceProperties(demo.gpu, &device_props);
        TCHAR buf[250];
        _stprintf(buf, _T("Selected gpu %d: %hs, type: %u\n"), demo.gpu_number, device_props.deviceName, device_props.deviceType);
        OutputDebugString(buf);
    }
    free(physical_devices);

    // TODO(omid): enumerate device extensions 

    // TODO(omid): setup VK_EXT_debug_utils function pointers

    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, NULL);
    _ASSERT_EXPR(demo.queue_family_count > 0, "Not enough queue family count");
    demo.queue_props = (VkQueueFamilyProperties *)calloc(demo.queue_family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, demo.queue_props);
    //
    // Search for a graphics in the array of queue families
    //
    UINT graphics_qfamid = UINT32_MAX;
    for (UINT i = 0; i < demo.queue_family_count; i++) {
        if ((demo.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphics_qfamid == UINT32_MAX) {
                graphics_qfamid = i;
                break;
            }
        }
    }
    _ASSERT_EXPR(graphics_qfamid != UINT32_MAX, _T("Didnt found graphics queue family index"));
    demo.graphics_queue_family_index = graphics_qfamid;

    float queue_priorites[1] = {0.0f};
    VkDeviceQueueCreateInfo queue_info = {0};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueFamilyIndex = demo.graphics_queue_family_index;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = queue_priorites;

    VkDeviceCreateInfo device_info = {0};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = 0;
    device_info.ppEnabledExtensionNames = NULL;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = NULL;
    device_info.pEnabledFeatures = NULL;

    err = vkCreateDevice(demo.gpu, &device_info, NULL, &demo.device);
    _ASSERT_EXPR(!err, _T("vkCreateDevice failed"));

#pragma endregion

#pragma region Main Loop
    // Run the message loop.
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#pragma endregion

    // TODO(omid): Cleanups 
    return(0);
}

