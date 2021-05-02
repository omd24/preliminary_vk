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

// Allow a maximum of two outstanding presentation operations.
#define FRAME_LAG 2

// NOTE(omid): Extension functions are not automatically loaded. We have to look up their address ourselves
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                                                                \
    {                                                                                                           \
        demo.fp##entrypoint = (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint);                \
        if (demo.fp##entrypoint == NULL) {                                                                      \
            ERREXIT("vkGetInstanceProcAddr failed to find vk" #entrypoint, "vkGetInstanceProcAddr Failure");    \
        }                                                                                                       \
    }

static PFN_vkGetDeviceProcAddr g_gdpa = NULL;

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                                                    \
    {                                                                                                            \
        if (!g_gdpa) g_gdpa = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(demo.inst, "vkGetDeviceProcAddr"); \
        demo.fp##entrypoint = (PFN_vk##entrypoint)g_gdpa(dev, "vk" #entrypoint);                                \
        if (demo.fp##entrypoint == NULL) {                                                                      \
            ERREXIT("vkGetDeviceProcAddr failed to find vk" #entrypoint, "vkGetDeviceProcAddr Failure");        \
        }                                                                                                        \
    }

typedef struct {
    VkImage image;
    VkCommandBuffer cmd;
    VkCommandBuffer graphics_to_present_cmd;
    VkImageView view;
    VkBuffer uniform_buffer;
    VkDeviceMemory uniform_memory;
    void * uniform_memory_ptr;
    VkFramebuffer framebuffer;
    VkDescriptorSet descriptor_set;
} SwapchainImageResources;
typedef struct {
    VkInstance                  inst;
    VkDevice                    device;     // logical device
    VkPhysicalDevice            gpu;        // physical device
    int                         gpu_number;
    VkPhysicalDeviceProperties  gpu_props;

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
    VkFormat                    format;
    VkColorSpaceKHR             color_space;

    VkSurfaceKHR                surface;

    UINT                        queue_family_count;
    VkQueueFamilyProperties *   queue_props;
    VkQueue                     graphics_queue;
    VkQueue                     present_queue;
    uint32_t                    graphics_queue_family_index;
    uint32_t                    present_queue_family_index;
    bool                        separate_present_queue;
    VkPresentModeKHR            present_mode;
    VkSwapchainKHR              swapchain;
    SwapchainImageResources *   swapchain_image_resources;
    UINT                        swapchain_image_count;
    bool                        is_minimized;

    VkFence                     fences[FRAME_LAG];
    int                         frame_index;
    UINT                        cur_frame;
    UINT                        frame_count;


    VkSemaphore                 image_acquired_semaphores[FRAME_LAG];
    VkSemaphore                 draw_complete_semaphores[FRAME_LAG];
    VkSemaphore                 image_ownership_semaphores[FRAME_LAG];

    VkPhysicalDeviceMemoryProperties memory_properties;

    VkDebugUtilsMessengerEXT dbg_messenger;

    // function pointers for debug utils stuff
    PFN_vkCreateDebugUtilsMessengerEXT      CreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT     DestroyDebugUtilsMessengerEXT;
    PFN_vkSubmitDebugUtilsMessageEXT        SubmitDebugUtilsMessageEXT;
    PFN_vkCmdBeginDebugUtilsLabelEXT        CmdBeginDebugUtilsLabelEXT;
    PFN_vkCmdEndDebugUtilsLabelEXT          CmdEndDebugUtilsLabelEXT;
    PFN_vkCmdInsertDebugUtilsLabelEXT       CmdInsertDebugUtilsLabelEXT;
    PFN_vkSetDebugUtilsObjectNameEXT        SetDebugUtilsObjectNameEXT;

    // function pointers for gpu / instance-based stuff
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR        fpGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR   fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR        fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR   fpGetPhysicalDeviceSurfacePresentModesKHR;

    // function pointers for [logical] device stuff
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;


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
static VkSurfaceFormatKHR
select_surface_format(VkSurfaceFormatKHR *surface_formats, uint32_t count) {
    VkSurfaceFormatKHR ret = surface_formats[0];
    _ASSERT_EXPR(count > 0, "format count must be greater than 0");
    // Prefer non-SRGB formats...
    for (uint32_t i = 0; i < count; i++) {
        const VkFormat format = surface_formats[i].format;

        if (format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM ||
            format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 || format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
            format == VK_FORMAT_R16G16B16A16_SFLOAT) {
            ret = surface_formats[i];
            break;
        }
    }

    // NOTE(omid): 
    // if can't find our preferred formats, we will use the first exposed format.
    // in that case rendering may be incorrect

    return ret;
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
static void
build_backbuffers (Demo * demo) {
    VkResult err;
    VkSwapchainKHR swapchain_old = demo->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surf_capabilities;
    err = demo->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(demo->gpu, demo->surface, &surf_capabilities);
    _ASSERT_EXPR(0 == err, "failed checking the surface capabilities");

    uint32_t present_mode_count;
    err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &present_mode_count, NULL);
    _ASSERT_EXPR(0 == err, "failed to get surface present modes");
    VkPresentModeKHR * present_modes = (VkPresentModeKHR *)malloc(present_mode_count * sizeof(VkPresentModeKHR));
    err = demo->fpGetPhysicalDeviceSurfacePresentModesKHR(demo->gpu, demo->surface, &present_mode_count, present_modes);
    _ASSERT_EXPR(0 == err, "failed to get surface present modes");

    VkExtent2D swapchain_extent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surf_capabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to the size
        // of the images requested, which must fit within the minimum and
        // maximum values.
        swapchain_extent.width = demo->width;
        swapchain_extent.height = demo->height;

        if (swapchain_extent.width < surf_capabilities.minImageExtent.width) {
            swapchain_extent.width = surf_capabilities.minImageExtent.width;
        } else if (swapchain_extent.width > surf_capabilities.maxImageExtent.width) {
            swapchain_extent.width = surf_capabilities.maxImageExtent.width;
        }

        if (swapchain_extent.height < surf_capabilities.minImageExtent.height) {
            swapchain_extent.height = surf_capabilities.minImageExtent.height;
        } else if (swapchain_extent.height > surf_capabilities.maxImageExtent.height) {
            swapchain_extent.height = surf_capabilities.maxImageExtent.height;
        }
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchain_extent = surf_capabilities.currentExtent;
        demo->width = surf_capabilities.currentExtent.width;
        demo->height = surf_capabilities.currentExtent.height;
    }

    if (demo->width == 0 || demo->height == 0) {
        demo->is_minimized = true;
        return;
    } else {
        demo->is_minimized = false;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    // and to have no tearing.  It's a great default present mode to use.
    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    //  There are times when you may wish to use another present mode.  The
    //  following code shows how to select them, and the comments provide some
    //  reasons you may wish to use them.
    //
    // It should be noted that Vulkan 1.0 doesn't provide a method for
    // synchronizing rendering with the presentation engine's display.  There
    // is a method provided for throttling rendering with the display, but
    // there are some presentation engines for which this method will not work.
    // If an application doesn't throttle its rendering, and if it renders much
    // faster than the refresh rate of the display, this can waste power on
    // mobile devices.  That is because power is being spent rendering images
    // that may never be seen.

    // VK_PRESENT_MODE_IMMEDIATE_KHR is for applications that don't care about
    // tearing, or have some way of synchronizing their rendering with the
    // display.
    // VK_PRESENT_MODE_MAILBOX_KHR may be useful for applications that
    // generally render a new presentable image every refresh cycle, but are
    // occasionally early.  In this case, the application wants the new image
    // to be displayed instead of the previously-queued-for-presentation image
    // that has not yet been displayed.
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR is for applications that generally
    // render a new presentable image every refresh cycle, but are occasionally
    // late.  In this case (perhaps because of stuttering/latency concerns),
    // the application wants the late image to be immediately displayed, even
    // though that may mean some tearing.

    if (demo->present_mode != swapchain_present_mode) {
        for (size_t i = 0; i < present_mode_count; ++i) {
            if (present_modes[i] == demo->present_mode) {
                swapchain_present_mode = demo->present_mode;
                break;
            }
        }
    }
    if (swapchain_present_mode != demo->present_mode) {
        ERREXIT("Present mode specified is not supported\n", "Present mode unsupported");
    }

    // Determine the number of VkImages to use in the swap chain.
    // Application desires to acquire 3 images at a time for triple
    // buffering
    uint32_t desired_swapchain_image_count = 3;
    if (desired_swapchain_image_count < surf_capabilities.minImageCount) {
        desired_swapchain_image_count = surf_capabilities.minImageCount;
    }
    // If maxImageCount is 0, we can ask for as many images as we want;
    // otherwise we're limited to maxImageCount
    if ((surf_capabilities.maxImageCount > 0) && (desired_swapchain_image_count > surf_capabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desired_swapchain_image_count = surf_capabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR surf_transform;
    if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        surf_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        surf_transform = surf_capabilities.currentTransform;
    }

    // Find a supported composite alpha mode - one of these is guaranteed to be set
    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < _countof(composite_alpha_flags); i++) {
        if (surf_capabilities.supportedCompositeAlpha & composite_alpha_flags[i]) {
            composite_alpha = composite_alpha_flags[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_ci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = demo->surface,
        .minImageCount = desired_swapchain_image_count,
        .imageFormat = demo->format,
        .imageColorSpace = demo->color_space,
        .imageExtent =
            {
                .width = swapchain_extent.width,
                .height = swapchain_extent.height,
            },
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surf_transform,
        .compositeAlpha = composite_alpha,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .presentMode = swapchain_present_mode,
        .oldSwapchain = swapchain_old,
        .clipped = true,
    };
    uint32_t i;
    err = demo->fpCreateSwapchainKHR(demo->device, &swapchain_ci, NULL, &demo->swapchain);
    _ASSERT_EXPR(0 == err, "failed to create swapchain");

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (swapchain_old != VK_NULL_HANDLE) {
        demo->fpDestroySwapchainKHR(demo->device, swapchain_old, NULL);
    }

    err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchain_image_count, NULL);
    _ASSERT_EXPR(0 == err, "failed to get swapchain images");

    VkImage * swapchain_images = (VkImage *)malloc(demo->swapchain_image_count * sizeof(VkImage));
    err = demo->fpGetSwapchainImagesKHR(demo->device, demo->swapchain, &demo->swapchain_image_count, swapchain_images);
    _ASSERT_EXPR(0 == err, "failed to get swapchain images");

    demo->swapchain_image_resources =
        (SwapchainImageResources *)malloc(sizeof(SwapchainImageResources) * demo->swapchain_image_count);

    for (i = 0; i < demo->swapchain_image_count; i++) {
        VkImageViewCreateInfo color_image_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .format = demo->format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
        };

        demo->swapchain_image_resources[i].image = swapchain_images[i];

        color_image_view.image = demo->swapchain_image_resources[i].image;

        err = vkCreateImageView(demo->device, &color_image_view, NULL, &demo->swapchain_image_resources[i].view);
        _ASSERT_EXPR(0 == err, "failed to create image view");
    }

    if (NULL != swapchain_images) {
        free(swapchain_images);
    }

    if (NULL != present_modes) {
        free(present_modes);
    }
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
        _ASSERT_EXPR(0 == err, _T("vkEnumerateInstanceLayerProperties failed"));

        if (instance_layer_count > 0) {
            VkLayerProperties * instance_layers = calloc(instance_layer_count, sizeof(VkLayerProperties));
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers);
            _ASSERT_EXPR(0 == err, _T("vkEnumerateInstanceLayerProperties failed"));

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
    _ASSERT_EXPR(0 == err, _T("vkEnumerateInstanceExtensionProperties failed"));
    if (instance_ext_count > 0) {
        VkExtensionProperties * instance_exts = calloc(instance_ext_count, sizeof(VkExtensionProperties));
        err = vkEnumerateInstanceExtensionProperties(NULL, &instance_ext_count, instance_exts);
        _ASSERT_EXPR(0 == err, _T("vkEnumerateInstanceExtensionProperties failed"));
        for (UINT i = 0; i < instance_ext_count; ++i) {
            if (0 == strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_exts[i].extensionName)) {
                suface_ext_found = 1;
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
            }
            if (0 == strcmp(VK_KHR_WIN32_SURFACE_EXTENSION_NAME, instance_exts[i].extensionName)) {
                platform_surface_ext_found = 1;
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
            }
            if (0 == strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, instance_exts[i].extensionName)) {
                if (demo.validate)
                    demo.ext_names[demo.enabled_ext_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
            if (0 == strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, instance_exts[i].extensionName)) {
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
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

    //
    // Enumerate physical devices
    //
    UINT gpu_count = 0;
    err = vkEnumeratePhysicalDevices(demo.inst, &gpu_count, NULL);
    if (gpu_count < 1)
        ERREXIT("vkEnumeratePhysicalDevices reported zero device??\n", "vkEnumeratePhysicalDevices failed");
    VkPhysicalDevice * physical_devices = (VkPhysicalDevice *)calloc(gpu_count, sizeof(VkPhysicalDevice));
    err = vkEnumeratePhysicalDevices(demo.inst, &gpu_count, physical_devices);
    _ASSERT_EXPR(0 == err, _T("Filling physical_devices array failed"));

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

    //
    // Enumerate device extensions
    //
    UINT device_extension_count = 0;
    VkBool32 swapchain_ext_found = 0;
    demo.enabled_ext_count = 0;
    memset(demo.ext_names, 0, sizeof(demo.ext_names));

    err = vkEnumerateDeviceExtensionProperties(demo.gpu, NULL, &device_extension_count, NULL);
    _ASSERT_EXPR(0 == err, _T("Failed to enumerate device extensions"));
    if (device_extension_count > 0) {
        VkExtensionProperties * device_extensions =
            calloc(device_extension_count, sizeof(VkExtensionProperties));
        err = vkEnumerateDeviceExtensionProperties(demo.gpu, NULL, &device_extension_count, device_extensions);
        _ASSERT_EXPR(0 == err, _T("Failed to enumerate device extensions"));

        for (UINT i = 0; i < device_extension_count; ++i) {
            if (0 == strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, device_extensions[i].extensionName)) {
                swapchain_ext_found = 1;
                demo.ext_names[demo.enabled_ext_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            if (0 == strcmp("VK_KHR_portability_subset", device_extensions[i].extensionName)) {
                demo.ext_names[demo.enabled_ext_count++] = "VK_KHR_portability_subset";
            }
            _ASSERT_EXPR(demo.enabled_ext_count < 64, _T("Invalid enabled ext count"));
        }

        // TODO(omid): handle other extensions such as
        //      VK_KHR_incremental_present_enabled,
        //      VK_GOOGLE_display_timing_enabled

        free(device_extensions);
    }

    if (0 == swapchain_ext_found)
        ERREXIT(
            "vkEnumerateDeviceExtensionProperties failed to find " VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            "Do u hav a compatible vulkan installable client driver (ICD)?"
        );

    //
    // Setup VK_EXT_debug_utils function pointers
    //
    if (demo.validate) {
        demo.CreateDebugUtilsMessengerEXT =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo.inst, "vkCreateDebugUtilsMessengerEXT");
        demo.DestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(demo.inst, "vkDestroyDebugUtilsMessengerEXT");
        demo.SubmitDebugUtilsMessageEXT =
            (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(demo.inst, "vkSubmitDebugUtilsMessageEXT");
        demo.CmdBeginDebugUtilsLabelEXT =
            (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo.inst, "vkCmdBeginDebugUtilsLabelEXT");
        demo.CmdEndDebugUtilsLabelEXT =
            (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo.inst, "vkCmdEndDebugUtilsLabelEXT");
        demo.CmdInsertDebugUtilsLabelEXT =
            (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(demo.inst, "vkCmdInsertDebugUtilsLabelEXT");
        demo.SetDebugUtilsObjectNameEXT =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(demo.inst, "vkSetDebugUtilsObjectNameEXT");

        if (
            NULL == demo.CreateDebugUtilsMessengerEXT || NULL == demo.DestroyDebugUtilsMessengerEXT ||
            NULL == demo.SubmitDebugUtilsMessageEXT || NULL == demo.CmdBeginDebugUtilsLabelEXT ||
            NULL == demo.CmdEndDebugUtilsLabelEXT || NULL == demo.CmdInsertDebugUtilsLabelEXT ||
            NULL == demo.SetDebugUtilsObjectNameEXT
            ) {
            ERREXIT("GetProcAddr: Failed to init VK_EXT_debug_utils\n", "GetProcAddr: Failure");
        }

        err = demo.CreateDebugUtilsMessengerEXT(demo.inst, &dbg_msg_create_info, NULL, &demo.dbg_messenger);
        switch (err) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            ERREXIT("CreateDebugUtilsMessengerEXT: out of host memory\n", "CreateDebugUtilsMessengerEXT Failure");
            break;
        default:
            ERREXIT("CreateDebugUtilsMessengerEXT: unknown failure\n", "CreateDebugUtilsMessengerEXT Failure");
            break;
        }
    }
    vkGetPhysicalDeviceProperties(demo.gpu, &demo.gpu_props);

    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, NULL);
    _ASSERT_EXPR(demo.queue_family_count > 0, "Not enough queue family count");
    demo.queue_props = (VkQueueFamilyProperties *)calloc(demo.queue_family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo.gpu, &demo.queue_family_count, demo.queue_props);

    //
    // Query feature support for device (if needed)
    //
    VkPhysicalDeviceFeatures physDevFeatures;
    vkGetPhysicalDeviceFeatures(demo.gpu, &physDevFeatures);

    GET_INSTANCE_PROC_ADDR(demo.inst, GetPhysicalDeviceSurfaceSupportKHR);
    GET_INSTANCE_PROC_ADDR(demo.inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_INSTANCE_PROC_ADDR(demo.inst, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_INSTANCE_PROC_ADDR(demo.inst, GetPhysicalDeviceSurfacePresentModesKHR);
    GET_INSTANCE_PROC_ADDR(demo.inst, GetSwapchainImagesKHR);

#pragma region Init Swapchain
    //
    // Init swapchain
    //
    VkWin32SurfaceCreateInfoKHR surface_info = {0};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.pNext = NULL;
    surface_info.hinstance = demo.connection;
    surface_info.hwnd = demo.window;
    err = vkCreateWin32SurfaceKHR(demo.inst, &surface_info, NULL, &demo.surface);
    _ASSERT_EXPR(0 == err, _T("vkCreateWin32SurfaceKHR failed"));
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

    demo.graphics_queue_family_index = graphics_qfamid;
    demo.present_queue_family_index = present_qfamid;
    demo.separate_present_queue = (demo.graphics_queue_family_index != demo.present_queue_family_index);
    free(supports_presents);

    //
    // Create [Logical] device
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
    device_info.enabledExtensionCount = demo.enabled_ext_count;
    device_info.ppEnabledExtensionNames = (char const * const *)demo.ext_names;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = NULL;
    device_info.pEnabledFeatures = NULL;

    err = vkCreateDevice(demo.gpu, &device_info, NULL, &demo.device);
    _ASSERT_EXPR(0 == err, _T("vkCreateDevice failed"));

    GET_DEVICE_PROC_ADDR(demo.device, CreateSwapchainKHR);
    GET_DEVICE_PROC_ADDR(demo.device, DestroySwapchainKHR);
    GET_DEVICE_PROC_ADDR(demo.device, GetSwapchainImagesKHR);
    GET_DEVICE_PROC_ADDR(demo.device, AcquireNextImageKHR);
    GET_DEVICE_PROC_ADDR(demo.device, QueuePresentKHR);

    vkGetDeviceQueue(demo.device, demo.graphics_queue_family_index, 0, &demo.graphics_queue);

    if (!demo.separate_present_queue)
        demo.present_queue = demo.graphics_queue;
    else
        vkGetDeviceQueue(demo.device, demo.present_queue_family_index, 0, &demo.present_queue);

    // -- get a list of VkFormat's that are supported:
    UINT format_count;
    err = demo.fpGetPhysicalDeviceSurfaceFormatsKHR(demo.gpu, demo.surface, &format_count, NULL);
    _ASSERT_EXPR(0 == err, _T("fpGetPhysicalDeviceSurfaceFormatsKHR failed"));
    VkSurfaceFormatKHR * surface_formats = (VkSurfaceFormatKHR *)calloc(format_count, sizeof(VkSurfaceFormatKHR));
    err = demo.fpGetPhysicalDeviceSurfaceFormatsKHR(demo.gpu, demo.surface, &format_count, surface_formats);
    _ASSERT_EXPR(0 == err, _T("fpGetPhysicalDeviceSurfaceFormatsKHR failed"));
    VkSurfaceFormatKHR selected_format = select_surface_format(surface_formats, format_count);
    demo.format = selected_format.format;
    demo.color_space = selected_format.colorSpace;
    free(surface_formats);

    demo.cur_frame = 0;

    // -- create semaphores to synchronize acquiring presentable buffers before
    //    rendering and waiting for drawing to be complete before presenting
    VkSemaphoreCreateInfo semaphore_ci = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
    };

    // -- create fences that we can use to throttle if we get too far
    //    ahead of the image presents
    VkFenceCreateInfo fence_ci = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = NULL, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (uint32_t i = 0; i < FRAME_LAG; i++) {
        err = vkCreateFence(demo.device, &fence_ci, NULL, &demo.fences[i]);
        _ASSERT_EXPR(0 == err, "vkCreateFence failed");

        err = vkCreateSemaphore(demo.device, &semaphore_ci, NULL, &demo.image_acquired_semaphores[i]);
        _ASSERT_EXPR(0 == err, "vkCreateSemaphore failed");

        err = vkCreateSemaphore(demo.device, &semaphore_ci, NULL, &demo.draw_complete_semaphores[i]);
        _ASSERT_EXPR(0 == err, "vkCreateSemaphore failed");

        if (demo.separate_present_queue) {
            err = vkCreateSemaphore(demo.device, &semaphore_ci, NULL, &demo.image_ownership_semaphores[i]);
            _ASSERT_EXPR(0 == err, "vkCreateSemaphore failed");
        }
    }
    demo.frame_index = 0;

    // -- get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(demo.gpu, &demo.memory_properties);
#pragma endregion
    //
    //  Initi command buffer
    //
    if (demo.cmd_pool == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo pool_ci = {0};
        pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_ci.pNext = NULL;
        pool_ci.queueFamilyIndex = demo.graphics_queue_family_index;
        pool_ci.flags = 0;
        err = vkCreateCommandPool(demo.device, &pool_ci, NULL, &demo.cmd_pool);
        _ASSERT_EXPR(0 == err, _T("vkCreateCommandPool failed"));
    }
    VkCommandBufferAllocateInfo cmd_buf_alloc_ci = {0};
    cmd_buf_alloc_ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buf_alloc_ci.pNext = NULL;
    cmd_buf_alloc_ci.commandPool = demo.cmd_pool;
    cmd_buf_alloc_ci.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_alloc_ci.commandBufferCount = 1;
    err = vkAllocateCommandBuffers(demo.device, &cmd_buf_alloc_ci, &demo.cmd_buf);
    _ASSERT_EXPR(0 == err, _T("vkAllocateCommandBuffers failed"));

    VkCommandBufferBeginInfo cmd_buf_begin_ci = {0};
    cmd_buf_begin_ci.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_begin_ci.pNext = NULL;
    cmd_buf_begin_ci.flags = 0;
    cmd_buf_begin_ci.pInheritanceInfo = NULL;
    err = vkBeginCommandBuffer(demo.cmd_buf, &cmd_buf_begin_ci);
    _ASSERT_EXPR(0 == err, _T("vkBeginCommandBuffer failed"));

#pragma endregion

    build_backbuffers(&demo);

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

