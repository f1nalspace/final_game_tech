/*
-------------------------------------------------------------------------------
Name:
	FPL-Demo | Vulkan

Description:
	This demo showcases the initialization and usage of the Vulkan graphics API.
	Yes it has a few thousands lines of code, but thats normal for Vulkan.
	Also there are switches how FPL should handle the creation of Instance or Surface.

Requirements:
	- C99 Compiler
	- Final Platform Layer

Author:
	Torsten Spaete

Todo:
	- Let it at least draw something, because clearing to blue is boring

Changelog:
	## 2021-05-17
	- Initial version

License:
	Copyright (c) 2017-2021 Torsten Spaete
	MIT License (See LICENSE file)
-------------------------------------------------------------------------------
*/

//
// Config
//
#define VULKANDEMO_FPL_VIDEO_MODE_NONE 0 // Do not use FPL at all
#define VULKANDEMO_FPL_VIDEO_MODE_SURFACE_ONLY 1 // Let FPL only create the VkSurfaceKHR
#define VULKANDEMO_FPL_VIDEO_MODE_FULL 2 // Let FPL create the instance and the VkSurfaceKHR

#define VULKANDEMO_FPL_VIDEO_MODE VULKANDEMO_FPL_VIDEO_MODE_FULL

#define VULKANDEMO_USE_VALIDATION_LAYER 1
#define VULKANDEMO_VALIDATION_LAYER_SEVERITY VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT

//
// FPL Header
//
#define FPL_IMPLEMENTATION
#define FPL_LOGGING
#define FPL_NO_VIDEO_SOFTWARE
#define FPL_NO_VIDEO_OPENGL
#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

//
// Vulkan Header
//
#if defined(FPL_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(FPL_SUBPLATFORM_X11)
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#if defined(FPL_PLATFORM_WINDOWS)
#	define VULKAN_PLATFORM_SURFACE_NAME "VK_KHR_win32_surface"
#elif defined(FPL_SUBPLATFORM_X11)
#	define VULKAN_PLATFORM_SURFACE_NAME "VK_KHR_xlib_surface"
#else
#	define VULKAN_PLATFORM_SURFACE_NAME fpl_null
#endif

#include <malloc.h>

//
// Dynamic Arrays, String-Pools, etc.
//
#include "containers.h"

//
// Vulkan Utils
// - Enums to Strings
// - Queue Family Lookup
//
const char *VulkanValidationLayerNames[] = { "VK_LAYER_KHRONOS_validation" };
const char *VulkanKHRSurfaceName = "VK_KHR_surface";

typedef enum VulkanVendorID {
	VulkanVendorID_Unknown = 0,
	VulkanVendorID_AMD = 0x1002,
	VulkanVendorID_ImgTec = 0x1010,
	VulkanVendorID_NVIDIA = 0x10DE,
	VulkanVendorID_ARM = 0x13B5,
	VulkanVendorID_Qualcomm = 0x5143,
	VulkanVendorID_Intel = 0x8086
} VulkanVendorID;

static const char *GetVulkanVendorName(const VulkanVendorID pci) {
	switch(pci) {
		case VulkanVendorID_AMD:
			return "AMD";
		case VulkanVendorID_ImgTec:
			return "ImgTec";
		case VulkanVendorID_NVIDIA:
			return "NVIDIA";
		case VulkanVendorID_ARM:
			return "ARM";
		case VulkanVendorID_Qualcomm:
			return "Qualcomm";
		case VulkanVendorID_Intel:
			return "Intel";
		default:
			return "Unknown Vendor";
	}
}

static const char *GetVulkanMessageSeverityName(VkDebugUtilsMessageSeverityFlagBitsEXT value) {
	switch(value) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			return "WARNING";
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			return "ERROR";
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			return "INFO";
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			return "VERBOSE";
		default:
			return "UNKNOWN";
	}
}

static const char *GetVulkanPhysicalDeviceTypeName(const VkPhysicalDeviceType type) {
	switch(type) {
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";
		default:
			return "Other";
	}
}

static const char *GetVulkanFormatName(const VkFormat value) {
	switch(value) {
		case VK_FORMAT_UNDEFINED:
			return "VK_FORMAT_UNDEFINED";
		case VK_FORMAT_R4G4_UNORM_PACK8:
			return "VK_FORMAT_R4G4_UNORM_PACK8";
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
			return "VK_FORMAT_R5G6B5_UNORM_PACK16";
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
			return "VK_FORMAT_B5G6R5_UNORM_PACK16";
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
		case VK_FORMAT_R8_UNORM:
			return "VK_FORMAT_R8_UNORM";
		case VK_FORMAT_R8_SNORM:
			return "VK_FORMAT_R8_SNORM";
		case VK_FORMAT_R8_USCALED:
			return "VK_FORMAT_R8_USCALED";
		case VK_FORMAT_R8_SSCALED:
			return "VK_FORMAT_R8_SSCALED";
		case VK_FORMAT_R8_UINT:
			return "VK_FORMAT_R8_UINT";
		case VK_FORMAT_R8_SINT:
			return "VK_FORMAT_R8_SINT";
		case VK_FORMAT_R8_SRGB:
			return "VK_FORMAT_R8_SRGB";
		case VK_FORMAT_R8G8_UNORM:
			return "VK_FORMAT_R8G8_UNORM";
		case VK_FORMAT_R8G8_SNORM:
			return "VK_FORMAT_R8G8_SNORM";
		case VK_FORMAT_R8G8_USCALED:
			return "VK_FORMAT_R8G8_USCALED";
		case VK_FORMAT_R8G8_SSCALED:
			return "VK_FORMAT_R8G8_SSCALED";
		case VK_FORMAT_R8G8_UINT:
			return "VK_FORMAT_R8G8_UINT";
		case VK_FORMAT_R8G8_SINT:
			return "VK_FORMAT_R8G8_SINT";
		case VK_FORMAT_R8G8_SRGB:
			return "VK_FORMAT_R8G8_SRGB";
		case VK_FORMAT_R8G8B8_UNORM:
			return "VK_FORMAT_R8G8B8_UNORM";
		case VK_FORMAT_R8G8B8_SNORM:
			return "VK_FORMAT_R8G8B8_SNORM";
		case VK_FORMAT_R8G8B8_USCALED:
			return "VK_FORMAT_R8G8B8_USCALED";
		case VK_FORMAT_R8G8B8_SSCALED:
			return "VK_FORMAT_R8G8B8_SSCALED";
		case VK_FORMAT_R8G8B8_UINT:
			return "VK_FORMAT_R8G8B8_UINT";
		case VK_FORMAT_R8G8B8_SINT:
			return "VK_FORMAT_R8G8B8_SINT";
		case VK_FORMAT_R8G8B8_SRGB:
			return "VK_FORMAT_R8G8B8_SRGB";
		case VK_FORMAT_B8G8R8_UNORM:
			return "VK_FORMAT_B8G8R8_UNORM";
		case VK_FORMAT_B8G8R8_SNORM:
			return "VK_FORMAT_B8G8R8_SNORM";
		case VK_FORMAT_B8G8R8_USCALED:
			return "VK_FORMAT_B8G8R8_USCALED";
		case VK_FORMAT_B8G8R8_SSCALED:
			return "VK_FORMAT_B8G8R8_SSCALED";
		case VK_FORMAT_B8G8R8_UINT:
			return "VK_FORMAT_B8G8R8_UINT";
		case VK_FORMAT_B8G8R8_SINT:
			return "VK_FORMAT_B8G8R8_SINT";
		case VK_FORMAT_B8G8R8_SRGB:
			return "VK_FORMAT_B8G8R8_SRGB";
		case VK_FORMAT_R8G8B8A8_UNORM:
			return "VK_FORMAT_R8G8B8A8_UNORM";
		case VK_FORMAT_R8G8B8A8_SNORM:
			return "VK_FORMAT_R8G8B8A8_SNORM";
		case VK_FORMAT_R8G8B8A8_USCALED:
			return "VK_FORMAT_R8G8B8A8_USCALED";
		case VK_FORMAT_R8G8B8A8_SSCALED:
			return "VK_FORMAT_R8G8B8A8_SSCALED";
		case VK_FORMAT_R8G8B8A8_UINT:
			return "VK_FORMAT_R8G8B8A8_UINT";
		case VK_FORMAT_R8G8B8A8_SINT:
			return "VK_FORMAT_R8G8B8A8_SINT";
		case VK_FORMAT_R8G8B8A8_SRGB:
			return "VK_FORMAT_R8G8B8A8_SRGB";
		case VK_FORMAT_B8G8R8A8_UNORM:
			return "VK_FORMAT_B8G8R8A8_UNORM";
		case VK_FORMAT_B8G8R8A8_SNORM:
			return "VK_FORMAT_B8G8R8A8_SNORM";
		case VK_FORMAT_B8G8R8A8_USCALED:
			return "VK_FORMAT_B8G8R8A8_USCALED";
		case VK_FORMAT_B8G8R8A8_SSCALED:
			return "VK_FORMAT_B8G8R8A8_SSCALED";
		case VK_FORMAT_B8G8R8A8_UINT:
			return "VK_FORMAT_B8G8R8A8_UINT";
		case VK_FORMAT_B8G8R8A8_SINT:
			return "VK_FORMAT_B8G8R8A8_SINT";
		case VK_FORMAT_B8G8R8A8_SRGB:
			return "VK_FORMAT_B8G8R8A8_SRGB";
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
		case VK_FORMAT_R16_UNORM:
			return "VK_FORMAT_R16_UNORM";
		case VK_FORMAT_R16_SNORM:
			return "VK_FORMAT_R16_SNORM";
		case VK_FORMAT_R16_USCALED:
			return "VK_FORMAT_R16_USCALED";
		case VK_FORMAT_R16_SSCALED:
			return "VK_FORMAT_R16_SSCALED";
		case VK_FORMAT_R16_UINT:
			return "VK_FORMAT_R16_UINT";
		case VK_FORMAT_R16_SINT:
			return "VK_FORMAT_R16_SINT";
		case VK_FORMAT_R16_SFLOAT:
			return "VK_FORMAT_R16_SFLOAT";
		case VK_FORMAT_R16G16_UNORM:
			return "VK_FORMAT_R16G16_UNORM";
		case VK_FORMAT_R16G16_SNORM:
			return "VK_FORMAT_R16G16_SNORM";
		case VK_FORMAT_R16G16_USCALED:
			return "VK_FORMAT_R16G16_USCALED";
		case VK_FORMAT_R16G16_SSCALED:
			return "VK_FORMAT_R16G16_SSCALED";
		case VK_FORMAT_R16G16_UINT:
			return "VK_FORMAT_R16G16_UINT";
		case VK_FORMAT_R16G16_SINT:
			return "VK_FORMAT_R16G16_SINT";
		case VK_FORMAT_R16G16_SFLOAT:
			return "VK_FORMAT_R16G16_SFLOAT";
		case VK_FORMAT_R16G16B16_UNORM:
			return "VK_FORMAT_R16G16B16_UNORM";
		case VK_FORMAT_R16G16B16_SNORM:
			return "VK_FORMAT_R16G16B16_SNORM";
		case VK_FORMAT_R16G16B16_USCALED:
			return "VK_FORMAT_R16G16B16_USCALED";
		case VK_FORMAT_R16G16B16_SSCALED:
			return "VK_FORMAT_R16G16B16_SSCALED";
		case VK_FORMAT_R16G16B16_UINT:
			return "VK_FORMAT_R16G16B16_UINT";
		case VK_FORMAT_R16G16B16_SINT:
			return "VK_FORMAT_R16G16B16_SINT";
		case VK_FORMAT_R16G16B16_SFLOAT:
			return "VK_FORMAT_R16G16B16_SFLOAT";
		case VK_FORMAT_R16G16B16A16_UNORM:
			return "VK_FORMAT_R16G16B16A16_UNORM";
		case VK_FORMAT_R16G16B16A16_SNORM:
			return "VK_FORMAT_R16G16B16A16_SNORM";
		case VK_FORMAT_R16G16B16A16_USCALED:
			return "VK_FORMAT_R16G16B16A16_USCALED";
		case VK_FORMAT_R16G16B16A16_SSCALED:
			return "VK_FORMAT_R16G16B16A16_SSCALED";
		case VK_FORMAT_R16G16B16A16_UINT:
			return "VK_FORMAT_R16G16B16A16_UINT";
		case VK_FORMAT_R16G16B16A16_SINT:
			return "VK_FORMAT_R16G16B16A16_SINT";
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return "VK_FORMAT_R16G16B16A16_SFLOAT";
		case VK_FORMAT_R32_UINT:
			return "VK_FORMAT_R32_UINT";
		case VK_FORMAT_R32_SINT:
			return "VK_FORMAT_R32_SINT";
		case VK_FORMAT_R32_SFLOAT:
			return "VK_FORMAT_R32_SFLOAT";
		case VK_FORMAT_R32G32_UINT:
			return "VK_FORMAT_R32G32_UINT";
		case VK_FORMAT_R32G32_SINT:
			return "VK_FORMAT_R32G32_SINT";
		case VK_FORMAT_R32G32_SFLOAT:
			return "VK_FORMAT_R32G32_SFLOAT";
		case VK_FORMAT_R32G32B32_UINT:
			return "VK_FORMAT_R32G32B32_UINT";
		case VK_FORMAT_R32G32B32_SINT:
			return "VK_FORMAT_R32G32B32_SINT";
		case VK_FORMAT_R32G32B32_SFLOAT:
			return "VK_FORMAT_R32G32B32_SFLOAT";
		case VK_FORMAT_R32G32B32A32_UINT:
			return "VK_FORMAT_R32G32B32A32_UINT";
		case VK_FORMAT_R32G32B32A32_SINT:
			return "VK_FORMAT_R32G32B32A32_SINT";
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return "VK_FORMAT_R32G32B32A32_SFLOAT";
		case VK_FORMAT_R64_UINT:
			return "VK_FORMAT_R64_UINT";
		case VK_FORMAT_R64_SINT:
			return "VK_FORMAT_R64_SINT";
		case VK_FORMAT_R64_SFLOAT:
			return "VK_FORMAT_R64_SFLOAT";
		case VK_FORMAT_R64G64_UINT:
			return "VK_FORMAT_R64G64_UINT";
		case VK_FORMAT_R64G64_SINT:
			return "VK_FORMAT_R64G64_SINT";
		case VK_FORMAT_R64G64_SFLOAT:
			return "VK_FORMAT_R64G64_SFLOAT";
		case VK_FORMAT_R64G64B64_UINT:
			return "VK_FORMAT_R64G64B64_UINT";
		case VK_FORMAT_R64G64B64_SINT:
			return "VK_FORMAT_R64G64B64_SINT";
		case VK_FORMAT_R64G64B64_SFLOAT:
			return "VK_FORMAT_R64G64B64_SFLOAT";
		case VK_FORMAT_R64G64B64A64_UINT:
			return "VK_FORMAT_R64G64B64A64_UINT";
		case VK_FORMAT_R64G64B64A64_SINT:
			return "VK_FORMAT_R64G64B64A64_SINT";
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return "VK_FORMAT_R64G64B64A64_SFLOAT";
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
		case VK_FORMAT_D16_UNORM:
			return "VK_FORMAT_D16_UNORM";
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return "VK_FORMAT_X8_D24_UNORM_PACK32";
		case VK_FORMAT_D32_SFLOAT:
			return "VK_FORMAT_D32_SFLOAT";
		case VK_FORMAT_S8_UINT:
			return "VK_FORMAT_S8_UINT";
		case VK_FORMAT_D16_UNORM_S8_UINT:
			return "VK_FORMAT_D16_UNORM_S8_UINT";
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return "VK_FORMAT_D24_UNORM_S8_UINT";
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return "VK_FORMAT_D32_SFLOAT_S8_UINT";
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
			return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
			return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
		case VK_FORMAT_BC2_UNORM_BLOCK:
			return "VK_FORMAT_BC2_UNORM_BLOCK";
		case VK_FORMAT_BC2_SRGB_BLOCK:
			return "VK_FORMAT_BC2_SRGB_BLOCK";
		case VK_FORMAT_BC3_UNORM_BLOCK:
			return "VK_FORMAT_BC3_UNORM_BLOCK";
		case VK_FORMAT_BC3_SRGB_BLOCK:
			return "VK_FORMAT_BC3_SRGB_BLOCK";
		case VK_FORMAT_BC4_UNORM_BLOCK:
			return "VK_FORMAT_BC4_UNORM_BLOCK";
		case VK_FORMAT_BC4_SNORM_BLOCK:
			return "VK_FORMAT_BC4_SNORM_BLOCK";
		case VK_FORMAT_BC5_UNORM_BLOCK:
			return "VK_FORMAT_BC5_UNORM_BLOCK";
		case VK_FORMAT_BC5_SNORM_BLOCK:
			return "VK_FORMAT_BC5_SNORM_BLOCK";
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return "VK_FORMAT_BC7_UNORM_BLOCK";
		case VK_FORMAT_BC7_SRGB_BLOCK:
			return "VK_FORMAT_BC7_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
			return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
			return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
			return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
			return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
			return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
			return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
			return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
		case VK_FORMAT_G8B8G8R8_422_UNORM:
			return "VK_FORMAT_G8B8G8R8_422_UNORM";
		case VK_FORMAT_B8G8R8G8_422_UNORM:
			return "VK_FORMAT_B8G8R8G8_422_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
			return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
			return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
			return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
			return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
			return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
		case VK_FORMAT_R10X6_UNORM_PACK16:
			return "VK_FORMAT_R10X6_UNORM_PACK16";
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
			return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
		case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
			return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
		case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
			return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
			return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
			return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
			return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
			return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
			return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
			return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
		case VK_FORMAT_R12X4_UNORM_PACK16:
			return "VK_FORMAT_R12X4_UNORM_PACK16";
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
			return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
			return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
			return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
			return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
			return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
			return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
			return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
			return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
			return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
		case VK_FORMAT_G16B16G16R16_422_UNORM:
			return "VK_FORMAT_G16B16G16R16_422_UNORM";
		case VK_FORMAT_B16G16R16G16_422_UNORM:
			return "VK_FORMAT_B16G16R16G16_422_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
			return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
			return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
			return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
			return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
			return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
			return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
			return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
			return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
			return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
			return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
			return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
			return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG";
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
			return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG";
		default:
			return "Unknown";
	}
}

static const char *GetVulkanPresentModeKHRName(VkPresentModeKHR mode) {
	switch(mode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return "VK_PRESENT_MODE_IMMEDIATE_KHR";
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return "VK_PRESENT_MODE_MAILBOX_KHR";
		case VK_PRESENT_MODE_FIFO_KHR:
			return "VK_PRESENT_MODE_FIFO_KHR";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
		default:
			return "Unknown Presentation Mode";
	}
}

static const char *GetVulkanColorSpaceName(const VkColorSpaceKHR value) {
	switch(value) {
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
			return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
		case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
			return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
		case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
			return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
		case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
			return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
		case VK_COLOR_SPACE_BT709_LINEAR_EXT:
			return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
		case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
			return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
		case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
			return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
		case VK_COLOR_SPACE_HDR10_ST2084_EXT:
			return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
		case VK_COLOR_SPACE_DOLBYVISION_EXT:
			return "VK_COLOR_SPACE_DOLBYVISION_EXT";
		case VK_COLOR_SPACE_HDR10_HLG_EXT:
			return "VK_COLOR_SPACE_HDR10_HLG_EXT";
		case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
			return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
		case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
			return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
		case VK_COLOR_SPACE_PASS_THROUGH_EXT:
			return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
		case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
			return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
		default:
			return "Unknown";
	}
}

typedef struct VulkanQueueFamilyIndex {
	uint32_t index;
	uint32_t maxCount;
} VulkanQueueFamilyIndex;

#define INVALID_VULKAN_QUEUE_FAMILY_INDEX fplStructInit(VulkanQueueFamilyIndex, UINT32_MAX, 0)

bool IsVulkanValidQueueFamilyIndex(const VulkanQueueFamilyIndex index) {
	bool result = (index.index < index.maxCount) && (index.index != UINT32_MAX);
	return(result);
}

bool AreVulkanQueueFamiliesEqual(const VulkanQueueFamilyIndex a, const VulkanQueueFamilyIndex b) {
	bool result = (a.maxCount == b.maxCount) && (a.index == b.index);
	return(result);
}

void VulkanVersionToString(const uint32_t versionNumber, const size_t outNameCapacity, char *outName) {
	int32_t major = VK_VERSION_MAJOR(versionNumber);
	int32_t minor = VK_VERSION_MINOR(versionNumber);
	int32_t patch = VK_VERSION_PATCH(versionNumber);

	size_t lenMajor = fplS32ToString(major, fpl_null, 0);
	size_t lenMinor = fplS32ToString(minor, fpl_null, 0);
	size_t lenPatch = fplS32ToString(patch, fpl_null, 0);

	size_t requiredLen = (lenMajor + 1 + lenMinor + 1 + lenPatch) + 3;
	fplAssert(outNameCapacity >= requiredLen);

	fplS32ToString(major, outName + 0, lenMajor + 1);
	outName[lenMajor] = '.';
	fplS32ToString(minor, outName + lenMajor + 1, lenMinor + 1);
	outName[lenMajor + 1 + lenMinor] = '.';
	fplS32ToString(patch, outName + lenMajor + 1 + lenMinor + 1, lenPatch + 1);
}

static VulkanQueueFamilyIndex GetVulkanQueueFamilyIndex(const VkQueueFlagBits flags, const VkQueueFamilyProperties *families, const uint32_t familyCount) {
	// Find a dedicated queue for compute (Not graphics)
	if((flags & VK_QUEUE_COMPUTE_BIT) > 0) {
		for(uint32_t i = 0; i < familyCount; ++i) {
			VkQueueFlagBits familyFlags = families[i].queueFlags;
			if(((familyFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) && ((familyFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
				return(fplStructInit(VulkanQueueFamilyIndex, i, families[i].queueCount));
			}
		}
	}

	// Find a dedicated queue for transfer (Not graphics and not compute)
	if((flags & VK_QUEUE_TRANSFER_BIT) > 0) {
		for(uint32_t i = 0; i < familyCount; ++i) {
			VkQueueFlagBits familyFlags = families[i].queueFlags;
			if(((familyFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) && ((familyFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((familyFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
				return(fplStructInit(VulkanQueueFamilyIndex, i, families[i].queueCount));
			}
		}
	}

	// For all other queues
	for(uint32_t i = 0; i < familyCount; ++i) {
		if((families[i].queueFlags & flags) > 0) {
			return(fplStructInit(VulkanQueueFamilyIndex, i, families[i].queueCount));
		}
	}

	return(INVALID_VULKAN_QUEUE_FAMILY_INDEX);
}

bool IsVulkanFeatureSupported(const char **features, const size_t supportedExtensionCount, const char *search) {
	for(size_t i = 0; i < supportedExtensionCount; ++i) {
		const char *feature = features[i];
		if(fplIsStringEqual(feature, search)) {
			return(true);
		}
	}
	return(false);
}

//
// Vulkan API
//
typedef struct VulkanCoreApi {
	fplDynamicLibraryHandle libHandle;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

	fpl_b32 isValid;
} VulkanCoreApi;

void VulkanUnloadCoreAPI(VulkanCoreApi *coreApi) {
	fplAssert(coreApi != fpl_null);
	if(coreApi->isValid) {
		fplConsoleFormatOut("Unload Vulkan API\n");
		fplDynamicLibraryUnload(&coreApi->libHandle);
	}
	fplClearStruct(coreApi);
}

bool VulkanLoadCoreAPI(VulkanCoreApi *coreApi) {
	fplAssert(coreApi != fpl_null);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	const char *libraryNames[] = {
		"vulkan-1.dll"
	};
#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_XLIB_KHR)
	const char *libraryNames[] = {
		"libvulkan.so",
		"libvulkan.so.1"
	};
#else
	return(false);
#endif

	fplClearStruct(coreApi);

	fplDynamicLibraryHandle *handle = &coreApi->libHandle;

#define VULKAN_LIBRARY_GET_PROC_ADDRESS(libHandle, libName, target, type, name) \
	(target)->name = (type)fplGetDynamicLibraryProc(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		VulkanUnloadCoreAPI(coreApi); \
		FPL__WARNING("Vulkan", "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		continue; \
	}

	bool success = false;
	for(uint32_t i = 0; i < fplArrayCount(libraryNames); ++i) {
		const char *vulkanLibraryFileName = libraryNames[i];
		fplConsoleFormatOut("Load Vulkan API '%s'\n", vulkanLibraryFileName);
		if(!fplDynamicLibraryLoad(vulkanLibraryFileName, &coreApi->libHandle)) {
			continue;
		}

		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkCreateInstance, vkCreateInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkDestroyInstance, vkDestroyInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkGetInstanceProcAddr, vkGetInstanceProcAddr);

		success = true;
		break;
	}

#undef VULKAN_LIBRARY_GET_PROC_ADDRESS

	if(!success) {
		VulkanUnloadCoreAPI(coreApi);
		return(false);
	}

	coreApi->isValid = true;
	return(true);
}

typedef struct VulkanInstanceApi {
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
	PFN_vkCreateDevice vkCreateDevice;
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
	PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif

	fpl_b32 isValid;
} VulkanInstanceApi;

void UnloadVulkanInstanceAPI(VulkanInstanceApi *instanceApi) {
	fplAssert(instanceApi != fpl_null);
	fplClearStruct(instanceApi);
}

bool LoadVulkanInstanceAPI(const VulkanCoreApi *coreApi, VulkanInstanceApi *instanceApi, VkInstance instanceHandle) {
	fplAssert(coreApi != fpl_null);
	fplAssert(instanceApi != fpl_null);

	if(!coreApi->isValid)
		return(false);
	if(instanceHandle == VK_NULL_HANDLE)
		return(false);

	fplClearStruct(instanceApi);

#define VULKAN_INSTANCE_GET_PROC_ADDRESS(target, type, name) \
	(target)->name = (type)coreApi->vkGetInstanceProcAddr(instanceHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting instance procedure address '%s'", #name); \
		break; \
	}

	bool success = false;
	do {

		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetDeviceProcAddr, vkGetDeviceProcAddr);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkEnumeratePhysicalDevices, vkEnumeratePhysicalDevices);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceFeatures, vkGetPhysicalDeviceFeatures);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkEnumerateDeviceExtensionProperties, vkEnumerateDeviceExtensionProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkEnumerateDeviceLayerProperties, vkEnumerateDeviceLayerProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkCreateDevice, vkCreateDevice);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkDestroySurfaceKHR, vkDestroySurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceSupportKHR, vkGetPhysicalDeviceSurfaceSupportKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, vkGetPhysicalDeviceSurfaceFormatsKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfacePresentModesKHR, vkGetPhysicalDeviceSurfacePresentModesKHR);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkCreateWin32SurfaceKHR, vkCreateWin32SurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR, vkGetPhysicalDeviceWin32PresentationSupportKHR);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkCreateXlibSurfaceKHR, vkCreateXlibSurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR, vkGetPhysicalDeviceXlibPresentationSupportKHR);
#endif

		success = true;
	} while(0);

#undef VULKAN_INSTANCE_GET_PROC_ADDRESS

	if(!success) {
		UnloadVulkanInstanceAPI(instanceApi);
		return(false);
	}

	instanceApi->isValid = true;
	return(true);
}

typedef struct VulkanDeviceApi {
	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;

	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;

	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkResetCommandPool vkResetCommandPool;

	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
	PFN_vkEndCommandBuffer vkEndCommandBuffer;
	PFN_vkResetCommandBuffer vkResetCommandBuffer;
	PFN_vkQueueSubmit vkQueueSubmit;

	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
	PFN_vkCmdClearColorImage vkCmdClearColorImage;

	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkDestroySemaphore vkDestroySemaphore;

	fpl_b32 isValid;
} VulkanDeviceApi;

void VulkanUnloadDeviceApi(VulkanDeviceApi *deviceApi) {
	fplAssert(deviceApi != fpl_null);
	fplClearStruct(deviceApi);
}

bool VulkanLoadDeviceApi(const VulkanInstanceApi *instanceApi, VulkanDeviceApi *deviceApi, VkDevice deviceHandle) {
	fplAssert(instanceApi != fpl_null);
	fplAssert(deviceApi != fpl_null);
	if(deviceHandle == VK_NULL_HANDLE) return(false);

	fplClearStruct(deviceApi);

#define VULKAN_DEVICE_GET_PROC_ADDRESS(target, type, name) \
	(target)->name = (type)instanceApi->vkGetDeviceProcAddr(deviceHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting device procedure address '%s'", #name); \
		break; \
	}

	bool success = false;
	do {
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDestroyDevice, vkDestroyDevice);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDeviceWaitIdle, vkDeviceWaitIdle);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkGetDeviceQueue, vkGetDeviceQueue);

		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCreateSwapchainKHR, vkCreateSwapchainKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDestroySwapchainKHR, vkDestroySwapchainKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkGetSwapchainImagesKHR, vkGetSwapchainImagesKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkAcquireNextImageKHR, vkAcquireNextImageKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkQueuePresentKHR, vkQueuePresentKHR);

		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCreateCommandPool, vkCreateCommandPool);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDestroyCommandPool, vkDestroyCommandPool);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkResetCommandPool, vkResetCommandPool);

		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkAllocateCommandBuffers, vkAllocateCommandBuffers);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkFreeCommandBuffers, vkFreeCommandBuffers);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkBeginCommandBuffer, vkBeginCommandBuffer);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkEndCommandBuffer, vkEndCommandBuffer);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkResetCommandBuffer, vkResetCommandBuffer);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkQueueSubmit, vkQueueSubmit);

		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCmdPipelineBarrier, vkCmdPipelineBarrier);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCmdClearColorImage, vkCmdClearColorImage);

		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCreateSemaphore, vkCreateSemaphore);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDestroySemaphore, vkDestroySemaphore);
		success = true;
	} while(0);

#undef VULKAN_DEVICE_GET_PROC_ADDRESS

	if(!success) {
		VulkanUnloadDeviceApi(deviceApi);
		return(false);
	}

	deviceApi->isValid = true;
	return(true);
}

typedef struct VulkanInstanceProperties {
	StringTable supportedLayers;
	StringTable supportedExtensions;
} VulkanInstanceProperties;

static void DestroyVulkanInstanceProperties(VulkanInstanceProperties *instanceProperties) {
	FreeStringTable(&instanceProperties->supportedExtensions);
	FreeStringTable(&instanceProperties->supportedLayers);
	fplClearStruct(instanceProperties);
}

static bool LoadVulkanInstanceProperties(const VulkanCoreApi *coreApi, VulkanInstanceProperties *outInstanceProperties) {
	fplAssert(coreApi != fpl_null);
	fplAssert(outInstanceProperties != fpl_null);

	VulkanInstanceProperties instanceProperties = fplZeroInit;

	VkResult res;

	//
	// Extensions
	//
	fplConsoleFormatOut("Enumerate instance extension properties...\n");
	uint32_t instanceExtensionCount = 0;
	res = coreApi->vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, fpl_null);
	if(res != VK_SUCCESS) {
		return(false);
	}
	VkExtensionProperties *tempInstanceExtensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	if(tempInstanceExtensions == fpl_null) {
		return(false);
	}
	res = coreApi->vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, tempInstanceExtensions);
	if(res != VK_SUCCESS) {
		free(tempInstanceExtensions);
		return(false);
	}

	instanceProperties.supportedExtensions = AllocStringTable(instanceExtensionCount);
	fplConsoleFormatOut("Successfully got instance extension properties of %lu\n", instanceExtensionCount);
	for(uint32_t extensionIndex = 0; extensionIndex < instanceExtensionCount; ++extensionIndex) {
		const VkExtensionProperties *extProp = tempInstanceExtensions + extensionIndex;
		const char *extName = extProp->extensionName;
		PushStringToTable(&instanceProperties.supportedExtensions, extName);
		fplConsoleFormatOut("- %s\n", extName);
	}

	free(tempInstanceExtensions);

	fplConsoleOut("\n");

	//
	// Layers
	//
	fplConsoleFormatOut("Enumerate instance layer properties...\n");
	uint32_t instanceLayerCount = 0;
	res = coreApi->vkEnumerateInstanceLayerProperties(&instanceLayerCount, fpl_null);
	if(res == VK_SUCCESS) {
		VkLayerProperties *tempInstanceLayers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instanceLayerCount);
		if(tempInstanceLayers != fpl_null) {
			res = coreApi->vkEnumerateInstanceLayerProperties(&instanceLayerCount, tempInstanceLayers);

			if(res == VK_SUCCESS) {
				fplConsoleFormatOut("Successfully got instance layer properties of %lu\n", instanceLayerCount);
				instanceProperties.supportedLayers = AllocStringTable(instanceLayerCount);
				for(uint32_t layerIndex = 0; layerIndex < instanceLayerCount; ++layerIndex) {
					VkLayerProperties *layerProp = tempInstanceLayers + layerIndex;
					const char *layerName = layerProp->layerName;
					PushStringToTable(&instanceProperties.supportedLayers, layerName);
					fplConsoleFormatOut("- %s\n", layerName);
				}
			}

			free(tempInstanceLayers);
		}
	}

	*outInstanceProperties = instanceProperties;
	return(true);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
	const char *severityName = GetVulkanMessageSeverityName(messageSeverity);
	fplConsoleFormatError("[%s] Validation layer: %s\n", severityName, pCallbackData->pMessage);
	return VK_FALSE;
}

static void VulkanDestroyDebugMessenger(const VulkanCoreApi *coreApi, VkInstance instanceHandle, VkDebugUtilsMessengerEXT debugMessenger) {
	fplAssert(coreApi != fpl_null);
	if(instanceHandle == VK_NULL_HANDLE) return;
	if(debugMessenger == VK_NULL_HANDLE) return;
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)coreApi->vkGetInstanceProcAddr(instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
	if(func != fpl_null) {
		func(instanceHandle, debugMessenger, fpl_null);
	}
}

static VkDebugUtilsMessengerCreateInfoEXT MakeVulkanDebugMessengerCreateInfo() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VULKANDEMO_VALIDATION_LAYER_SEVERITY;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = VulkanDebugCallback;
	createInfo.pUserData = fpl_null;
	return(createInfo);
}

static bool VulkanCreateDebugMessenger(VkAllocationCallbacks *allocator, const VulkanCoreApi *coreApi, const VkInstance instanceHandle, const VkDebugUtilsMessengerCreateInfoEXT *createInfo, VkDebugUtilsMessengerEXT *outDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)coreApi->vkGetInstanceProcAddr(instanceHandle, "vkCreateDebugUtilsMessengerEXT");
	if(func != fpl_null) {
		VkResult res = func(instanceHandle, createInfo, allocator, outDebugMessenger);
		if(res != VK_SUCCESS) {
			return(false);
		}
	} else {
		return(false);
	}

	return(true);
}

typedef struct VulkanInstance {
	VulkanInstanceProperties properties;
	VulkanInstanceApi instanceApi;
	VkApplicationInfo appInfo;
	VkInstance instanceHandle;
	fpl_b32 hasValidationLayer;
	fpl_b32 isUserDefined;
} VulkanInstance;

static void VulkanDestroyInstance(VkAllocationCallbacks *allocator, const VulkanCoreApi *coreApi, VulkanInstance *instance) {
	if(coreApi == fpl_null || instance == fpl_null) return;

	// Unload Instance API
	UnloadVulkanInstanceAPI(&instance->instanceApi);

	// Destroy Vulkan instance
	if(instance->instanceHandle != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan instance '%p'\n", instance->instanceHandle);
		coreApi->vkDestroyInstance(instance->instanceHandle, allocator);
	}

	// Destroy instance properties
	DestroyVulkanInstanceProperties(&instance->properties);

	fplClearStruct(instance);
}

static bool VulkanCreateInstance(VkAllocationCallbacks *allocator, const VulkanCoreApi *coreApi, const bool useValidation, const char **requiredExtensions, const uint32_t requiredExtensionCount, VulkanInstance *instance) {
	fplAssert(coreApi != fpl_null);
	fplAssert(instance != fpl_null);

	fplClearStruct(instance);

	VkResult res;

	VulkanInstanceProperties *instanceProperties = &instance->properties;
	if(!LoadVulkanInstanceProperties(coreApi, instanceProperties)) {
		fplConsoleFormatError("Failed loading instance properties!\n");
		return(false);
	}

	//
	// Check and validate extensions and layers
	//
	const char *supportedValidationLayerName = fpl_null;
	fplConsoleFormatOut("Validate instance layers:\n");
	for(uint32_t i = 0; i < fplArrayCount(VulkanValidationLayerNames); ++i) {
		const char *validationLayerName = VulkanValidationLayerNames[i];
		bool isSupported = IsVulkanFeatureSupported(instanceProperties->supportedLayers.items, instanceProperties->supportedLayers.count, validationLayerName);
		fplConsoleFormatOut("- Supported %s: %s\n", validationLayerName, (isSupported ? "yes" : "no"));
		if(isSupported) {
			supportedValidationLayerName = validationLayerName;
			break;
		}
	}

	bool supportsKHRSurface = IsVulkanFeatureSupported(instanceProperties->supportedExtensions.items, instanceProperties->supportedExtensions.count, VulkanKHRSurfaceName);
	bool supportsKHRPlatformSurface = IsVulkanFeatureSupported(instanceProperties->supportedExtensions.items, instanceProperties->supportedExtensions.count, VULKAN_PLATFORM_SURFACE_NAME);

	if(requiredExtensions != fpl_null && requiredExtensionCount > 0) {
		fplConsoleFormatOut("Validate %lu instance extensions:\n", requiredExtensionCount);
		bool missing = false;
		for(uint32_t i = 0; i < requiredExtensionCount; ++i) {
			const char *ext = requiredExtensions[i];
			bool isSupported = IsVulkanFeatureSupported(instanceProperties->supportedExtensions.items, instanceProperties->supportedExtensions.count, ext);
			fplConsoleFormatOut("- Supported %s: %s\n", ext, (isSupported ? "yes" : "no"));
			if(!isSupported) {
				missing = true;
			}
		}
		if(missing) {
			fplConsoleFormatError("At least one from %lu instance extension are not supported!\n", requiredExtensionCount);
			VulkanDestroyInstance(allocator, coreApi, instance);
			return(false);
		}
	} else {
		fplConsoleFormatOut("Validate instance extensions:\n");
		fplConsoleFormatOut("- Supported %s: %s\n", VulkanKHRSurfaceName, (supportsKHRSurface ? "yes" : "no"));
		fplConsoleFormatOut("- Supported %s: %s\n", VULKAN_PLATFORM_SURFACE_NAME, (supportsKHRPlatformSurface ? "yes" : "no"));
	}

	if(!supportsKHRSurface || !supportsKHRPlatformSurface) {
		fplConsoleFormatError("Not supported KHR platform!\n");
		VulkanDestroyInstance(allocator, coreApi, instance);
		return(false);
	}

	fplConsoleFormatOut("\n");

	//
	// Vulkan Instance (vkInstance)
	//
	VkApplicationInfo *appInfo = &instance->appInfo;
	appInfo->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo->pApplicationName = "FPL_Vulkan";
	appInfo->pEngineName = "FPL_Vulkan";
	appInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo->apiVersion = VK_API_VERSION_1_1;

	//
	// Extensions
	//
	uint32_t enabledInstanceExtensionCount = 0;
	const char *enabledInstanceExtensions[16] = { 0 };

	if(useValidation) {
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME; // VK_EXT_debug_utils is always supported
	}

	if(requiredExtensions != fpl_null && requiredExtensionCount > 0) {
		bool hasKHRSurface = false;
		bool hasKHRPlatformSurface = false;
		for(uint32_t i = 0; i < requiredExtensionCount; ++i) {
			const char *extName = requiredExtensions[i];
			if(fplIsStringEqual(extName, VulkanKHRSurfaceName)) {
				hasKHRSurface = true;
			}
			if(fplIsStringEqual(extName, VULKAN_PLATFORM_SURFACE_NAME)) {
				hasKHRPlatformSurface = true;
			}
		}
		if(!hasKHRSurface)
			enabledInstanceExtensions[enabledInstanceExtensionCount++] = VulkanKHRSurfaceName;
		if(!hasKHRPlatformSurface)
			enabledInstanceExtensions[enabledInstanceExtensionCount++] = VULKAN_PLATFORM_SURFACE_NAME;

		for(uint32_t i = 0; i < requiredExtensionCount; ++i) {
			const char *extName = requiredExtensions[i];
			fplAssert(enabledInstanceExtensionCount < fplArrayCount(enabledInstanceExtensions));
			enabledInstanceExtensions[enabledInstanceExtensionCount++] = extName;
		}

	} else {
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VulkanKHRSurfaceName;
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VULKAN_PLATFORM_SURFACE_NAME;
	}


	uint32_t enabledInstanceLayerCount = 0;
	const char *enabledInstanceLayers[8] = { 0 };
	if(useValidation && fplGetStringLength(supportedValidationLayerName) > 0) {
		instance->hasValidationLayer = true;
		enabledInstanceLayers[enabledInstanceLayerCount++] = supportedValidationLayerName;
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = fplZeroInit;

	VkInstanceCreateInfo instanceCreateInfo = fplZeroInit;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = appInfo;
	instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	instanceCreateInfo.enabledLayerCount = enabledInstanceLayerCount;
	instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
	instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayers;
	if(useValidation && fplGetStringLength(supportedValidationLayerName) > 0) {
		debugCreateInfo = MakeVulkanDebugMessengerCreateInfo();
		instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	}

	char appVersionName[100] = { 0 };
	char engineVersionName[100] = { 0 };
	char vulkanVersionName[100] = { 0 };
	VulkanVersionToString(appInfo->applicationVersion, fplArrayCount(appVersionName), appVersionName);
	VulkanVersionToString(appInfo->engineVersion, fplArrayCount(engineVersionName), engineVersionName);
	VulkanVersionToString(appInfo->apiVersion, fplArrayCount(vulkanVersionName), vulkanVersionName);

	fplConsoleFormatOut("Creating Vulkan instance for application '%s' v%s and engine '%s' v%s for Vulkan v%s...\n", appInfo->pApplicationName, appVersionName, appInfo->pEngineName, engineVersionName, vulkanVersionName);
	fplConsoleFormatOut("With %lu enabled extensions & %lu layers\n", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount);
	res = coreApi->vkCreateInstance(&instanceCreateInfo, allocator, &instance->instanceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating Vulkan instance for application '%s'!\n", appInfo->pApplicationName);
		VulkanDestroyInstance(allocator, coreApi, instance);
		return(false);
	}
	fplConsoleFormatOut("Successfully created instance -> '%p'\n", instance->instanceHandle);
	fplConsoleFormatOut("\n");

	//
	// Load instance API
	//
	if(!LoadVulkanInstanceAPI(coreApi, &instance->instanceApi, instance->instanceHandle)) {
		fplConsoleFormatError("Failed to load the Vulkan instance API for instance '%p'!\n", instance->instanceHandle);
		VulkanDestroyInstance(allocator, coreApi, instance);
		return(false);
	}

	return(true);
}

typedef struct VulkanQueueFamilyList {
	FIXED_TYPED_ARRAY_INNER(VkQueueFamilyProperties)
} VulkanQueueFamilyList;

typedef struct VulkanPhysicalDevice {
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VulkanQueueFamilyList queueFamilies;
	StringTable supportedExtensions;
	StringTable supportedLayers;

	VkPhysicalDevice physicalDeviceHandle;
	const char *name;
} VulkanPhysicalDevice;

void VulkanDestroyPhysicalDevice(const VulkanCoreApi *coreApi, VulkanPhysicalDevice *physicalDevice) {
	fplAssert(coreApi != fpl_null);
	fplAssert(physicalDevice != fpl_null);

	FreeStringTable(&physicalDevice->supportedLayers);
	FreeStringTable(&physicalDevice->supportedExtensions);
	FREE_FIXED_TYPED_ARRAY(&physicalDevice->queueFamilies);

	fplClearStruct(physicalDevice);
}

bool VulkanCreatePhysicalDevice(const VulkanCoreApi *coreApi, const VulkanInstanceApi *instanceApi, VulkanPhysicalDevice *physicalDevice, VkInstance instanceHandle) {
	fplAssert(coreApi != fpl_null);
	fplAssert(instanceApi != fpl_null);
	fplAssert(physicalDevice != fpl_null);
	if(instanceHandle == VK_NULL_HANDLE) return(false);

	VkResult res;

	//
	// Get Physical Devices
	//
	fplConsoleFormatOut("Enumerate physical devices for instance '%p'\n", instanceHandle);
	uint32_t physicalDeviceCount = 0;
	res = instanceApi->vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instanceHandle);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	if(physicalDevices == fpl_null) {
		fplConsoleFormatError("Failed allocating memory for %lu physical devices!\n", physicalDeviceCount);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	res = instanceApi->vkEnumeratePhysicalDevices(instanceHandle, &physicalDeviceCount, physicalDevices);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instanceHandle);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		free(physicalDevices);
	}
	fplConsoleFormatOut("Successfully enumerated physical devices, got %lu physics devices\n", physicalDeviceCount);
	fplConsoleFormatOut("\n");

	//
	// Find physical device (Discrete GPU is preferred over integrated GPU)
	//
	VkPhysicalDevice foundGpu = VK_NULL_HANDLE;
	uint32_t bestScore = 0;
	uint32_t foundGPUIndex = 0;
	for(uint32_t physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount; ++physicalDeviceIndex) {
		VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];

		VkPhysicalDeviceProperties props;
		instanceApi->vkGetPhysicalDeviceProperties(physicalDevice, &props);

		uint32_t score = 0;

		// Prefer discrete GPU
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += props.limits.maxImageDimension2D;

		// TODO(final): Rate by features?

		if(score > bestScore) {
			bestScore = score;
			foundGpu = physicalDevice;
			foundGPUIndex = physicalDeviceIndex;
		}
	}

	for(uint32_t physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount; ++physicalDeviceIndex) {
		VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];

		bool isActive = physicalDevice == foundGpu;

		VkPhysicalDeviceProperties physicalDeviceProps = fplZeroInit;
		instanceApi->vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);
		fplConsoleFormatOut("[%lu] Physical Device %s '%s' (%s)\n", physicalDeviceIndex, GetVulkanVendorName(physicalDeviceProps.vendorID), physicalDeviceProps.deviceName, GetVulkanPhysicalDeviceTypeName(physicalDeviceProps.deviceType));

		char apiVersionName[100];
		char driverVersionName[100];
		VulkanVersionToString(physicalDeviceProps.apiVersion, fplArrayCount(apiVersionName), apiVersionName);
		VulkanVersionToString(physicalDeviceProps.driverVersion, fplArrayCount(driverVersionName), driverVersionName);
		fplConsoleFormatOut("\tVersion Driver/API: %s / %s\n", driverVersionName, apiVersionName);

		VkPhysicalDeviceFeatures physicalDeviceFeatures = fplZeroInit;
		instanceApi->vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		fplConsoleFormatOut("\tGeometry shader supported: %s\n", (physicalDeviceFeatures.geometryShader ? "yes" : "no"));
		fplConsoleFormatOut("\tTesselation shader supported: %s\n", (physicalDeviceFeatures.tessellationShader ? "yes" : "no"));

		// TODO(final): Print out VkPhysicalDeviceMemoryProperties
	}

	free(physicalDevices);

	if(foundGpu == VK_NULL_HANDLE) {
		fplConsoleFormatError("No discrete or integrated GPU found. Please upgrade your Vulkan Driver!\n");
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}

	fplConsoleOut("\n");

	instanceApi->vkGetPhysicalDeviceProperties(foundGpu, &physicalDevice->properties);
	instanceApi->vkGetPhysicalDeviceFeatures(foundGpu, &physicalDevice->features);
	instanceApi->vkGetPhysicalDeviceMemoryProperties(foundGpu, &physicalDevice->memoryProperties);
	physicalDevice->physicalDeviceHandle = foundGpu;
	physicalDevice->name = physicalDevice->properties.deviceName;

	fplConsoleFormatOut("Using [%lu] Physical Device %s '%s' (%s)\n", foundGPUIndex, GetVulkanVendorName(physicalDevice->properties.vendorID), physicalDevice->name, GetVulkanPhysicalDeviceTypeName(physicalDevice->properties.deviceType));
	fplConsoleOut("\n");

	//
	// Device Extensions
	//
	{
		fplConsoleFormatOut("Enumerate device extensions for Physical Device '%s'\n", physicalDevice->name);
		uint32_t deviceExtensionCount = 0;
		instanceApi->vkEnumerateDeviceExtensionProperties(physicalDevice->physicalDeviceHandle, fpl_null, &deviceExtensionCount, fpl_null);
		if(deviceExtensionCount > 0) {
			VkExtensionProperties *extensionProperties = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
			instanceApi->vkEnumerateDeviceExtensionProperties(physicalDevice->physicalDeviceHandle, fpl_null, &deviceExtensionCount, extensionProperties);
			fplConsoleFormatOut("Successfully enumerated device extensions for Physical Device '%s', got %lu extensions\n", physicalDevice->name, deviceExtensionCount);
			physicalDevice->supportedExtensions = AllocStringTable(deviceExtensionCount);
			for(uint32_t extIndex = 0; extIndex < deviceExtensionCount; ++extIndex) {
				const char *extName = extensionProperties[extIndex].extensionName;
				PushStringToTable(&physicalDevice->supportedExtensions, extName);
				fplConsoleFormatOut("- %s\n", extName);
			}
			free(extensionProperties);
		}
		fplConsoleOut("\n");
	}

	//
	// Device Layers
	//
	{
		fplConsoleFormatOut("Enumerate device layers for Physical Device '%s'\n", physicalDevice->name);
		uint32_t deviceLayerCount = 0;
		instanceApi->vkEnumerateDeviceLayerProperties(physicalDevice->physicalDeviceHandle, &deviceLayerCount, fpl_null);
		if(deviceLayerCount > 0) {
			VkLayerProperties *layerProperties = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * deviceLayerCount);
			instanceApi->vkEnumerateDeviceLayerProperties(physicalDevice->physicalDeviceHandle, &deviceLayerCount, layerProperties);
			fplConsoleFormatOut("Successfully %lu enumerated device layers for Physical Device '%s'\n", deviceLayerCount, physicalDevice->name);
			physicalDevice->supportedLayers = AllocStringTable(deviceLayerCount);
			for(uint32_t extIndex = 0; extIndex < deviceLayerCount; ++extIndex) {
				const char *layerName = layerProperties[extIndex].layerName;
				PushStringToTable(&physicalDevice->supportedLayers, layerName);
				fplConsoleFormatOut("- %s\n", layerName);
			}
			free(layerProperties);
		}
		fplConsoleOut("\n");
	}

	//
	// Queue Families
	//

	fplConsoleFormatOut("Get queue family properties for Physical Device '%s'\n", physicalDevice->name);
	uint32_t queueFamilyPropertiesCount = 0;
	instanceApi->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDeviceHandle, &queueFamilyPropertiesCount, fpl_null);
	fplAssert(queueFamilyPropertiesCount > 0);

	ALLOC_FIXED_TYPED_ARRAY(&physicalDevice->queueFamilies, VkQueueFamilyProperties, queueFamilyPropertiesCount);
	instanceApi->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDeviceHandle, &queueFamilyPropertiesCount, physicalDevice->queueFamilies.items);
	fplConsoleFormatOut("Successfully got %lu queue family properties for Physical Device '%s'\n", queueFamilyPropertiesCount, physicalDevice->name);
	for(uint32_t queueFamilyPropertiesIndex = 0; queueFamilyPropertiesIndex < queueFamilyPropertiesCount; ++queueFamilyPropertiesIndex) {
		const VkQueueFamilyProperties *queueFamilyProps = physicalDevice->queueFamilies.items + queueFamilyPropertiesIndex;
		fplConsoleFormatOut("[%lu] Count: %lu, Flags: ", queueFamilyPropertiesIndex, queueFamilyProps->queueCount);
		int c = 0;
		if((queueFamilyProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
			if(c++ > 0) fplConsoleOut(", ");
			fplConsoleOut("VK_QUEUE_GRAPHICS_BIT");
		}
		if((queueFamilyProps->queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
			if(c++ > 0) fplConsoleOut(", ");
			fplConsoleOut("VK_QUEUE_COMPUTE_BIT");
		}
		if((queueFamilyProps->queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
			if(c++ > 0) fplConsoleOut(", ");
			fplConsoleOut("VK_QUEUE_TRANSFER_BIT");
		}
		if((queueFamilyProps->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT) {
			if(c++ > 0) fplConsoleOut(", ");
			fplConsoleOut("VK_QUEUE_SPARSE_BINDING_BIT");
		}
		if((queueFamilyProps->queueFlags & VK_QUEUE_PROTECTED_BIT) == VK_QUEUE_PROTECTED_BIT) {
			if(c++ > 0) fplConsoleOut(", ");
			fplConsoleOut("VK_QUEUE_PROTECTED_BIT");
		}
		fplConsoleOut("\n");
	}
	fplConsoleOut("\n");

	return(true);
}

typedef struct VulkanLogicalDevice {
	VkPhysicalDeviceFeatures enabledFeatures;
	VulkanDeviceApi deviceApi;
	VkDevice logicalDeviceHandle;
	VulkanQueueFamilyIndex computeQueueFamilyIndex;
	VulkanQueueFamilyIndex transferQueueFamilyIndex;
	VulkanQueueFamilyIndex graphicsQueueFamilyIndex;
} VulkanLogicalDevice;

void VulkanDestroyLogicalDevice(VkAllocationCallbacks *allocator, const VulkanInstanceApi *instanceApi, VulkanLogicalDevice *logicalDevice) {
	fplAssert(instanceApi != fpl_null);
	fplAssert(logicalDevice != fpl_null);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	if(logicalDevice->logicalDeviceHandle != fpl_null) {
		deviceApi->vkDestroyDevice(logicalDevice->logicalDeviceHandle, allocator);
	}

	VulkanUnloadDeviceApi(&logicalDevice->deviceApi);

	fplClearStruct(logicalDevice);
	logicalDevice->computeQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, UINT32_MAX);
	logicalDevice->graphicsQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, UINT32_MAX);
	logicalDevice->transferQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, UINT32_MAX);
}

bool VulkanCreateLogicalDevice(
	VkAllocationCallbacks *allocator,
	const VulkanCoreApi *coreApi,
	const VulkanInstanceApi *instanceApi,
	const VulkanPhysicalDevice *physicalDevice,
	const VkPhysicalDeviceFeatures *enabledFeatures,
	VulkanLogicalDevice *logicalDevice,
	const VkInstance instanceHandle,
	const char **reqExtensions,
	const uint32_t reqExtensionCount,
	const bool useSwapChain,
	void *pNextChain) {

	fplAssert(coreApi != fpl_null);
	fplAssert(instanceApi != fpl_null);
	fplAssert(physicalDevice != fpl_null);
	fplAssert(enabledFeatures != fpl_null);
	fplAssert(logicalDevice != fpl_null);

	if(instanceHandle == VK_NULL_HANDLE)
		return(false);

	uint32_t queueCreationInfoCount = 0;
	VkDeviceQueueCreateInfo queueCreationInfos[4] = fplZeroInit;

	const float defaultQueuePriority = 1.0f;

	fplClearStruct(logicalDevice);
	logicalDevice->computeQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	logicalDevice->graphicsQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	logicalDevice->transferQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;

	//
	// Graphics queue
	//
	fplConsoleFormatOut("Detect queue families...\n");
	logicalDevice->graphicsQueueFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	logicalDevice->computeQueueFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	logicalDevice->transferQueueFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	if(!IsVulkanValidQueueFamilyIndex(logicalDevice->graphicsQueueFamilyIndex)) {
		fplConsoleFormatError("No graphics queue family for physical device '%s' found!\n", physicalDevice->name);
		VulkanDestroyLogicalDevice(allocator, instanceApi, logicalDevice);
		return(false);
	}
	if(!IsVulkanValidQueueFamilyIndex(logicalDevice->computeQueueFamilyIndex)) {
		// Use graphics queue for compute queue
		logicalDevice->computeQueueFamilyIndex = logicalDevice->graphicsQueueFamilyIndex;
	}
	if(!IsVulkanValidQueueFamilyIndex(logicalDevice->transferQueueFamilyIndex)) {
		// Use graphics queue for transfer queue
		logicalDevice->transferQueueFamilyIndex = logicalDevice->graphicsQueueFamilyIndex;
	}
	fplAssert(IsVulkanValidQueueFamilyIndex(logicalDevice->graphicsQueueFamilyIndex) && IsVulkanValidQueueFamilyIndex(logicalDevice->computeQueueFamilyIndex) && IsVulkanValidQueueFamilyIndex(logicalDevice->transferQueueFamilyIndex));
	fplConsoleFormatOut("Successfully detected required queue families:\n");
	fplConsoleFormatOut("\tGraphics queue family: %lu (%lu)\n", logicalDevice->graphicsQueueFamilyIndex.index, logicalDevice->graphicsQueueFamilyIndex.maxCount);
	fplConsoleFormatOut("\tCompute queue family: %lu (%lu)\n", logicalDevice->computeQueueFamilyIndex.index, logicalDevice->computeQueueFamilyIndex.maxCount);
	fplConsoleFormatOut("\tTransfer queue family: %lu (%lu)\n", logicalDevice->transferQueueFamilyIndex.index, logicalDevice->transferQueueFamilyIndex.maxCount);
	fplConsoleOut("\n");

	// Add graphics queue family
	{
		fplAssert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *queueCreateInfo = &queueCreationInfos[queueCreationInfoCount++];
		queueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo->queueFamilyIndex = logicalDevice->graphicsQueueFamilyIndex.index;
		queueCreateInfo->queueCount = 1;
		queueCreateInfo->pQueuePriorities = &defaultQueuePriority;
	}

	//
	// Add dedicated compute queue
	//
	if(!AreVulkanQueueFamiliesEqual(logicalDevice->computeQueueFamilyIndex, logicalDevice->graphicsQueueFamilyIndex)) {
		fplAssert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *createInfo = &queueCreationInfos[queueCreationInfoCount++];
		createInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo->queueFamilyIndex = logicalDevice->computeQueueFamilyIndex.index;
		createInfo->queueCount = 1;
		createInfo->pQueuePriorities = &defaultQueuePriority;
	}

	//
	// Add dedicated transfer queue
	//
	if(!AreVulkanQueueFamiliesEqual(logicalDevice->transferQueueFamilyIndex, logicalDevice->graphicsQueueFamilyIndex)) {
		fplAssert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *createInfo = &queueCreationInfos[queueCreationInfoCount++];
		createInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo->queueFamilyIndex = logicalDevice->transferQueueFamilyIndex.index;
		createInfo->queueCount = 1;
		createInfo->pQueuePriorities = &defaultQueuePriority;
	}

	// We don't allow more than 16 extensions for now
	uint32_t enabledDeviceExtensionCount = 0;
	const char *enabledDeviceExtensions[16] = fplZeroInit;
	const uint32_t maxEnableDeviceExtensionCount = fplArrayCount(enabledDeviceExtensions);
	for(uint32_t i = 0; i < reqExtensionCount; ++i) {
		const char *reqExtensionName = reqExtensions[i];
		if(IsVulkanFeatureSupported(physicalDevice->supportedExtensions.items, physicalDevice->supportedExtensions.count, reqExtensionName)) {
			fplAssert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
			enabledDeviceExtensions[enabledDeviceExtensionCount++] = reqExtensions[i];
		} else {
			fplConsoleFormatError("Extension %s is not supported for the device '%s'\n", physicalDevice->name, reqExtensionName);
		}
	}

	uint32_t enabledDeviceLayerCount = 0;
	const char *enabledDeviceLayers[8] = fplZeroInit;

	// Add SwapChain KHR extension when logical device will be used for a swap chain
	if(useSwapChain) {
		fplAssert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
		enabledDeviceExtensions[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	}

	VkDeviceCreateInfo deviceCreateInfo = fplZeroInit;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queueCreationInfoCount;
	deviceCreateInfo.pQueueCreateInfos = queueCreationInfos;
	deviceCreateInfo.pEnabledFeatures = enabledFeatures;

	// If a pNext(Chain) has been passed, we need to add it to the device creation info
	VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = fplZeroInit;
	if(pNextChain != fpl_null) {
		physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDeviceFeatures2.features = *enabledFeatures;
		physicalDeviceFeatures2.pNext = pNextChain;
		deviceCreateInfo.pEnabledFeatures = fpl_null;
		deviceCreateInfo.pNext = &physicalDeviceFeatures2;
	}

	// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
	if(IsVulkanFeatureSupported(physicalDevice->supportedExtensions.items, physicalDevice->supportedExtensions.count, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
		fplAssert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
		enabledDeviceExtensions[enabledDeviceExtensionCount++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
	}

	//
	// Add Swap-Chain support
	//
	bool hasSwapChainSupport = IsVulkanFeatureSupported(physicalDevice->supportedExtensions.items, physicalDevice->supportedExtensions.count, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	if(!hasSwapChainSupport) {
		fplConsoleFormatError("The device '%s' has no support for %s. Please select a physical device which can render graphics to the screen!\n", physicalDevice->name, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		VulkanDestroyLogicalDevice(allocator, instanceApi, logicalDevice);
		return(false);
	}
	fplAssert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
	enabledDeviceExtensions[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

	// Set extensions and layers
	if(enabledDeviceExtensionCount > 0) {
		deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
		deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions;
	}
	if(enabledDeviceLayerCount > 0) {
		deviceCreateInfo.enabledLayerCount = enabledDeviceLayerCount;
		deviceCreateInfo.ppEnabledLayerNames = enabledDeviceLayers;
	}

	fplConsoleFormatOut("Creating Logical Device from physical device '%s'\n", physicalDevice->name);
	VkResult res = instanceApi->vkCreateDevice(physicalDevice->physicalDeviceHandle, &deviceCreateInfo, allocator, &logicalDevice->logicalDeviceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating the logical device from physical device '%s'!\n", physicalDevice->name);
		VulkanDestroyLogicalDevice(allocator, instanceApi, logicalDevice);
		return(false);
	}
	fplConsoleFormatOut("Successfully created logical device from physical device '%s'\n", physicalDevice->name);
	fplConsoleOut("\n");

	logicalDevice->enabledFeatures = *enabledFeatures;

	//
	// Load Device Api
	//
	fplConsoleFormatOut("Loading device API for device '%p'\n", logicalDevice->logicalDeviceHandle);
	if(!VulkanLoadDeviceApi(instanceApi, &logicalDevice->deviceApi, logicalDevice->logicalDeviceHandle)) {
		fplConsoleFormatError("Failed loading device API for device '%p'!\n", logicalDevice->logicalDeviceHandle);
		VulkanDestroyLogicalDevice(allocator, instanceApi, logicalDevice);
		return(false);
	}
	fplConsoleFormatOut("Successfully loaded device API for device '%p'\n", logicalDevice->logicalDeviceHandle);

	return(true);
}

typedef struct VulkanSurfacePresentationModeList {
	FIXED_TYPED_ARRAY_INNER(VkPresentModeKHR)
} VulkanSurfacePresentationModeList;

typedef struct VulkanSurfaceFormatList {
	FIXED_TYPED_ARRAY_INNER(VkSurfaceFormatKHR)
} VulkanSurfaceFormatList;

typedef struct VulkanSurfaceQueuesForPresentList {
	FIXED_TYPED_ARRAY_INNER(VkBool32)
} VulkanSurfaceQueuesForPresentList;

typedef struct VulkanSurface {
	VulkanSurfaceQueuesForPresentList supportedQueuesForPresent;
	VulkanSurfacePresentationModeList presentationModes;
	VulkanSurfaceFormatList surfaceFormats;
	VulkanQueueFamilyIndex graphicsQueueFamilyIndex;
	VulkanQueueFamilyIndex presentationQueueFamilyIndex;
	VkSurfaceKHR surfaceHandle;
	VkQueue graphicsQueueHandle;
	VkQueue presentationQueueHandle;
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	fpl_b32 isUserDefined;
} VulkanSurface;

void VulkanDestroySurface(VkAllocationCallbacks *allocator, const VulkanInstanceApi *instanceApi, VulkanSurface *surface, const VkInstance instanceHandle) {
	fplAssert(instanceApi != fpl_null);
	fplAssert(surface != fpl_null);

	FREE_FIXED_TYPED_ARRAY(&surface->surfaceFormats);
	FREE_FIXED_TYPED_ARRAY(&surface->presentationModes);
	FREE_FIXED_TYPED_ARRAY(&surface->supportedQueuesForPresent);
	if(surface->surfaceHandle != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan surface '%p'\n", surface->surfaceHandle);
		instanceApi->vkDestroySurfaceKHR(instanceHandle, surface->surfaceHandle, allocator);
	}

	fplClearStruct(surface);
	surface->graphicsQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	surface->presentationQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
}

bool VulkanCreateSurface(VkAllocationCallbacks *allocator, const VulkanInstanceApi *instanceApi, VulkanSurface *surface, const VkInstance instanceHandle) {
	fplAssert(instanceApi != fpl_null);
	fplAssert(surface != fpl_null);
	if(instanceHandle == VK_NULL_HANDLE) return(false);

	fplClearStruct(surface);
	surface->graphicsQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	surface->presentationQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;

	//
	// Create Surface KHR
	//
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	// TODO(final): This is just temporary, until we can query the platform window informations from FPL
	HWND windowHandle = fpl__global__AppState->window.win32.windowHandle;
	HINSTANCE appHandle = GetModuleHandle(fpl_null);

	VkWin32SurfaceCreateInfoKHR createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = windowHandle;
	createInfo.hinstance = appHandle;

	fplConsoleFormatOut("Creating win32 surface KHR for window handle '%p' and instance '%p'\n", createInfo.hwnd, instanceHandle);
	VkResult res = instanceApi->vkCreateWin32SurfaceKHR(instanceHandle, &createInfo, allocator, &surface->surfaceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating win32 surface KHR for instance '%p'!\n", instanceHandle);
		VulkanDestroySurface(allocator, instanceApi, surface, instanceHandle);
		return(false);
	}
	fplConsoleFormatOut("Successfully created win32 surface KHR for window handle '%p' and instance '%p' -> '%p'\n\n", createInfo.hwnd, instanceHandle, surface->surfaceHandle);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	// TODO(final): This is just temporary, until we can query the platform window informations from FPL
	Window window = fpl__global__AppState->window.x11.window;
	Display *display = fpl__global__AppState->window.x11.display;

	VkXlibSurfaceCreateInfoKHR createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	createInfo.dpy = display;
	createInfo.window = window;

	fplConsoleFormatOut("Creating X11 surface KHR for window '%UL', display '%p' and instance '%p'\n", window, display, instanceHandle);
	VkResult res = instanceApi->vkCreateXlibSurfaceKHR(instanceHandle, &createInfo, allocator, &surface->surfaceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating X11 surface KHR for window '%UL', display '%p' and instance '%p'!\n", window, display, instanceHandle);
		VulkanDestroySurface(allocator, instanceApi, surface, instanceHandle);
		return(false);
	}
	fplConsoleFormatOut("Successfully created X11 surface KHR for window '%UL', display '%p' and instance '%p' -> '%p'\n\n", window, display, instanceHandle, surface->surfaceHandle);
#else
	fplConsoleFormatError("Unsupported Platform!\n");
	return(false);
#endif

	return(true);
}

static bool QueryVulkanSurfaceProperties(const VulkanInstanceApi *instanceApi, const VulkanPhysicalDevice *physicalDevice, const VulkanLogicalDevice *logicalDevice, VulkanSurface *surface, const VkInstance instanceHandle) {
	fplAssert(instanceApi != fpl_null);
	fplAssert(physicalDevice != fpl_null);
	fplAssert(logicalDevice != fpl_null);
	fplAssert(surface != fpl_null);

	if(instanceHandle == VK_NULL_HANDLE) return(false);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	VkResult res;

	//
	// Check for presentation support in queues
	//
	fplConsoleFormatOut("Get present supports for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	const uint32_t queueFamilyCount = physicalDevice->queueFamilies.itemCount;
	ALLOC_FIXED_TYPED_ARRAY(&surface->supportedQueuesForPresent, VkBool32, queueFamilyCount);
	for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {

		VkBool32 supported = false;
		instanceApi->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->physicalDeviceHandle, queueFamilyIndex, surface->surfaceHandle, &supported);
		surface->supportedQueuesForPresent.items[queueFamilyIndex] = supported;
		fplConsoleFormatOut("[%lu] supported: %s\n", queueFamilyIndex, (supported ? "yes" : "no"));
	}
	fplConsoleOut("\n");

	// Search for a graphics and a present queue in the array of queue families, try to find one that supports both
	fplConsoleFormatOut("Search graphics and presentation queue family\n");
	VulkanQueueFamilyIndex graphicsQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	VulkanQueueFamilyIndex presentQueueFamilyIndex = INVALID_VULKAN_QUEUE_FAMILY_INDEX;
	for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
		const VkQueueFamilyProperties *queueProps = physicalDevice->queueFamilies.items + queueFamilyIndex;
		const uint32_t queueCount = queueProps->queueCount;
		if((queueProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if(graphicsQueueFamilyIndex.index == UINT32_MAX) {
				graphicsQueueFamilyIndex.index = queueFamilyIndex;
				graphicsQueueFamilyIndex.maxCount = queueCount;
			}

			if(surface->supportedQueuesForPresent.items[queueFamilyIndex]) {
				graphicsQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, queueFamilyIndex, queueCount);
				presentQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, queueFamilyIndex, queueCount);
				break;
			}
		}
	}

	if(!IsVulkanValidQueueFamilyIndex(presentQueueFamilyIndex)) {
		// If there's no queue that supports both present and graphics, try to find a separate present queue
		for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
			const VkQueueFamilyProperties *queueProps = physicalDevice->queueFamilies.items + queueFamilyIndex;
			const uint32_t queueCount = queueProps->queueCount;
			if(surface->supportedQueuesForPresent.items[queueFamilyIndex]) {
				presentQueueFamilyIndex = fplStructInit(VulkanQueueFamilyIndex, queueFamilyIndex, queueCount);
				break;
			}
		}
	}

	fplConsoleFormatOut("Graphics queue family: %lu (%lu)\n", graphicsQueueFamilyIndex.index, graphicsQueueFamilyIndex.maxCount);
	fplConsoleFormatOut("Presentation queue family: %lu (%lu)\n", presentQueueFamilyIndex.index, graphicsQueueFamilyIndex.maxCount);

	if(!IsVulkanValidQueueFamilyIndex(graphicsQueueFamilyIndex) || !IsVulkanValidQueueFamilyIndex(presentQueueFamilyIndex)) {
		fplConsoleFormatError("Could not find queue families for graphics or presentation!\n");
		return(false);
	}
	if(!AreVulkanQueueFamiliesEqual(graphicsQueueFamilyIndex, presentQueueFamilyIndex)) {
		fplConsoleFormatError("Separate presentation queues are not supported!\n");
		return(false);
	}

	fplConsoleOut("\n");

	surface->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	surface->presentationQueueFamilyIndex = presentQueueFamilyIndex;

	//
	// Queues Handles
	//
	fplAssert(IsVulkanValidQueueFamilyIndex(surface->graphicsQueueFamilyIndex));
	fplAssert(IsVulkanValidQueueFamilyIndex(surface->presentationQueueFamilyIndex));
	uint32_t graphicsQueueIndex = 0; // We use the first graphics queue
	uint32_t presentationQueueIndex = 0; // We use the first presentation queue
	surface->graphicsQueueHandle = VK_NULL_HANDLE;
	surface->presentationQueueHandle = VK_NULL_HANDLE;
	deviceApi->vkGetDeviceQueue(logicalDevice->logicalDeviceHandle, surface->graphicsQueueFamilyIndex.index, graphicsQueueIndex, &surface->graphicsQueueHandle);
	deviceApi->vkGetDeviceQueue(logicalDevice->logicalDeviceHandle, surface->presentationQueueFamilyIndex.index, graphicsQueueIndex, &surface->presentationQueueHandle);
	fplAssert(surface->graphicsQueueHandle != VK_NULL_HANDLE);
	fplAssert(surface->presentationQueueHandle != VK_NULL_HANDLE);

	//
	// Find supported formats
	//
	fplConsoleFormatOut("Get surface formats for physical device '%s' and surface '%p'...\n", physicalDevice->name, surface->surfaceHandle);
	uint32_t formatCount = 0;
	res = instanceApi->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &formatCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface format count for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		return(false);
	}
	fplAssert(formatCount > 0);
	ALLOC_FIXED_TYPED_ARRAY(&surface->surfaceFormats, VkSurfaceFormatKHR, formatCount);
	res = instanceApi->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &formatCount, surface->surfaceFormats.items);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get %lu surface formats for physical device '%s' and surface '%p'!\n", formatCount, physicalDevice->name, surface->surfaceHandle);
		return(false);
	}

	//
	// Get Presentation Modes
	//
	fplConsoleFormatOut("Get surface presentation modes for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	uint32_t presentationModeCount = 0;
	res = instanceApi->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &presentationModeCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface presentation mode count for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		return(false);
	}
	ALLOC_FIXED_TYPED_ARRAY(&surface->presentationModes, VkPresentModeKHR, presentationModeCount);
	res = instanceApi->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &presentationModeCount, surface->presentationModes.items);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get %lu surface presentation modes for physical device '%s' and surface '%p'!\n", presentationModeCount, physicalDevice->name, surface->surfaceHandle);
		return(false);
	}

	// Use first format initially (Worst case)
	surface->colorFormat = surface->surfaceFormats.items[0].format;
	surface->colorSpace = surface->surfaceFormats.items[0].colorSpace;

	bool found = false;
	for(uint32_t formatIndex = 0; formatIndex < formatCount; ++formatIndex) {
		const VkSurfaceFormatKHR *format = surface->surfaceFormats.items + formatIndex;
		if(!found && format->format == VK_FORMAT_B8G8R8A8_UNORM) {
			surface->colorFormat = format->format;
			surface->colorSpace = format->colorSpace;
			found = true;
		}
		if(!found && formatCount == 1 && format->format == VK_FORMAT_UNDEFINED) {
			// Special case when format is not preferred
			surface->colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			surface->colorSpace = format->colorSpace;
			found = true;
		}
		const char *formatName = GetVulkanFormatName(format->format);
		const char *colorspaceName = GetVulkanColorSpaceName(format->colorSpace);
		fplConsoleFormatOut("[%lu] '%s' with color-space of '%s'\n", formatIndex, formatName, colorspaceName);
	}
	fplConsoleFormatOut("Successfully got %lu surface formats for physical device '%s' and surface '%p'\n", formatCount, physicalDevice->name, surface->surfaceHandle);
	fplConsoleOut("\n");

	return(true);
}

#define MAX_SWAPCHAIN_IMAGE_COUNT 8

typedef struct VulkanSwapChain {
	VkSurfaceCapabilitiesKHR capabilities;
	VkExtent2D extent;
	VkSwapchainKHR swapChainHandle;
	VkCommandPool presentationCommandPoolHandle;
	VkCommandBuffer presentationCommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT];
	VkImage images[MAX_SWAPCHAIN_IMAGE_COUNT];
	uint32_t imageCount;
	fpl_b32 isVSync;
} VulkanSwapChain;

static void VulkanClearSwapChain(VkAllocationCallbacks *allocator, const VulkanLogicalDevice *logicalDevice, VulkanSwapChain *swapChain) {
	fplAssert(logicalDevice != fpl_null);
	fplAssert(swapChain != fpl_null);

	if(logicalDevice->logicalDeviceHandle == VK_NULL_HANDLE) return;
	if(swapChain->swapChainHandle == VK_NULL_HANDLE) return;

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	deviceApi->vkDeviceWaitIdle(logicalDevice->logicalDeviceHandle);

	if(swapChain->imageCount > 0 && swapChain->presentationCommandBuffers[0] != VK_NULL_HANDLE) {
		deviceApi->vkFreeCommandBuffers(logicalDevice->logicalDeviceHandle, swapChain->presentationCommandPoolHandle, swapChain->imageCount, swapChain->presentationCommandBuffers);
		fplClearStruct(swapChain->presentationCommandBuffers);
	}

	if(swapChain->presentationCommandPoolHandle != fpl_null) {
		deviceApi->vkDestroyCommandPool(logicalDevice->logicalDeviceHandle, swapChain->presentationCommandPoolHandle, allocator);
		swapChain->presentationCommandPoolHandle = VK_NULL_HANDLE;
	}
}

void VulkanDestroySwapChain(VkAllocationCallbacks *allocator, const VulkanLogicalDevice *logicalDevice, VulkanSwapChain *swapChain) {
	fplAssert(logicalDevice != fpl_null);
	fplAssert(swapChain != fpl_null);

	if(logicalDevice->logicalDeviceHandle == VK_NULL_HANDLE) return;
	if(swapChain->swapChainHandle == VK_NULL_HANDLE) return;

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	VulkanClearSwapChain(allocator, logicalDevice, swapChain);

	if(swapChain->swapChainHandle != VK_NULL_HANDLE) {
		deviceApi->vkDestroySwapchainKHR(logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle, allocator);
	}

	fplClearStruct(swapChain);
}

bool VulkanCreateSwapChain(
	VkAllocationCallbacks *allocator,
	const VulkanInstance *instance,
	const VulkanPhysicalDevice *physicalDevice,
	const VulkanLogicalDevice *logicalDevice,
	const VulkanSurface *surface,
	VulkanSwapChain *swapChain,
	VkSwapchainKHR oldSwapchainHandle,
	const VkExtent2D requestedSize,
	const bool isVSync) {

	fplAssert(instance != fpl_null);
	fplAssert(physicalDevice != fpl_null);
	fplAssert(logicalDevice != fpl_null);
	fplAssert(surface != fpl_null);
	fplAssert(swapChain != fpl_null);

	if(logicalDevice->logicalDeviceHandle == VK_NULL_HANDLE)
		return(false);
	if(surface->surfaceHandle == VK_NULL_HANDLE)
		return(false);

	VkResult res;

	const VulkanInstanceApi *instanceApi = &instance->instanceApi;
	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	// We may need to wait until the device is idle
	deviceApi->vkDeviceWaitIdle(logicalDevice->logicalDeviceHandle);

	//
	// Get Surface Capabilties
	//
	fplConsoleFormatOut("Get surface capabilities for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	res = instanceApi->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &swapChain->capabilities);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface capabilities for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		return(false);
	}
	fplConsoleFormatOut("Successfully got surface capabilities for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	fplConsoleOut("\n");

	// Determine the number of images
	const VkSurfaceCapabilitiesKHR *caps = &swapChain->capabilities;
	uint32_t desiredNumberOfSwapchainImages = caps->minImageCount + 1;
	uint32_t actualNumberOfSwapchainImages = fplMin(desiredNumberOfSwapchainImages, caps->maxImageCount);

	VkExtent2D swapchainExtent = fplZeroInit;
	swapchainExtent.width = fplMax(caps->minImageExtent.width, fplMin(caps->maxImageExtent.width, requestedSize.width));
	swapchainExtent.height = fplMax(caps->minImageExtent.height, fplMin(caps->maxImageExtent.height, requestedSize.height));

	swapChain->extent = swapchainExtent;

	// Create suface format from found color space and format
	VkColorSpaceKHR colorSpace = surface->colorSpace;
	VkFormat format = surface->colorFormat;
	fplConsoleFormatOut("Use color space: %s\n", GetVulkanColorSpaceName(colorSpace));
	fplConsoleFormatOut("Use color format: %s\n", GetVulkanFormatName(format));

	//
	// Find presentation mode
	//
	uint32_t bestPresentationModeScore = 0;
	VkPresentModeKHR bestPresentationMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	for(uint32_t presentationModeIndex = 0; presentationModeIndex < surface->presentationModes.itemCount; ++presentationModeIndex) {
		VkPresentModeKHR presentMode = surface->presentationModes.items[presentationModeIndex];

		uint32_t score = 0;
		if(isVSync) {
			if(presentMode == VK_PRESENT_MODE_FIFO_KHR) {
				score += 10;
			} else if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				score += 1000;
			}
		} else {
			if(presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				score += 1000;
			}
		}

		if(score > bestPresentationModeScore) {
			bestPresentationModeScore = score;
			bestPresentationMode = presentMode;
		}
	}

	if(bestPresentationMode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
		fplConsoleFormatOut("Warning: No presentation mode found, use VK_PRESENT_MODE_FIFO_KHR as fallback!");
		bestPresentationMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	fplConsoleFormatOut("Use presentation mode: %s\n", GetVulkanPresentModeKHRName(bestPresentationMode));

	fplConsoleFormatOut("\n");

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if(caps->supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		preTransform = caps->currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	{
		VkCompositeAlphaFlagBitsKHR testFlagBits[] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};
		for(uint32_t i = 0; i < fplArrayCount(testFlagBits); ++i) {
			VkCompositeAlphaFlagBitsKHR testFlagBit = testFlagBits[i];
			if((caps->supportedCompositeAlpha & testFlagBit) == testFlagBit) {
				compositeAlpha = caps->supportedCompositeAlpha;
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = fplZeroInit;
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface->surfaceHandle;
	swapChainCreateInfo.minImageCount = actualNumberOfSwapchainImages;
	swapChainCreateInfo.imageFormat = format;
	swapChainCreateInfo.imageColorSpace = colorSpace;
	swapChainCreateInfo.imageExtent = swapchainExtent;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.presentMode = bestPresentationMode;

	// Queue families
	uint32_t queueIndices[] = { surface->graphicsQueueFamilyIndex.index,  surface->presentationQueueFamilyIndex.index };
	if(queueIndices[0] != queueIndices[1]) {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainCreateInfo.queueFamilyIndexCount = 2;
	} else {
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 1;
	}
	swapChainCreateInfo.pQueueFamilyIndices = queueIndices;

	// Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
	swapChainCreateInfo.oldSwapchain = oldSwapchainHandle;

	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.compositeAlpha = compositeAlpha;

	// Enable transfer source on swap chain images if supported
	if(caps->supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if(caps->supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	fplConsoleFormatOut("Creating Swap-Chain for device '%p' with size of %lu x %lu\n", logicalDevice->logicalDeviceHandle, swapchainExtent.width, swapchainExtent.height);
	res = deviceApi->vkCreateSwapchainKHR(logicalDevice->logicalDeviceHandle, &swapChainCreateInfo, allocator, &swapChain->swapChainHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating Swap-Chain for device '%p' with size of %lu x %lu!\n", logicalDevice->logicalDeviceHandle, swapchainExtent.width, swapchainExtent.height);
		VulkanDestroySwapChain(allocator, logicalDevice, swapChain);
		return(false);
	}
	fplConsoleFormatOut("Successfully created Swap-Chain for device '%p' with size of %lu x %lu -> %p\n\n", logicalDevice->logicalDeviceHandle, swapchainExtent.width, swapchainExtent.height, swapChain->swapChainHandle);

	// Destroy old swap chain
	if(oldSwapchainHandle != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy previous Swap-Chain '%p' for device '%p'\n", oldSwapchainHandle, logicalDevice->logicalDeviceHandle);
		deviceApi->vkDestroySwapchainKHR(logicalDevice->logicalDeviceHandle, oldSwapchainHandle, allocator);
	}

	//
	// Get images
	//
	fplConsoleFormatOut("Get swap-chain images for device '%p' and swap-chain '%p'\n", logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle);
	swapChain->imageCount = 0;
	fplClearStruct(swapChain->images);
	res = deviceApi->vkGetSwapchainImagesKHR(logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle, &swapChain->imageCount, fpl_null);
	if(res != VK_SUCCESS || swapChain->imageCount > MAX_SWAPCHAIN_IMAGE_COUNT) {
		fplConsoleFormatError("Failed to get swap-chain images for device '%p' and swap-chain '%p' or the image-count of '&%lu' exceeds the maximum available count of %lu!\n", logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle, swapChain->imageCount, MAX_SWAPCHAIN_IMAGE_COUNT);
		VulkanDestroySwapChain(allocator, logicalDevice, swapChain);
		return(false);
	}
	res = deviceApi->vkGetSwapchainImagesKHR(logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle, &swapChain->imageCount, swapChain->images);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get swap-chain images for device '%p' and swap-chain '%p' or the image-count of '&%lu' exceeds the maximum available count of %lu!\n", logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle, swapChain->imageCount, MAX_SWAPCHAIN_IMAGE_COUNT);
		VulkanDestroySwapChain(allocator, logicalDevice, swapChain);
		return(false);
	}
	fplConsoleFormatOut("Successfully got %lu swap-chain images for device '%p' and swap-chain '%p'\n\n", swapChain->imageCount, logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle);

	//
	// Presentation Command Pool
	// 
	VkCommandPoolCreateInfo cmdPoolCreateInfo = fplZeroInit;
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.queueFamilyIndex = surface->presentationQueueFamilyIndex.index;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	fplConsoleFormatOut("Create presentation command pool for device '%p' and queue family '%lu'\n", logicalDevice->logicalDeviceHandle, cmdPoolCreateInfo.queueFamilyIndex);
	res = deviceApi->vkCreateCommandPool(logicalDevice->logicalDeviceHandle, &cmdPoolCreateInfo, allocator, &swapChain->presentationCommandPoolHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to create the presentation command pool for device '%p' and queue family '%lu'!\n\n", logicalDevice->logicalDeviceHandle, cmdPoolCreateInfo.queueFamilyIndex);
		VulkanDestroySwapChain(allocator, logicalDevice, swapChain);
		return(false);
	}
	fplConsoleFormatOut("Successfully created presentation command pool for device '%p' and queue family '%lu'\n\n", logicalDevice->logicalDeviceHandle, cmdPoolCreateInfo.queueFamilyIndex);

	//
	// Command Buffers
	//
	fplConsoleFormatOut("Create %lu command buffers for device '%p' and swap chain '%p'\n", swapChain->imageCount, logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle);
	VkCommandBufferAllocateInfo cmdBufferAllocateInfo = fplZeroInit;
	cmdBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocateInfo.commandPool = swapChain->presentationCommandPoolHandle;
	cmdBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocateInfo.commandBufferCount = swapChain->imageCount;
	fplClearStruct(swapChain->presentationCommandBuffers);
	res = deviceApi->vkAllocateCommandBuffers(logicalDevice->logicalDeviceHandle, &cmdBufferAllocateInfo, swapChain->presentationCommandBuffers);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to create %lu command buffers for device '%p' and swap chain '%p'!\n", swapChain->imageCount, logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle);
		VulkanDestroySwapChain(allocator, logicalDevice, swapChain);
		return(false);
	}
	fplConsoleFormatOut("Successfully created %lu command buffers for device '%p' and swap chain '%p'\n\n", swapChain->imageCount, logicalDevice->logicalDeviceHandle, swapChain->swapChainHandle);

	swapChain->isVSync = isVSync;

	return(true);
}

typedef struct VulkanFrame {
	VulkanSwapChain swapChain;
	VkSemaphore imageAvailableSemaphoreHandle;
	VkSemaphore renderCompleteSemaphoreHandle;
} VulkanFrame;

static void VulkanTemporaryRecordBuffer(const VulkanLogicalDevice *logicalDevice, VulkanFrame *frame) {
	fplAssert(logicalDevice != fpl_null);
	fplAssert(frame != fpl_null);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	VkCommandBufferBeginInfo cmdBufferBeginInfo = fplZeroInit;
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkClearColorValue clearColor = fplStructInit(VkClearColorValue, 0.392f, 0.584f, 0.929f, 1.0f);

	VkImageSubresourceRange imageSubresourceRange = fplZeroInit;
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	for(uint32_t imageIndex = 0; imageIndex < frame->swapChain.imageCount; ++imageIndex) {
		VkCommandBuffer cmdBuffer = frame->swapChain.presentationCommandBuffers[imageIndex];
		VkImage image = frame->swapChain.images[imageIndex];

		VkImageMemoryBarrier barrierFromPresentToClear = fplZeroInit;
		barrierFromPresentToClear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromPresentToClear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromPresentToClear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrierFromPresentToClear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrierFromPresentToClear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrierFromPresentToClear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrierFromPresentToClear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrierFromPresentToClear.image = image;
		barrierFromPresentToClear.subresourceRange = imageSubresourceRange;

		VkImageMemoryBarrier barrierFromClearToPresent = fplZeroInit;
		barrierFromClearToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierFromClearToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrierFromClearToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrierFromClearToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrierFromClearToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrierFromClearToPresent.srcQueueFamilyIndex = 0;
		barrierFromClearToPresent.dstQueueFamilyIndex = 0;
		barrierFromClearToPresent.image = image;
		barrierFromClearToPresent.subresourceRange = imageSubresourceRange;

		deviceApi->vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);

		deviceApi->vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			fpl_null,
			0,
			fpl_null,
			1,
			&barrierFromPresentToClear
		);

		deviceApi->vkCmdClearColorImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageSubresourceRange);

		deviceApi->vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0,
			fpl_null,
			0,
			fpl_null,
			1,
			&barrierFromClearToPresent
		);

		deviceApi->vkEndCommandBuffer(cmdBuffer);
	}
}

void VulkanDestroyFrame(VkAllocationCallbacks *allocator, const VulkanLogicalDevice *logicalDevice, VulkanFrame *frame) {
	fplAssert(logicalDevice != fpl_null);
	fplAssert(frame != fpl_null);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	// Destroy Semaphores
	if(frame->imageAvailableSemaphoreHandle != VK_NULL_HANDLE) {
		deviceApi->vkDestroySemaphore(logicalDevice->logicalDeviceHandle, frame->imageAvailableSemaphoreHandle, allocator);
	}
	if(frame->renderCompleteSemaphoreHandle != VK_NULL_HANDLE) {
		deviceApi->vkDestroySemaphore(logicalDevice->logicalDeviceHandle, frame->renderCompleteSemaphoreHandle, allocator);
	}

	// Destroy Swap Chain
	VulkanDestroySwapChain(allocator, logicalDevice, &frame->swapChain);

	fplClearStruct(frame);
}

bool VulkanCreateFrame(VkAllocationCallbacks *allocator, const VulkanInstance *instance, const VulkanPhysicalDevice *physicalDevice, const VulkanLogicalDevice *logicalDevice, const VulkanSurface *surface, VulkanFrame *frame, const VkExtent2D size, const bool vsync) {
	fplAssert(instance != fpl_null);
	fplAssert(physicalDevice != fpl_null);
	fplAssert(logicalDevice != fpl_null);
	fplAssert(surface != fpl_null);
	fplAssert(frame != fpl_null);

	if(logicalDevice->logicalDeviceHandle == VK_NULL_HANDLE)
		return(false);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	//
	// Create Semaphores
	//
	VkSemaphoreCreateInfo semaphoreCreateInfo = fplZeroInit;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult res;

	fplConsoleFormatOut("Creating required semaphores for device '%p'\n", logicalDevice->logicalDeviceHandle);
	res = deviceApi->vkCreateSemaphore(logicalDevice->logicalDeviceHandle, &semaphoreCreateInfo, allocator, &frame->imageAvailableSemaphoreHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to create the image available semaphore for device '%p'!\n", logicalDevice->logicalDeviceHandle);
		VulkanDestroyFrame(allocator, logicalDevice, frame);
		return(false);
	}
	res = deviceApi->vkCreateSemaphore(logicalDevice->logicalDeviceHandle, &semaphoreCreateInfo, allocator, &frame->renderCompleteSemaphoreHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to create the render completion semaphore for device '%p'!\n", logicalDevice->logicalDeviceHandle);
		VulkanDestroyFrame(allocator, logicalDevice, frame);
		return(false);
	}
	fplConsoleFormatOut("Successfully created required semaphores for device '%p'\n", logicalDevice->logicalDeviceHandle);
	fplConsoleFormatOut("\n");

	//
	// Create swap chain
	//
	VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE; // Initially we dont have a previous swap-chain
	if(!VulkanCreateSwapChain(allocator, instance, physicalDevice, logicalDevice, surface, &frame->swapChain, oldSwapChain, size, vsync)) {
		fplConsoleFormatError("Failed to create a swap-chain for device '%p' with size of %lu x %lu'!\n", logicalDevice->logicalDeviceHandle, size.width, size.height);
		VulkanDestroyFrame(allocator, logicalDevice, frame);
		return(false);
	}

	// Temporary Record buffer (Clear only)
	VulkanTemporaryRecordBuffer(logicalDevice, frame);

	return(true);
}

typedef struct VulkanState {
	VulkanPhysicalDevice physicalDevice;

	VulkanLogicalDevice logicalDevice;

	VulkanCoreApi coreApi;

	VkAllocationCallbacks __allocationCallbacks;
	VkAllocationCallbacks *allocator;

	VulkanInstance instance;

	VulkanSurface surface;

	VulkanFrame frame;

	VkDebugUtilsMessengerEXT debugMessenger;

	fpl_b32 isInitialized;
} VulkanState;

static void VulkanShutdownStepRest(VulkanState *state) {
	fplAssert(state != fpl_null);

	VkAllocationCallbacks *allocator = state->allocator;
	const VulkanDeviceApi *deviceApi = &state->logicalDevice.deviceApi;

	// Clear any commands
	VulkanClearSwapChain(allocator, &state->logicalDevice, &state->frame.swapChain);

	// We may need to wait until the device is idle
	if(state->logicalDevice.logicalDeviceHandle != VK_NULL_HANDLE) {
		deviceApi->vkDeviceWaitIdle(state->logicalDevice.logicalDeviceHandle);
	}

	// Destroy Frame
	VulkanDestroyFrame(allocator, &state->logicalDevice, &state->frame);

	// Destroy Logical Device
	VulkanDestroyLogicalDevice(allocator, &state->instance.instanceApi, &state->logicalDevice);

	// Destroy Physical device
	VulkanDestroyPhysicalDevice(&state->coreApi, &state->physicalDevice);

	// Destroy Surface, but only when not used defined
	if(!state->surface.isUserDefined) {
		VulkanDestroySurface(allocator, &state->instance.instanceApi, &state->surface, state->instance.instanceHandle);
	}

	// @NOTE(final): Do not destroy the instance or unload the api here, because it will crash while the windowing system is still active
}

static void VulkanShutdownStepInit(VulkanState *state) {
	fplAssert(state != fpl_null);

	VkAllocationCallbacks *allocator = state->allocator;

	// Destroy Instance (Only when not passed by user)
	if(!state->instance.isUserDefined) {
		// Destroy debug messenger
		if(state->instance.hasValidationLayer) {
			VulkanDestroyDebugMessenger(&state->coreApi, state->instance.instanceHandle, state->debugMessenger);
		}

		VulkanDestroyInstance(allocator, &state->coreApi, &state->instance);
	}

	VulkanUnloadCoreAPI(&state->coreApi);

	fplClearStruct(state);
}

static void VulkanShutdownAll(VulkanState *state) {
	VulkanShutdownStepRest(state);
	VulkanShutdownStepInit(state);
}

static bool VulkanInitializeStepInit(VulkanState *state, const bool createInstance, const char **instanceExtensions, const uint32_t instanceExtensionCount) {
	fplClearStruct(state);
	if(!VulkanLoadCoreAPI(&state->coreApi)) {
		return(false);
	}

	// TODO(final): Setup allocation callbacks!
	//state->__allocationCallbacks

	VkAllocationCallbacks *allocator = state->allocator = fpl_null;

	if(createInstance) {
#if VULKANDEMO_USE_VALIDATION_LAYER
		bool useValidations = true;
#else
		bool useValidations = false;
#endif

		VulkanCoreApi *coreApi = &state->coreApi;

		//
		// Create instance
		//
		fplConsoleFormatOut("*************************************************************************\n");
		fplConsoleFormatOut("Instance\n");
		fplConsoleFormatOut("*************************************************************************\n");
		if(!VulkanCreateInstance(allocator, coreApi, useValidations, instanceExtensions, instanceExtensionCount, &state->instance)) {
			fplConsoleFormatError("Failed to create a Vulkan instance!\n");
			VulkanShutdownStepInit(state);
			return(false);
		}

		//
		// Debug messenger
		//
		if(state->instance.hasValidationLayer) {
			fplConsoleFormatOut("*************************************************************************\n");
			fplConsoleFormatOut("Debug Messenger\n");
			fplConsoleFormatOut("*************************************************************************\n");
			VkDebugUtilsMessengerCreateInfoEXT createInfo = MakeVulkanDebugMessengerCreateInfo();
			if(!VulkanCreateDebugMessenger(allocator, coreApi, state->instance.instanceHandle, &createInfo, &state->debugMessenger)) {
				fplConsoleFormatError("Failed to create the Vulkan debug messenger!\n");
			}
		}
	}

	return(true);
}

static bool VulkanInitializeStepRest(VulkanState *state, const uint32_t winWidth, const uint32_t winHeight) {
	fplAssert(state != fpl_null);

	VkAllocationCallbacks *allocator = state->allocator;
	VulkanCoreApi *coreApi = &state->coreApi;
	VulkanInstanceApi *instanceApi = &state->instance.instanceApi;
	VulkanDeviceApi *deviceApi = &state->logicalDevice.deviceApi;

	// https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-1.html
	// https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-2.html

	fplAssert(coreApi->isValid);

	if(state->instance.instanceHandle != VK_NULL_HANDLE) {
		// We have an existing instance, just the load the instance API
		if(!LoadVulkanInstanceAPI(coreApi, &state->instance.instanceApi, state->instance.instanceHandle)) {
			fplConsoleFormatError("Failed to load the Vulkan instance API for instance '%p'!\n", state->instance.instanceHandle);
			goto failed;
		}
	}

	// Create surface
	if(state->surface.surfaceHandle == VK_NULL_HANDLE) {
		fplConsoleFormatOut("*************************************************************************\n");
		fplConsoleFormatOut("Surface Step 1/2\n");
		fplConsoleFormatOut("*************************************************************************\n");
		if(!VulkanCreateSurface(allocator, instanceApi, &state->surface, state->instance.instanceHandle)) {
			fplConsoleFormatError("Failed to create surface for instance '%p'!\n", state->instance.instanceHandle);
			goto failed;
		}
	} else {
		state->surface.isUserDefined = true;
	}

	//
	// Physical Device (vkPhysicalDevice)
	//
	fplConsoleFormatOut("*************************************************************************\n");
	fplConsoleFormatOut("Physical Device\n");
	fplConsoleFormatOut("*************************************************************************\n");
	if(!VulkanCreatePhysicalDevice(coreApi, instanceApi, &state->physicalDevice, state->instance.instanceHandle)) {
		fplConsoleFormatError("Failed to find a physical device from instance '%p'!\n", state->instance.instanceHandle);
		goto failed;
	}

	//
	// Logical Device (vkDevice)
	//
	fplConsoleFormatOut("*************************************************************************\n");
	fplConsoleFormatOut("Logical Device\n");
	fplConsoleFormatOut("*************************************************************************\n");
	{
		VkPhysicalDeviceFeatures enabledFeatures = fplZeroInit;
		bool isSwapChain = true;
		const char **reqExtensions = fpl_null;
		uint32_t requiredExtensionCount = 0;
		void *pNextChain = fpl_null;
		if(!VulkanCreateLogicalDevice(
			allocator,
			coreApi,
			instanceApi,
			&state->physicalDevice,
			&enabledFeatures,
			&state->logicalDevice,
			state->instance.instanceHandle,
			reqExtensions,
			requiredExtensionCount,
			isSwapChain,
			pNextChain)) {
			fplConsoleFormatError("Failed to create a logical device from physical device '%s'!\n", state->physicalDevice.name);
			goto failed;
		}
	}

	//
	// Surface Properties
	//
	fplConsoleFormatOut("*************************************************************************\n");
	fplConsoleFormatOut("Surface Step 2/2\n");
	fplConsoleFormatOut("*************************************************************************\n");
	if(!QueryVulkanSurfaceProperties(instanceApi, &state->physicalDevice, &state->logicalDevice, &state->surface, state->instance.instanceHandle)) {
		fplConsoleFormatError("Failed to query surface properties for instance '%p', physical device '%s' and surface '%p'!\n", state->instance.instanceHandle, state->physicalDevice.name, state->surface.surfaceHandle);
		goto failed;
	}

	//
	// Frame (Semaphores, Swap-Chain, Command-Buffer)
	//
	fplConsoleFormatOut("*************************************************************************\n");
	fplConsoleFormatOut("Frame\n");
	fplConsoleFormatOut("*************************************************************************\n");
	bool vsync = true;
	VkExtent2D size = fplStructInit(VkExtent2D, winWidth, winHeight);
	if(!VulkanCreateFrame(allocator, &state->instance, &state->physicalDevice, &state->logicalDevice, &state->surface, &state->frame, size, vsync)) {
		fplConsoleFormatError("Failed to create a frame for device '%p' and surface '%p' with size of %lu x %lu!\n", state->logicalDevice.logicalDeviceHandle, state->surface.surfaceHandle, size.width, size.height);
		goto failed;
	}

	goto success;

failed:
	VulkanShutdownAll(state);
	return(false);

success:
	state->isInitialized = true;
	return(true);
}

// Swap-Chain images are not compatible with the window surface anymore (Resized)
static bool InvalidateFrame(VulkanState *state, const VkExtent2D size) {
	fplAssert(state != fpl_null);

	VulkanInstance *instance = &state->instance;
	VulkanPhysicalDevice *physicalDevice = &state->physicalDevice;
	VulkanLogicalDevice *logicalDevice = &state->logicalDevice;
	VulkanFrame *frame = &state->frame;
	VulkanSwapChain *swapChain = &frame->swapChain;
	VulkanSurface *surface = &state->surface;

	if(logicalDevice->logicalDeviceHandle == VK_NULL_HANDLE)
		return(false);
	if(frame->swapChain.swapChainHandle == VK_NULL_HANDLE)
		return(false);

	const VulkanDeviceApi *deviceApi = &logicalDevice->deviceApi;

	// Clear swap chain
	VkSwapchainKHR oldSwapChain = swapChain->swapChainHandle;
	VulkanClearSwapChain(state->allocator, logicalDevice, swapChain);

	// Re-create swap chain (Old will be removed)
	bool isVsync = swapChain->isVSync;
	if(!VulkanCreateSwapChain(state->allocator, instance, physicalDevice, logicalDevice, surface, swapChain, oldSwapChain, size, isVsync)) {
		return(false);
	}

	// Re-build command buffer
	VulkanTemporaryRecordBuffer(logicalDevice, frame);

	return(true);
}

static bool Draw(VulkanState *state, const VkExtent2D size) {
	if(!state->isInitialized) return(false);

	VkResult res;

	VkDevice device = state->logicalDevice.logicalDeviceHandle;
	VkSwapchainKHR swapChain = state->frame.swapChain.swapChainHandle;

	const VulkanDeviceApi *deviceApi = &state->logicalDevice.deviceApi;

	// Aquire next image, we it fails call InvalidateFrame() and exit out
	VkFence fence = VK_NULL_HANDLE;
	uint32_t imageIndex;
	res = deviceApi->vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, state->frame.imageAvailableSemaphoreHandle, fence, &imageIndex);
	switch(res) {
		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			return InvalidateFrame(state, size);
		default:
			return false;
	}

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submitInfo = fplZeroInit;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &state->frame.imageAvailableSemaphoreHandle;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &state->frame.swapChain.presentationCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &state->frame.renderCompleteSemaphoreHandle;

	res = deviceApi->vkQueueSubmit(state->surface.presentationQueueHandle, 1, &submitInfo, fence);
	if(res != VK_SUCCESS) {
		return(false);
	}

	VkPresentInfoKHR presentInfo = fplZeroInit;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &state->frame.renderCompleteSemaphoreHandle;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &state->frame.swapChain.swapChainHandle;
	presentInfo.pImageIndices = &imageIndex;
	res = deviceApi->vkQueuePresentKHR(state->surface.presentationQueueHandle, &presentInfo);
	switch(res) {
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			return InvalidateFrame(state, size);
		default:
			return false;
	}

	return(true);
}

int main(int argc, char **argv) {
	int appResult = -1;

	bool isPlatformInitialized = false;

	VulkanState *state = fpl_null;

	fplPlatformType platformType = fplGetPlatformType();
	const char *platformName = fplGetPlatformName(platformType);
	fplConsoleFormatOut("-> Initialize %s Platform\n", platformName);

	size_t stateSize = sizeof(VulkanState);
	state = (VulkanState *)fplMemoryAllocate(stateSize);
	if(state == fpl_null) {
		fplConsoleFormatError("Failed to allocate memory of size '%zu' for Vulkan state!", stateSize);
		goto cleanup;
	}

	//
	// Get Vulkan Requirements for FPL
	//
	fplVideoRequirements videoRequirements = fplZeroInit;
	bool createInstance;
	const char **requiredExtensions = fpl_null;
	uint32_t requiredExtensionCount = 0;
#if VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_SURFACE_ONLY
	if(fplGetVideoRequirements(fplVideoBackendType_Vulkan, &videoRequirements)) {
		fplConsoleFormatOut("%lu required instance extensions:\n", videoRequirements.vulkan.instanceExtensionCount);
		for(uint32_t i = 0; i < videoRequirements.vulkan.instanceExtensionCount; ++i) {
			fplConsoleFormatOut("- [%lu] %s\n", i, videoRequirements.vulkan.instanceExtensions[i]);
		}
	}
	createInstance = true;
	requiredExtensions = videoRequirements.vulkan.instanceExtensions;
	requiredExtensionCount = videoRequirements.vulkan.instanceExtensionCount;
	fplAssert(requiredExtensionCount > 0);
#elif VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_FULL
	createInstance = false;
#else
	createInstance = true;
#endif

	//
	// Initialize Vulkan (Step 1/2) -> API only
	//
	fplConsoleFormatOut("-> Initialize Vulkan Step (1/2)\n");
	fplConsoleFormatOut("\n");
	if(!VulkanInitializeStepInit(state, createInstance, requiredExtensions, requiredExtensionCount)) {
		fplConsoleFormatError("Failed to initialize Vulkan (Step 1/2)!\n");
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized Vulkan (Step 1/2)\n");
	fplConsoleOut("\n");

	fplLogSettings logSettings = fplZeroInit;
	logSettings.maxLevel = fplLogLevel_All;
	logSettings.writers[0].flags = fplLogWriterFlags_StandardConsole;
	fplSetLogSettings(&logSettings);

	fplSettings settings = fplMakeDefaultSettings();
	fplCopyString("FPL Demo | Vulkan", settings.window.title, fplArrayCount(settings.window.title));
	fplCopyString("FPL Demo | Vulkan", settings.console.title, fplArrayCount(settings.console.title));

	fplInitFlags initFlags = fplInitFlags_Window | fplInitFlags_GameController | fplInitFlags_Console;

#if VULKANDEMO_FPL_VIDEO_MODE != VULKANDEMO_FPL_VIDEO_MODE_NONE
	initFlags |= fplInitFlags_Video;
	settings.video.backend = fplVideoBackendType_Vulkan;

#if VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_FULL
	// We want FPL to create the instance and the surface for us
	fplVersionInfo *apiVer = &settings.video.graphics.vulkan.apiVersion;
	settings.video.graphics.vulkan.apiVersion.major[0] = '1';
	settings.video.graphics.vulkan.apiVersion.minor[0] = '1';
	settings.video.graphics.vulkan.engineVersion.major[0] = '1';
	settings.video.graphics.vulkan.engineVersion.minor[0] = '0';
	settings.video.graphics.vulkan.appVersion.major[0] = '1';
	settings.video.graphics.vulkan.appVersion.minor[0] = '0';
	settings.video.graphics.vulkan.appName = "FPL-Vulkan-Demo";
	settings.video.graphics.vulkan.engineName = "FPL-Vulkan-Demo";
	settings.video.graphics.vulkan.validationLayerMode = fplVulkanValidationLayerMode_Logging;
	settings.video.graphics.vulkan.validationSeverity = fplVulkanValidationSeverity_All;
#elif VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_SURFACE_ONLY
	// We want FPL only to create the surface for us
	settings.video.graphics.vulkan.instanceHandle = state->instance.instanceHandle;
	state->instance.isUserDefined = true;
#endif

#else
	settings.video.backend = fplVideoBackendType_None;
#endif // VULKANDEMO_FPL_VIDEO_MODE != VULKANDEMO_FPL_VIDEO_MODE_NONE

	fplConsoleFormatOut("-> Initialize %s Platform\n", platformName);
	if(!fplPlatformInit(initFlags, &settings)) {
		fplPlatformResultType resultType = fplGetPlatformResult();
		const char *resultName = fplGetPlatformResultName(resultType);
		fplConsoleFormatError("Failed to initialize FPL '%s'!\n", resultName);
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized %s Platform\n", platformName);
	fplConsoleOut("\n");

	isPlatformInitialized = true;

#if VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_SURFACE_ONLY || VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_FULL
	const fplVideoSurface *videoSurface = fplGetVideoSurface();
	fplAssert(videoSurface != fpl_null);

#if VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_SURFACE_ONLY
	state->surface.surfaceHandle = (VkSurfaceKHR)videoSurface->vulkan.surfaceKHR;
	state->surface.isUserDefined = true;
#elif VULKANDEMO_FPL_VIDEO_MODE == VULKANDEMO_FPL_VIDEO_MODE_FULL
	state->instance.instanceHandle = (VkInstance)videoSurface->vulkan.instance;
	state->surface.surfaceHandle = (VkSurfaceKHR)videoSurface->vulkan.surfaceKHR;
	state->surface.isUserDefined = true;
	state->instance.isUserDefined = true;
#endif

	fplAssert(state->instance.instanceHandle != VK_NULL_HANDLE);
	fplAssert(state->surface.surfaceHandle != VK_NULL_HANDLE);
#endif

	fplWindowSize initialWinSize = fplZeroInit;
	fplGetWindowSize(&initialWinSize);

	fplConsoleFormatOut("-> Initialize Vulkan (Step 2/2)\n");
	fplConsoleFormatOut("\n");
	if(!VulkanInitializeStepRest(state, initialWinSize.width, initialWinSize.height)) {
		fplConsoleFormatError("Failed to initialize Vulkan (Step 2/2)!\n");
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized Vulkan (Step 2/2)\n");
	fplConsoleOut("\n");

	appResult = 0;

	fplConsoleFormatOut("-> Run main loop\n");
	fplConsoleFormatOut("\n");

	VkExtent2D drawSize = fplStructInit(VkExtent2D, initialWinSize.width, initialWinSize.height);
	while(fplWindowUpdate()) {
		fplEvent ev;
		while(fplPollEvent(&ev)) {
			if(ev.type == fplEventType_Window) {
				if(ev.window.type == fplWindowEventType_Resized) {
					drawSize.width = ev.window.size.width;
					drawSize.height = ev.window.size.height;
					InvalidateFrame(state, drawSize);
				}
			}
		}

		Draw(state, drawSize);

#if VULKANDEMO_USE_FPL_VIDEO
		fplVideoFlip();
#endif
	}

cleanup:
	fplConsoleFormatOut("\n");

	if(isPlatformInitialized) {
		if(state != fpl_null) {
			// Shutdown Vulkan (Destroy swap-chain, logical/physical devices, buffers and surface)
			fplConsoleFormatOut("Shutdown Vulkan (Step 1/2)\n");
			VulkanShutdownStepRest(state);
		}

		// Release platform
		fplConsoleFormatOut("Shutdown %s Platform\n", platformName);
		fplPlatformRelease();
	}

	if(state != fpl_null) {
		// Shutdown Vulkan (Destroy instance and unload library)
		fplConsoleFormatOut("Shutdown Vulkan (Step 2/2)\n");
		VulkanShutdownStepInit(state);
		fplMemoryFree(state);
	}

#if 0
	fplConsoleFormatOut("Press any key to exit\n");
	fplConsoleWaitForCharInput();
#endif

	return(appResult);
}