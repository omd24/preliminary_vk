#ifndef UNICODE
#define UNICODE
#endif

#define _CRT_SECURE_NO_WARNINGS

// turn off specific warnings
#pragma warning(disable : 5105)     // macro expansion producing 'defined' has undefined behavior
#pragma warning(disable : 4477)     // vk types used in fmt str
#pragma warning(disable : 6302)     // vk types used in fmt str
#pragma warning(disable : 6011)     // derefrencing null pointer?

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

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

// NOTE(omid): Extension functions are not automatically loaded. We have to look up their address ourselves
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                                \
    {                                                                                                           \
        demo.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint);                \
        if (demo.fp##entrypoint == NULL) {                                                                      \
            ERREXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint, "vkGetInstanceProcAddr Failure");    \
        }                                                                                                       \
    }

typedef struct {
    VkInstance                  inst;
    VkDevice                    device;     // logical device
    VkPhysicalDevice            gpu;        // physical device
    int                         gpu_number;

    bool                        validate;
    UINT                        enabled_ext_count;
    UINT                        enabled_layer_count;
    char *                      ext_names[64];
    char *                      enabled_layers[64];

    VkCommandPool               cmd_pool;
    VkCommandBuffer             cmd_buf;

    int                         width;
    int                         height;
    HINSTANCE                   connection;
    HWND                        window;

    VkSurfaceKHR                surface;

    UINT                        queue_family_count;
    VkQueueFamilyProperties *   queue_props;
    UINT                        graphics_queue_family_index;

    VkPresentModeKHR            present_mode;

    // function pointers
    PFN_vkCreateDebugUtilsMessengerEXT          CreateDebugUtilsMessengerEXT;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR    fpGetPhysicalDeviceSurfaceSupportKHR;

    VkDebugUtilsMessengerEXT dbg_messenger;
} Demo;

static VkBool32
debug_messenger_callback (
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT msg_type,
    VkDebugUtilsMessengerCallbackDataEXT const * cb_ptr,
    void * user_data
) {
    TCHAR prefix[64] = _T("");
    TCHAR * message = (TCHAR *)malloc(strlen(cb_ptr->pMessage) + 5000);
    assert(message);
    Demo * demo = (Demo *)user_data;
    UNREFERENCED_PARAMETER(demo);

    //DebugBreak();

    if (msg_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        _tcscat(prefix, _T("VERBOSE : "));
    } else if (msg_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        _tcscat(prefix, _T("INFO : "));
    } else if (msg_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        _tcscat(prefix, _T("WARNING : "));
    } else if (msg_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        _tcscat(prefix, _T("ERROR : "));
    }

    if (msg_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        _tcscat(prefix, _T("GENERAL"));
    } else {
        if (msg_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            _tcscat(prefix, _T("VALIDATION"));
            global_validation_error = 1;
        }
        if (msg_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (msg_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                _tcscat(prefix, _T("|"));
            }
            _tcscat(prefix, _T("PERFORMANCE"));
        }
    }

    _stprintf(
        message, _T("%ls - Message Id Number: %d | Message Id Name: %hs\n\t%hs\n"),
        prefix, cb_ptr->messageIdNumber,
        cb_ptr->pMessageIdName, cb_ptr->pMessage
    );
    if (cb_ptr->objectCount > 0) {
        TCHAR tmp_message[500];
        _stprintf(tmp_message, _T("\n\tObjects - %d\n"), cb_ptr->objectCount);
        _tcscat(message, tmp_message);
        for (uint32_t object = 0; object < cb_ptr->objectCount; ++object) {
            if (NULL != cb_ptr->pObjects[object].pObjectName && strlen(cb_ptr->pObjects[object].pObjectName) > 0) {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %hs, Handle %p, Name \"%hs\"\n"), object,
                          string_VkObjectType(cb_ptr->pObjects[object].objectType),
                          (void *)(cb_ptr->pObjects[object].objectHandle), cb_ptr->pObjects[object].pObjectName);
            } else {
                _stprintf(tmp_message, _T("\t\tObject[%d] - %hs, Handle %p\n"), object,
                          string_VkObjectType(cb_ptr->pObjects[object].objectType),
                          (void *)(cb_ptr->pObjects[object].objectHandle));
            }
            _tcscat(message, tmp_message);
        }
    }
    if (cb_ptr->cmdBufLabelCount > 0) {
        TCHAR tmp_message[500];
        _stprintf(tmp_message, _T("\n\tCommand Buffer Labels - %d\n"), cb_ptr->cmdBufLabelCount);
        _tcscat(message, tmp_message);
        for (uint32_t cmd_buf_label = 0; cmd_buf_label < cb_ptr->cmdBufLabelCount; ++cmd_buf_label) {
            _stprintf(tmp_message, _T("\t\tLabel[%d] - %hs { %f, %f, %f, %f}\n"), cmd_buf_label,
                      cb_ptr->pCmdBufLabels[cmd_buf_label].pLabelName, cb_ptr->pCmdBufLabels[cmd_buf_label].color[0],
                      cb_ptr->pCmdBufLabels[cmd_buf_label].color[1], cb_ptr->pCmdBufLabels[cmd_buf_label].color[2],
                      cb_ptr->pCmdBufLabels[cmd_buf_label].color[3]);
            _tcscat(message, tmp_message);
        }
    }

    if (!global_suppress_popups) MessageBox(NULL, message, _T("Alert"), MB_OK);

    free(message);

    // Don't bail out, but keep going.
    return false;
}
/*
 * Return 1 (true) if all layer names specified in check_names
 * can be found in given layer properties.
 */
static VkBool32
demo_check_layers (UINT check_count, char ** check_names, UINT layer_count, VkLayerProperties * layers) {
    for (UINT i = 0; i < check_count; i++) {
        VkBool32 found = 0;
        for (UINT j = 0; j < layer_count; j++) {
            if (!strcmp(check_names[i], layers[j].layerName)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
            return 0;
        }
    }
    return 1;
}
static void
demo_init (Demo * demo, int w, int h, HINSTANCE win32_hinstance, HWND wnd) {
    _ASSERT_EXPR(demo, _T("Invalid demo pointer"));
    memset(demo, 0, sizeof(demo));
    demo->width = w;
    demo->height = h;
    demo->gpu_number = -1;
    demo->cmd_pool = VK_NULL_HANDLE;
    demo->connection =  win32_hinstance;
    demo->window = wnd;
    demo->present_mode = VK_PRESENT_MODE_FIFO_KHR;
    demo->validate = true;
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
    demo_init(&demo, 500, 500, instance, hwnd);

    VkResult err = VK_NOT_READY;
    UINT instance_ext_count = 0;
    UINT instance_layer_count = 0;
    char * instance_validation_layers [] = {"VK_LAYER_KHRONOS_validation"};

    //
    // Look for validation layer
    //
    VkBool32 validation_found = 0;
    if (demo.validate) {
        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        _ASSERT_EXPR(!err, _T("vkEnumerateInstanceLayerProperties failed"));

        if (instance_layer_count > 0) {
            VkLayerProperties * instance_layers = calloc(instance_layer_count, sizeof(VkLayerProperties));
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
            _ASSERT_EXPR(!err, _T("vkEnumerateInstanceLayerProperties failed"));

            validation_found = demo_check_layers(_countof(instance_validation_layers), instance_validation_layers,
                                                 instance_layer_count, instance_layers);
            if (validation_found) {
                demo.enabled_layer_count = _countof(instance_validation_layers);
                demo.enabled_layers[0] = "VK_LAYER_KHRONOS_validation";
            }
            free(instance_layers);
        }

        if (!validation_found) {
            ERREXIT(
                "vkEnumerateInstanceLayerProperties failed to find required validation layer.\n\n"
                "Please look at the Getting Started guide for additional information.\n",
                "vkCreateInstance Failure");
        }
    }
    //
    // Look for instance extensions
    //
    VkBool32 suface_ext_found = 0;
    VkBool32 platform_surface_ext_found = 0;
    memset(demo.ext_names, 0, sizeof(demo.ext_names));
    err = vkEnumerateInstanceExtensionProperties(NULL, &instance_ext_count, NULL);
    _ASSERT_EXPR(!err, _T("vkEnumerateInstanceExtensionProperties failed"));
    if (instance_ext_count > 0) {
        VkExtensionProperties * instance_exts = calloc(instance_ext_count, sizeof(VkExtensionProperties));
        err = vkEnumerateInstanceExtensionProperties(NULL, &instance_ext_count, instance_exts);
        _ASSERT_EXPR(!err, _T("vkEnumerateInstanceExtensionProperties failed"));
        for (UINT i = 0; i < instance_ext_count; ++i) {
            if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_exts[i].extensionName)) {
                suface_ext_found = 1;
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }
            if (!strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_exts[i].extensionName)) {
                platform_surface_ext_found = 1;
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
            if (!strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_exts[i].extensionName)) {
                if (demo.validate)
                    demo.ext_names[demo.enabled_ext_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
            _ASSERT_EXPR(demo.enabled_ext_count < 64, _T("Enabled extension counts exceeded upperbound!"));
        }
        free(instance_exts);
    }
    if (!suface_ext_found) {
        ERREXIT(
            "vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_SURFACE_EXTENSION_NAME,
            "Do u hav a compatible vk icd installed?"
        );
    }
    if (!platform_surface_ext_found) {
        ERREXIT(
            "vkEnumerateInstanceExtensionProperties failed to find the " VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            "Do u hav a compatible vk icd installed?"
        );
    }
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
    inst_info.enabledLayerCount = demo.enabled_layer_count;
    inst_info.ppEnabledLayerNames = (char const * const *)instance_validation_layers;
    inst_info.enabledExtensionCount = demo.enabled_ext_count;
    inst_info.ppEnabledExtensionNames = (char const * const *)demo.ext_names;

    // NOTE(omid): for vkCreateInstance we need a temp callback 
    // After creating the instance we can register our debug callback
    // using instance-based function

    VkDebugUtilsMessengerCreateInfoEXT dbg_msg_create_info = {0};
    if (demo.validate) {
        dbg_msg_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg_msg_create_info.pNext = NULL;
        dbg_msg_create_info.flags = 0;
        dbg_msg_create_info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg_msg_create_info.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg_msg_create_info.pfnUserCallback = debug_messenger_callback;
        inst_info.pNext = &dbg_msg_create_info;
    }

    err = vkCreateInstance(&inst_info, NULL, &demo.inst);
    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERREXIT(
            "Cannot find a compatible Vulkan installable client driver (ICD)\n"
            "Refer to specs for more info\n",
            "vkCreateInstance failed)"
        );
    } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
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
 /*demo.CreateDebugUtilsMessengerEXT =
     (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo.inst, "vkCreateDebugUtilsMessengerEXT");
 err = demo.CreateDebugUtilsMessengerEXT(demo.inst, &dbg_messenger_create_info, NULL, &demo.dbg_messenger);*/

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


    GET_INSTANCE_PROC_ADDR(demo.inst, GetPhysicalDeviceSurfaceSupportKHR);
    // TODO(omid): enumerate device extensions 

    // TODO(omid): setup VK_EXT_debug_utils function pointers

    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, NULL);
    _ASSERT_EXPR(demo.queue_family_count > 0, "Not enough queue family count");
    demo.queue_props = (VkQueueFamilyProperties *)calloc(demo.queue_family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, demo.queue_props);

    //
    // Init swapchain
    //
    VkWin32SurfaceCreateInfoKHR surface_info = {0};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = NULL;
    surface_info.hinstance = demo.connection;
    surface_info.hwnd = demo.window;
    err = vkCreateWin32SurfaceKHR(demo.inst, &surface_info, NULL, &demo.surface);
    _ASSERT_EXPR(!err, _T("vkCreateWin32SurfaceKHR failed"));
    VkBool32 * supports_presents = (VkBool32 *)calloc(demo.queue_family_count, sizeof(VkBool32));
    for (UINT i = 0; i < demo.queue_family_count; ++i) {
        demo.fpGetPhysicalDeviceSurfaceSupportKHR(demo.gpu, i, demo.surface, &supports_presents[i]);
    }
    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    UINT graphics_qfamid = UINT32_MAX;
    UINT present_qfamid = UINT32_MAX;
    for (UINT i = 0; i < demo.queue_family_count; i++) {
        if ((demo.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphics_qfamid == UINT32_MAX) {
                graphics_qfamid = i;
            }

            if (supports_presents[i] == VK_TRUE) {
                graphics_qfamid = i;
                present_qfamid = i;
                break;
            }
        }
    }
    if (present_qfamid == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (uint32_t i = 0; i < demo.queue_family_count; ++i) {
            if (supports_presents[i] == VK_TRUE) {
                present_qfamid = i;
                break;
            }
        }
    }
    // Generate error if could not find both a graphics and a present queue
    if (graphics_qfamid == UINT32_MAX || present_qfamid == UINT32_MAX) {
        ERREXIT("Could not find both graphics and present queues\n", "Swapchain Initialization Failure");
    }

    //
    // Init [Logical] device
    //
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

    //
    //  Initi command buffer
    //
    if (demo.cmd_pool == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo pool_info = {0};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.pNext = NULL;
        pool_info.queueFamilyIndex = demo.graphics_queue_family_index;
        pool_info.flags = 0;
        err = vkCreateCommandPool(demo.device, &pool_info, NULL, &demo.cmd_pool);
        _ASSERT_EXPR(!err, _T("vkCreateCommandPool failed"));
    }
    VkCommandBufferAllocateInfo cmd_buf_alloc_info = {0};
    cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buf_alloc_info.pNext = NULL;
    cmd_buf_alloc_info.commandPool = demo.cmd_pool;
    cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_alloc_info.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(demo.device, &cmd_buf_alloc_info, &demo.cmd_buf);
    _ASSERT_EXPR(!err, _T("vkAllocateCommandBuffers failed"));

    VkCommandBufferBeginInfo cmd_buf_begin_info = {0};
    cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_begin_info.pNext = NULL;
    cmd_buf_begin_info.flags = 0;
    cmd_buf_begin_info.pInheritanceInfo = NULL;
    err = vkBeginCommandBuffer(demo.cmd_buf, &cmd_buf_begin_info);
    _ASSERT_EXPR(!err, _T("vkBeginCommandBuffer failed"));

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

