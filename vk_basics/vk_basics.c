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
#include "linmath.h"

// Allow a maximum of two outstanding presentation operations.
#define FRAME_LAG 2

#define _TEXTURE_COUNT 1

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

// TODO(omid): Remove all this global hard-coded data stuff 
#pragma region Global Data
//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------
// clang-format off
static const float g_vertex_buffer_data [] = {
    -1.0f,-1.0f,-1.0f,  // -X side
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Z side
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f,-1.0f,  // -Y side
     1.0f,-1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,  // +Y side
    -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,

     1.0f, 1.0f,-1.0f,  // +X side
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f, 1.0f,  // +Z side
    -1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
};

static const float g_uv_buffer_data [] = {
    0.0f, 1.0f,  // -X side
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // -Z side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // -Y side
    1.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // +Y side
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,  // +X side
    0.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,  // +Z side
    0.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

static char *tex_files [] = {"lunarg.ppm"};

#pragma endregion

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

// NOTE(omid): structure to track all objects related to a texture.
typedef struct {
    VkSampler sampler;

    VkImage image;
    VkBuffer buffer;
    VkImageLayout image_layout;

    VkMemoryAllocateInfo mem_alloc;
    VkDeviceMemory mem;
    VkImageView view;
    int32_t tex_width, tex_height;
} TextureObject;

typedef struct {
    // Must start with MVP
    float mvp[4][4];
    float position[12 * 3][4];
    float attr[12 * 3][4];
} VkCubeTexVSUniform;

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
    VkCommandPool               cmd_pool_present;
    VkCommandBuffer             cmd_buf;
    VkPipelineLayout            pipeline_layout;
    VkDescriptorSetLayout       descriptor_layout;
    VkPipelineCache             pipeline_cache;
    VkRenderPass                renderpass;
    VkPipeline                  pipeline;
    VkDescriptorPool            descriptor_pool;

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

    struct {
        VkFormat                format;
        VkImage                 image;
        VkMemoryAllocateInfo    mem_alloc;
        VkDeviceMemory          mem;
        VkImageView view;
    } depth;

    bool use_staging_buffer;
    TextureObject textures[_TEXTURE_COUNT];
    TextureObject staging_texture;

    mat4x4 projection_matrix;
    mat4x4 view_matrix;
    mat4x4 model_matrix;

    VkShaderModule vertex_shader_module;
    VkShaderModule pixel_shader_module;

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

    VkImage * swapchain_images = (VkImage *)calloc(demo->swapchain_image_count, sizeof(VkImage));
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
static bool
memory_type_from_properties (
    Demo * demo,
    uint32_t type_bits,
    VkFlags requirements_mask,
    uint32_t * type_index
) {
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((type_bits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((demo->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *type_index = i;
                return true;
            }
        }
        type_bits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}
static void
build_depthbuffer (Demo * demo) {
    VkFormat depth_format = VK_FORMAT_D16_UNORM;
    VkImageCreateInfo image_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent = {demo->width, demo->height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .flags = 0,
    };
    VkImageViewCreateInfo view_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .image = VK_NULL_HANDLE,
        .format = depth_format,
        .subresourceRange =
            {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
        .flags = 0,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };
    VkMemoryRequirements mem_reqs;
    VkResult err;
    bool pass;

    demo->depth.format = depth_format;

    // -- create image
    err = vkCreateImage(demo->device, &image_ci, NULL, &demo->depth.image);
    _ASSERT_EXPR(0 == err, "vkCreateImage failed");


    // -- allocate memory
    vkGetImageMemoryRequirements(demo->device, demo->depth.image, &mem_reqs);
    demo->depth.mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    demo->depth.mem_alloc.pNext = NULL;
    demo->depth.mem_alloc.allocationSize = mem_reqs.size;
    demo->depth.mem_alloc.memoryTypeIndex = 0;
    pass = memory_type_from_properties(
        demo,
        mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &demo->depth.mem_alloc.memoryTypeIndex
    );
    _ASSERT_EXPR(pass, "could not find memory type index");
    err = vkAllocateMemory(demo->device, &demo->depth.mem_alloc, NULL, &demo->depth.mem);
    _ASSERT_EXPR(0 == err, "vkAllocateMemory failed");

    // -- bind memory
    err = vkBindImageMemory(demo->device, demo->depth.image, demo->depth.mem, 0);
    _ASSERT_EXPR(0 == err, "vkBindeImageMemory Failed");

    // -- create image view
    view_ci.image = demo->depth.image;
    err = vkCreateImageView(demo->device, &view_ci, NULL, &demo->depth.view);
    _ASSERT_EXPR(0 == err, "vkCreateImageView failed");
}
/* Convert ppm image data from header file into RGBA texture image */
#include "lunarg.ppm.h"
bool load_texture(
    char const * filename,
    uint8_t * rgba_data,
    VkSubresourceLayout * layout,
    int32_t * width, int32_t * height
) {
    (void)filename;
    char *cPtr;
    cPtr = (char *)lunarg_ppm;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "P6\n", 3)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    sscanf(cPtr, "%u %u", width, height);
    if (rgba_data == NULL) {
        return true;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "255\n", 4)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    for (int y = 0; y < *height; y++) {
        uint8_t *rowPtr = rgba_data;
        for (int x = 0; x < *width; x++) {
            memcpy(rowPtr, cPtr, 3);
            rowPtr[3] = 255; /* Alpha of 1 */
            rowPtr += 4;
            cPtr += 3;
        }
        rgba_data += layout->rowPitch;
    }
    return true;
}
static void
build_texture_buffer (Demo * demo, char const * filename, TextureObject * tex_obj) {
    int32_t tex_width = 0;
    int32_t tex_height = 0;
    VkResult err;
    bool pass;

    if (load_texture(filename, NULL, NULL, &tex_width, &tex_height)) {
        ERREXIT("Failed to load textures", "Load Texture Failure");
    }

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .size = tex_width * tex_height * 4,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL
    };

    err = vkCreateBuffer(demo->device, &buffer_create_info, NULL, &tex_obj->buffer);
    _ASSERT_EXPR(0 == err, "vkCreateBuffer failed");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(demo->device, tex_obj->buffer, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    VkFlags requirements = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, requirements, &tex_obj->mem_alloc.memoryTypeIndex);
    _ASSERT_EXPR(0 == err, "memory_type_from_properties failed");

    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    _ASSERT_EXPR(0 == err, "vkAllocateMemory failed");

    /* bind memory */
    err = vkBindBufferMemory(demo->device, tex_obj->buffer, tex_obj->mem, 0);
    _ASSERT_EXPR(0 == err, "vkBindBufferMemory failed");

    VkSubresourceLayout layout;
    memset(&layout, 0, sizeof(layout));
    layout.rowPitch = tex_width * 4;

    void *data;
    err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
    _ASSERT_EXPR(0 == err, "vkMapMemory failed");

    if (!load_texture(filename, data, &layout, &tex_width, &tex_height)) {
        ERREXIT("Error loading texture", "Check file name");
    }

    vkUnmapMemory(demo->device, tex_obj->mem);
}
static void
build_texture_image (
    Demo * demo, char const * filename, TextureObject * tex_obj,
    VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props
) {
    VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    int32_t tex_width = 0;
    int32_t tex_height = 0;
    VkResult err;
    bool pass;

    if (!load_texture(filename, NULL, NULL, &tex_width, &tex_height)) {
        ERREXIT("Failed to load textures", "Load Texture Failure");
    }

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = tex_format,
        .extent = {tex_width, tex_height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .flags = 0,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
    };

    VkMemoryRequirements mem_reqs;

    err = vkCreateImage(demo->device, &image_create_info, NULL, &tex_obj->image);
    _ASSERT_EXPR(0 == err, "vkCreateImage failed");

    vkGetImageMemoryRequirements(demo->device, tex_obj->image, &mem_reqs);

    tex_obj->mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tex_obj->mem_alloc.pNext = NULL;
    tex_obj->mem_alloc.allocationSize = mem_reqs.size;
    tex_obj->mem_alloc.memoryTypeIndex = 0;

    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits, required_props, &tex_obj->mem_alloc.memoryTypeIndex);
    _ASSERT_EXPR(pass, "could not get memory type from properties");

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &tex_obj->mem_alloc, NULL, &(tex_obj->mem));
    _ASSERT_EXPR(0 == err, "vkAllocateMemory failed");

    /* bind memory */
    err = vkBindImageMemory(demo->device, tex_obj->image, tex_obj->mem, 0);
    _ASSERT_EXPR(0 == err, "vkBindImageMemory failed");

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VkImageSubresource subres = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        void *data;

        vkGetImageSubresourceLayout(demo->device, tex_obj->image, &subres, &layout);

        err = vkMapMemory(demo->device, tex_obj->mem, 0, tex_obj->mem_alloc.allocationSize, 0, &data);
        _ASSERT_EXPR(0 == err, "vkMapMemory Failed");

        if (!load_texture(filename, data, &layout, &tex_width, &tex_height)) {
            ERREXIT("Error loading texture", "check filename");
        }

        vkUnmapMemory(demo->device, tex_obj->mem);
    }

    tex_obj->image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}
static void
set_image_layout (
    Demo * demo, VkImage image,
    VkImageAspectFlags aspect_mask,
    VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    VkAccessFlagBits src_access_mask,
    VkPipelineStageFlags src_stages,
    VkPipelineStageFlags dest_stages
) {
    _ASSERT_EXPR(demo->cmd_buf, "invalid cmd buffer");

    VkImageMemoryBarrier image_memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = src_access_mask,
        .dstAccessMask = 0,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .oldLayout = old_image_layout,
        .newLayout = new_image_layout,
        .image = image,
        .subresourceRange = {aspect_mask, 0, 1, 0, 1}};

    switch (new_image_layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        /* Make sure anything that was copying from this image has completed */
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        break;

    default:
        image_memory_barrier.dstAccessMask = 0;
        break;
    }

    VkImageMemoryBarrier * pmemory_barrier = &image_memory_barrier;

    vkCmdPipelineBarrier(demo->cmd_buf, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
}
static void
build_textures (Demo * demo) {
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props;

    vkGetPhysicalDeviceFormatProperties(demo->gpu, tex_format, &props);

    for (unsigned i = 0; i < _TEXTURE_COUNT; i++) {
        VkResult err;

        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !demo->use_staging_buffer) {
            /* Device can texture using linear textures */
            build_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
     // Nothing in the pipeline_ci needs to be complete to start, and don't allow fragment
     // shader to run until layout transition completes
            set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                             demo->textures[i].image_layout, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            demo->staging_texture.image = 0;
        } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
            /* Must use staging buffer to copy linear texture to optimized */

            memset(&demo->staging_texture, 0, sizeof(demo->staging_texture));
            build_texture_buffer(demo, tex_files[i], &demo->staging_texture);

            build_texture_image(demo, tex_files[i], &demo->textures[i], VK_IMAGE_TILING_OPTIMAL,
                                (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT);

            VkBufferImageCopy copy_region = {
                .bufferOffset = 0,
                .bufferRowLength = demo->staging_texture.tex_width,
                .bufferImageHeight = demo->staging_texture.tex_height,
                .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .imageOffset = {0, 0, 0},
                .imageExtent = {demo->staging_texture.tex_width, demo->staging_texture.tex_height, 1},
            };

            vkCmdCopyBufferToImage(demo->cmd_buf, demo->staging_texture.buffer, demo->textures[i].image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            set_image_layout(demo, demo->textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             demo->textures[i].image_layout, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        } else {
            ERREXIT("No support for R8G8B8A8_UNORM as texture image format", "Can't support VK_FORMAT_R8G8B8A8_UNORM !?");
        }

        VkSamplerCreateInfo sampler = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = NULL,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkImageViewCreateInfo view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = tex_format,
            .components =
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .flags = 0,
        };

        /* create sampler */
        err = vkCreateSampler(demo->device, &sampler, NULL, &demo->textures[i].sampler);
        _ASSERT_EXPR(0 == err, "vkCreateSampler failed");

        /* create image view */
        view.image = demo->textures[i].image;
        err = vkCreateImageView(demo->device, &view, NULL, &demo->textures[i].view);
        _ASSERT_EXPR(0 == err, "vkCreateImageView");
    }
}
static void
build_cube (Demo * demo) {
    VkBufferCreateInfo buf_info;
    VkMemoryRequirements mem_reqs;
    VkMemoryAllocateInfo mem_alloc;
    mat4x4 MVP, VP;
    VkResult err;
    bool pass;
    VkCubeTexVSUniform data;

    mat4x4_mul(VP, demo->projection_matrix, demo->view_matrix);
    mat4x4_mul(MVP, VP, demo->model_matrix);
    memcpy(data.mvp, MVP, sizeof(MVP));
    //    dumpMatrix("MVP", MVP);

    for (unsigned int i = 0; i < 12 * 3; i++) {
        data.position[i][0] = g_vertex_buffer_data[i * 3];
        data.position[i][1] = g_vertex_buffer_data[i * 3 + 1];
        data.position[i][2] = g_vertex_buffer_data[i * 3 + 2];
        data.position[i][3] = 1.0f;
        data.attr[i][0] = g_uv_buffer_data[2 * i];
        data.attr[i][1] = g_uv_buffer_data[2 * i + 1];
        data.attr[i][2] = 0;
        data.attr[i][3] = 0;
    }

    memset(&buf_info, 0, sizeof(buf_info));
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = sizeof(data);

    for (unsigned int i = 0; i < demo->swapchain_image_count; i++) {
        err = vkCreateBuffer(demo->device, &buf_info, NULL, &demo->swapchain_image_resources[i].uniform_buffer);
        _ASSERT_EXPR(0 == err, "vkCreateBuffer failed");

        vkGetBufferMemoryRequirements(demo->device, demo->swapchain_image_resources[i].uniform_buffer, &mem_reqs);

        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = mem_reqs.size;
        mem_alloc.memoryTypeIndex = 0;

        pass = memory_type_from_properties(
            demo, mem_reqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &mem_alloc.memoryTypeIndex
        );
        _ASSERT_EXPR(pass, "could not get memory type from properties");

        err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->swapchain_image_resources[i].uniform_memory);
        _ASSERT_EXPR(0 == err, "vkAllocateMemory failed");

        err = vkMapMemory(demo->device, demo->swapchain_image_resources[i].uniform_memory, 0, VK_WHOLE_SIZE, 0,
                          &demo->swapchain_image_resources[i].uniform_memory_ptr);
        _ASSERT_EXPR(0 == err, "vkMapMemory failed");

        memcpy(demo->swapchain_image_resources[i].uniform_memory_ptr, &data, sizeof data);

        err = vkBindBufferMemory(
            demo->device, demo->swapchain_image_resources[i].uniform_buffer,
            demo->swapchain_image_resources[i].uniform_memory, 0
        );
        _ASSERT_EXPR(0 == err, "vkBindBufferMemory failed");
    }
}
static void
build_descriptor_layout(Demo * demo) {
    VkDescriptorSetLayoutBinding layout_bindings[2] = {
        [0] =
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = NULL,
            },
        [1] =
            {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = _TEXTURE_COUNT,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = NULL,
            },
    };
    VkDescriptorSetLayoutCreateInfo descriptor_layout_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .bindingCount = 2,
        .pBindings = layout_bindings,
    };
    VkResult err;

    err = vkCreateDescriptorSetLayout(demo->device, &descriptor_layout_ci, NULL, &demo->descriptor_layout);
    _ASSERT_EXPR(0 == err, "vkCreateDescriptorSetLayout failed");

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .setLayoutCount = 1,
        .pSetLayouts = &demo->descriptor_layout,
    };

    err = vkCreatePipelineLayout(demo->device, &pPipelineLayoutCreateInfo, NULL, &demo->pipeline_layout);
    _ASSERT_EXPR(0 == err, "vkCreatePipelineLayout failed");
}
static void
build_renderpass (Demo * demo) {
        // The initial layout for the color and depth attachments will be LAYOUT_UNDEFINED
        // because at the start of the renderpass, we don't care about their contents.
        // At the start of the subpass, the color attachment's layout will be transitioned
        // to LAYOUT_COLOR_ATTACHMENT_OPTIMAL and the depth stencil attachment's layout
        // will be transitioned to LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of
        // the renderpass, the color attachment's layout will be transitioned to
        // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part of
        // the renderpass, no barriers are necessary.
    VkAttachmentDescription attachments[2] = {
        [0] =
            {
                .format = demo->format,
                .flags = 0,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        [1] =
            {
                .format = demo->depth.format,
                .flags = 0,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
    };
    VkAttachmentReference color_reference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference depth_reference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depth_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };

    VkSubpassDependency attachment_deps[2] = {
        [0] =
            {
                // Depth buffer is shared between swapchain images
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = 0,
            },
        [1] =
            {
                // Image Layout Transition
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                .dependencyFlags = 0,
            },
    };

    VkRenderPassCreateInfo renderpass_ci = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 2,
        .pDependencies = attachment_deps,
    };
    VkResult err = vkCreateRenderPass(demo->device, &renderpass_ci, NULL, &demo->renderpass);
    _ASSERT_EXPR(0 == err, "vkCreateRenderPass failed");
}
static VkShaderModule
build_shader_module (Demo * demo, const uint32_t * code, size_t size) {
    VkShaderModule module;
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkResult err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;

    err = vkCreateShaderModule(demo->device, &moduleCreateInfo, NULL, &module);
    _ASSERT_EXPR(0 == err, "vkCreateShaderModule failed");

    return module;
}

static void
build_vs_module (Demo * demo) {
    const uint32_t vs_code [] = {
#include "cube.vert.inc"
    };
    demo->vertex_shader_module = build_shader_module(demo, vs_code, sizeof(vs_code));
}

static void
build_ps_module (Demo * demo) {
    const uint32_t fs_code [] = {
#include "cube.frag.inc"
    };
    demo->pixel_shader_module = build_shader_module(demo, fs_code, sizeof(fs_code));
}
static void
build_pipeline (Demo * demo) {
#define NUM_DYNAMIC_STATES 2 /*Viewport + Scissor*/

    VkGraphicsPipelineCreateInfo pipeline_ci;
    VkPipelineCacheCreateInfo pipeline_cache_ci;
    VkPipelineVertexInputStateCreateInfo vertex_input_ci;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_ci;
    VkPipelineRasterizationStateCreateInfo rasterizer_ci;
    VkPipelineColorBlendStateCreateInfo blend_state_ci;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_ci;
    VkPipelineViewportStateCreateInfo viewport_ci;
    VkPipelineMultisampleStateCreateInfo mulitsample_state_ci;
    VkDynamicState dynamic_states_enables[NUM_DYNAMIC_STATES];
    VkPipelineDynamicStateCreateInfo dynamic_state_ci;
    VkResult err;

    memset(dynamic_states_enables, 0, sizeof(dynamic_states_enables));
    memset(&dynamic_state_ci, 0, sizeof(dynamic_state_ci));
    dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_ci.pDynamicStates = dynamic_states_enables;

    memset(&pipeline_ci, 0, sizeof(pipeline_ci));
    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_ci.layout = demo->pipeline_layout;

    memset(&vertex_input_ci, 0, sizeof(vertex_input_ci));
    vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    memset(&input_assembly_ci, 0, sizeof(input_assembly_ci));
    input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rasterizer_ci, 0, sizeof(rasterizer_ci));
    rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_ci.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_ci.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_ci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_ci.depthClampEnable = VK_FALSE;
    rasterizer_ci.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_ci.depthBiasEnable = VK_FALSE;
    rasterizer_ci.lineWidth = 1.0f;

    memset(&blend_state_ci, 0, sizeof(blend_state_ci));
    blend_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att_state[1];
    memset(att_state, 0, sizeof(att_state));
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;
    blend_state_ci.attachmentCount = 1;
    blend_state_ci.pAttachments = att_state;

    memset(&viewport_ci, 0, sizeof(viewport_ci));
    viewport_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_ci.viewportCount = 1;
    dynamic_states_enables[dynamic_state_ci.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    viewport_ci.scissorCount = 1;
    dynamic_states_enables[dynamic_state_ci.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    memset(&depth_stencil_ci, 0, sizeof(depth_stencil_ci));
    depth_stencil_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_ci.depthTestEnable = VK_TRUE;
    depth_stencil_ci.depthWriteEnable = VK_TRUE;
    depth_stencil_ci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_ci.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_ci.back.failOp = VK_STENCIL_OP_KEEP;
    depth_stencil_ci.back.passOp = VK_STENCIL_OP_KEEP;
    depth_stencil_ci.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depth_stencil_ci.stencilTestEnable = VK_FALSE;
    depth_stencil_ci.front = depth_stencil_ci.back;

    memset(&mulitsample_state_ci, 0, sizeof(mulitsample_state_ci));
    mulitsample_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mulitsample_state_ci.pSampleMask = NULL;
    mulitsample_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    build_vs_module(demo);
    build_ps_module(demo);

    // Two stages: vertex shader and pixel shader
    VkPipelineShaderStageCreateInfo shader_stages[2];
    memset(&shader_stages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = demo->vertex_shader_module;
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = demo->pixel_shader_module;
    shader_stages[1].pName = "main";

    memset(&pipeline_cache_ci, 0, sizeof(pipeline_cache_ci));
    pipeline_cache_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    err = vkCreatePipelineCache(demo->device, &pipeline_cache_ci, NULL, &demo->pipeline_cache);
    _ASSERT_EXPR(0 == err, "vkCreatePipelineCache failed");

    pipeline_ci.pVertexInputState = &vertex_input_ci;
    pipeline_ci.pInputAssemblyState = &input_assembly_ci;
    pipeline_ci.pRasterizationState = &rasterizer_ci;
    pipeline_ci.pColorBlendState = &blend_state_ci;
    pipeline_ci.pMultisampleState = &mulitsample_state_ci;
    pipeline_ci.pViewportState = &viewport_ci;
    pipeline_ci.pDepthStencilState = &depth_stencil_ci;
    pipeline_ci.stageCount = _countof(shader_stages);
    pipeline_ci.pStages = shader_stages;
    pipeline_ci.renderPass = demo->renderpass;
    pipeline_ci.pDynamicState = &dynamic_state_ci;

    pipeline_ci.renderPass = demo->renderpass;

    err = vkCreateGraphicsPipelines(demo->device, demo->pipeline_cache, 1, &pipeline_ci, NULL, &demo->pipeline);
    _ASSERT_EXPR(0 == err, "vkCreateGraphicsPipelines failed");

    vkDestroyShaderModule(demo->device, demo->pixel_shader_module, NULL);
    vkDestroyShaderModule(demo->device, demo->vertex_shader_module, NULL);
}
static void
build_image_ownership_cmd(Demo * demo, int i) {
    VkResult err;

    VkCommandBufferBeginInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = NULL,
    };
    err = vkBeginCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd, &cmd_buf_info);
    _ASSERT_EXPR(0 == err, "vkBeginCommandBUffer failed");

    VkImageMemoryBarrier image_ownership_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = demo->graphics_queue_family_index,
        .dstQueueFamilyIndex = demo->present_queue_family_index,
        .image = demo->swapchain_image_resources[i].image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    vkCmdPipelineBarrier(
        demo->swapchain_image_resources[i].graphics_to_present_cmd,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, NULL, 0, NULL, 1, &image_ownership_barrier
    );
    err = vkEndCommandBuffer(demo->swapchain_image_resources[i].graphics_to_present_cmd);
    _ASSERT_EXPR(0 == err, "vkEndCommandBuffer failed");
}
static void
build_descriptor_pool (Demo * demo) {
    VkDescriptorPoolSize type_counts[2] = {
        [0] =
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = demo->swapchain_image_count,
            },
        [1] =
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = demo->swapchain_image_count * _TEXTURE_COUNT,
            },
    };
    VkDescriptorPoolCreateInfo descriptor_pool_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .maxSets = demo->swapchain_image_count,
        .poolSizeCount = 2,
        .pPoolSizes = type_counts,
    };
    VkResult err;

    err = vkCreateDescriptorPool(demo->device, &descriptor_pool_ci, NULL, &demo->descriptor_pool);
    _ASSERT_EXPR(0 == err, "failed to create descriptor pool");
}
static void
build_descriptor_set (Demo * demo) {
    VkDescriptorImageInfo tex_descs[_TEXTURE_COUNT];
    VkWriteDescriptorSet writes[2];
    VkResult err;

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = demo->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &demo->descriptor_layout};

    VkDescriptorBufferInfo buffer_info;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(VkCubeTexVSUniform);

    memset(&tex_descs, 0, sizeof(tex_descs));
    for (unsigned int i = 0; i < _TEXTURE_COUNT; i++) {
        tex_descs[i].sampler = demo->textures[i].sampler;
        tex_descs[i].imageView = demo->textures[i].view;
        tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    memset(&writes, 0, sizeof(writes));

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &buffer_info;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = _TEXTURE_COUNT;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = tex_descs;

    for (unsigned int i = 0; i < demo->swapchain_image_count; i++) {
        err = vkAllocateDescriptorSets(demo->device, &alloc_info, &demo->swapchain_image_resources[i].descriptor_set);
        _ASSERT_EXPR(0 == err, "vkAllocateDescriptorsets failed");
        buffer_info.buffer = demo->swapchain_image_resources[i].uniform_buffer;
        writes[0].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        writes[1].dstSet = demo->swapchain_image_resources[i].descriptor_set;
        vkUpdateDescriptorSets(demo->device, 2, writes, 0, NULL);
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

    build_depthbuffer(&demo);

    build_textures(&demo);

    build_cube(&demo);

    build_descriptor_layout(&demo);

    build_renderpass(&demo);

    build_pipeline(&demo);

    for (uint32_t i = 0; i < demo.swapchain_image_count; i++) {
        err = vkAllocateCommandBuffers(demo.device, &cmd_buf_alloc_ci, &demo.swapchain_image_resources[i].cmd);
        _ASSERT_EXPR(0 == err, "vkAllocateCommandBuffers failed");
    }

    if (demo.separate_present_queue) {
        const VkCommandPoolCreateInfo present_cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = NULL,
            .queueFamilyIndex = demo.present_queue_family_index,
            .flags = 0,
        };
        err = vkCreateCommandPool(demo.device, &present_cmd_pool_info, NULL, &demo.cmd_pool_present);
        _ASSERT_EXPR(0 == err, "vkCreateCommandPool failed");
        const VkCommandBufferAllocateInfo present_cmd_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = demo.cmd_pool_present,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        for (uint32_t i = 0; i < demo.swapchain_image_count; i++) {
            err = vkAllocateCommandBuffers(demo.device, &present_cmd_info,
                                           &demo.swapchain_image_resources[i].graphics_to_present_cmd);
            _ASSERT_EXPR(0 == err, "vkAllocateCommandBuffers failed");
            build_image_ownership_cmd(&demo, i);
        }
    }

    build_descriptor_pool(&demo);
    build_descriptor_set(&demo);


    // build_framebuffers


    // draw cmd

    // flush init cmd

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

