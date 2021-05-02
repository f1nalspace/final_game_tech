#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#if defined(FPL_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <malloc.h>

#include "containers.h"

//
// Vulkan Utils
//
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
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
			return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT";
		case VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT:
			return "VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT";
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT:
			return "VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT";
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
		case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
			return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
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
		case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
			return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD";
		default:
			return "Unknown";
	}
}

static void VulkanVersionToString(const uint32_t versionNumber, const size_t outNameCapacity, char *outName) {
	int32_t major = VK_VERSION_MAJOR(versionNumber);
	int32_t minor = VK_VERSION_MINOR(versionNumber);
	int32_t patch = VK_VERSION_PATCH(versionNumber);

	size_t lenMajor = fplS32ToString(major, fpl_null, 0);
	size_t lenMinor = fplS32ToString(minor, fpl_null, 0);
	size_t lenPatch = fplS32ToString(patch, fpl_null, 0);

	size_t requiredLen = (lenMajor + 1 + lenMinor + 1 + lenPatch) + 3;
	assert(outNameCapacity >= requiredLen);

	fplS32ToString(major, outName + 0, lenMajor + 1);
	outName[lenMajor] = '.';
	fplS32ToString(minor, outName + lenMajor + 1, lenMinor + 1);
	outName[lenMajor + 1 + lenMinor] = '.';
	fplS32ToString(patch, outName + lenMajor + 1 + lenMinor + 1, lenPatch + 1);
}

static uint32_t GetVulkanQueueFamilyIndex(const VkQueueFlagBits flags, const VkQueueFamilyProperties *families, const uint32_t familyCount) {
	// Find a dedicated queue for compute (Not graphics)
	if((flags & VK_QUEUE_COMPUTE_BIT) > 0) {
		for(uint32_t i = 0; i < familyCount; ++i) {
			VkQueueFlagBits familyFlags = families[i].queueFlags;
			if(((familyFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) && ((familyFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
				return(i);
			}
		}
	}

	// Find a dedicated queue for transfer (Not graphics and not compute)
	if((flags & VK_QUEUE_TRANSFER_BIT) > 0) {
		for(uint32_t i = 0; i < familyCount; ++i) {
			VkQueueFlagBits familyFlags = families[i].queueFlags;
			if(((familyFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) && ((familyFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((familyFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
				return(i);
			}
		}
	}

	// For all other queues
	for(uint32_t i = 0; i < familyCount; ++i) {
		if((families[i].queueFlags & flags) > 0) {
			return(i);
		}
	}

	return(UINT32_MAX);
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
	assert(coreApi != fpl_null);
	if(coreApi->isValid) {
		fplConsoleFormatOut("Unload Vulkan API\n");
		fplDynamicLibraryUnload(&coreApi->libHandle);
	}
	fplClearStruct(coreApi);
}

bool VulkanLoadCoreAPI(VulkanCoreApi *coreApi) {
	assert(coreApi != fpl_null);

#if defined(FPL_PLATFORM_WINDOWS)
	const char *vulkanLibraryFileName = "vulkan-1.dll";
#elif defined(FPL_SUBPLATFORM_POSIX)
	const char *vulkanLibraryFileName = "libvulkan.so";
#else
#	error "Unsupported Platform!"
#endif

	fplClearStruct(coreApi);

	fplConsoleFormatOut("Load Vulkan API '%s'\n", vulkanLibraryFileName);
	if(!fplDynamicLibraryLoad(vulkanLibraryFileName, &coreApi->libHandle)) {
		return(false);
	}

	fplDynamicLibraryHandle *handle = &coreApi->libHandle;

#define VULKAN_LIBRARY_GET_PROC_ADDRESS(libHandle, libName, target, type, name) \
	(target)->name = (type)fplGetDynamicLibraryProc(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		break; \
	}

	bool success = false;
	do {

		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkCreateInstance, vkCreateInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkDestroyInstance, vkDestroyInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, coreApi, PFN_vkGetInstanceProcAddr, vkGetInstanceProcAddr);

		success = true;
	} while(0);

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
	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkCreateCommandPool vkCreateCommandPool;
	PFN_vkDestroyCommandPool vkDestroyCommandPool;
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

#if defined(FPL_PLATFORM_WINDOWS)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif

	fpl_b32 isValid;
} VulkanInstanceApi;

void UnloadVulkanInstanceAPI(VulkanInstanceApi *instanceApi) {
	assert(instanceApi != fpl_null);
	fplClearStruct(instanceApi);
}

bool LoadVulkanInstanceAPI(const VulkanCoreApi *coreApi, VkInstance instanceHandle, VulkanInstanceApi *instanceApi) {
	assert(coreApi != fpl_null && instanceApi != fpl_null);
	if(instanceHandle == VK_NULL_HANDLE)
		return(false);
	if(!coreApi->isValid)
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
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkDestroyDevice, vkDestroyDevice);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkCreateCommandPool, vkCreateCommandPool);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkDestroyCommandPool, vkDestroyCommandPool);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkDestroySurfaceKHR, vkDestroySurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceSupportKHR, vkGetPhysicalDeviceSurfaceSupportKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceFormatsKHR, vkGetPhysicalDeviceSurfaceFormatsKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceSurfacePresentModesKHR, vkGetPhysicalDeviceSurfacePresentModesKHR);

#if defined(FPL_PLATFORM_WINDOWS)
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkCreateWin32SurfaceKHR, vkCreateWin32SurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(instanceApi, PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR, vkGetPhysicalDeviceWin32PresentationSupportKHR);
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
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR vkQueuePresentKHR;

	fpl_b32 isValid;
} VulkanDeviceApi;

void VulkanUnloadDeviceApi(VulkanDeviceApi *deviceApi) {
	assert(deviceApi != fpl_null);
	fplClearStruct(deviceApi);
}

bool VulkanLoadDeviceApi(const VulkanInstanceApi *instanceApi, VkDevice deviceHandle, VulkanDeviceApi *deviceApi) {
	assert(instanceApi != fpl_null && deviceApi != fpl_null);

	fplClearStruct(deviceApi);

#define VULKAN_DEVICE_GET_PROC_ADDRESS(target, type, name) \
	(target)->name = (type)instanceApi->vkGetDeviceProcAddr(deviceHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting device procedure address '%s'", #name); \
		break; \
	}

	bool success = false;
	do {
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkCreateSwapchainKHR, vkCreateSwapchainKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkDestroySwapchainKHR, vkDestroySwapchainKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkGetSwapchainImagesKHR, vkGetSwapchainImagesKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkAcquireNextImageKHR, vkAcquireNextImageKHR);
		VULKAN_DEVICE_GET_PROC_ADDRESS(deviceApi, PFN_vkQueuePresentKHR, vkQueuePresentKHR);

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
	if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		fplConsoleFormatError("[%s] Validation layer: %s\n", severityName, pCallbackData->pMessage);
	}
	return VK_FALSE;
}

static void VulkanDestroyDebugMessenger(const VulkanCoreApi *coreApi, VkInstance instanceHandle, VkDebugUtilsMessengerEXT debugMessenger) {
	assert(coreApi != fpl_null);
	if(instanceHandle == VK_NULL_HANDLE) return;
	if(debugMessenger != VK_NULL_HANDLE) {
		PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)coreApi->vkGetInstanceProcAddr(instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
		if(func != fpl_null) {
			func(instanceHandle, debugMessenger, fpl_null);
		}
	}
}

static VkDebugUtilsMessengerCreateInfoEXT MakeVulkanDebugMessengerCreateInfo() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = VulkanDebugCallback;
	createInfo.pUserData = fpl_null;
	return(createInfo);
}

static bool VulkanCreateDebugMessenger(const VulkanCoreApi *coreApi, const VkInstance instanceHandle, const VkDebugUtilsMessengerCreateInfoEXT *createInfo, VkDebugUtilsMessengerEXT *outDebugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)coreApi->vkGetInstanceProcAddr(instanceHandle, "vkCreateDebugUtilsMessengerEXT");
	if(func != fpl_null) {
		VkResult res = func(instanceHandle, createInfo, fpl_null, outDebugMessenger);
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
} VulkanInstance;

static void VulkanDestroyInstance(const VulkanCoreApi *coreApi, VulkanInstance *instance) {
	if(coreApi == fpl_null || instance == fpl_null) return;

	// Unload Instance API
	UnloadVulkanInstanceAPI(&instance->instanceApi);

	// Destroy Vulkan instance
	if(instance->instanceHandle != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan instance\n");
		coreApi->vkDestroyInstance(instance->instanceHandle, fpl_null);
	}

	// Destroy instance properties
	DestroyVulkanInstanceProperties(&instance->properties);

	fplClearStruct(instance);
}

static bool VulkanCreateInstance(const VulkanCoreApi *coreApi, const bool useValidation, VulkanInstance *instance) {
	const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
	const char *khrSurfaceName = "VK_KHR_surface";

	const char *khrPlatformSurfaceName = fpl_null;
#if defined(FPL_PLATFORM_WINDOWS)
	khrPlatformSurfaceName = "VK_KHR_win32_surface";
#elif defined(FPL_SUBPLATFORM_X11)
	khrPlatformSurfaceName = "VK_KHR_xlib_surface";
#endif

	fplClearStruct(instance);

	VkResult res;

	//
	// Load properties
	//
	VulkanInstanceProperties *instanceProperties = &instance->properties;
	if(!LoadVulkanInstanceProperties(coreApi, instanceProperties)) {
		fplConsoleFormatError("Failed loading instance properties!\n");
		return(false);
	}

	//
	// Check and validate extensions and layers
	//
	bool supportsKHRSurface = false;
	bool supportsKHRPlatformSurface = false;
	for(uint32_t extensionIndex = 0; extensionIndex < instanceProperties->supportedExtensions.count; ++extensionIndex) {
		const char *extensionName = instanceProperties->supportedExtensions.items[extensionIndex];
		if(fplIsStringEqual(khrSurfaceName, extensionName)) {
			supportsKHRSurface = true;
		}
		if(fplIsStringEqual(khrPlatformSurfaceName, extensionName)) {
			supportsKHRPlatformSurface = true;
		}
	}

	bool supportsValidationLayer = false;
	for(uint32_t layerIndex = 0; layerIndex < instanceProperties->supportedLayers.count; ++layerIndex) {
		const char *layerName = instanceProperties->supportedLayers.items[layerIndex];
		if(fplIsStringEqual(layerName, validationLayerName)) {
			supportsValidationLayer = true;
		}
	}

	fplConsoleFormatOut("\n");

	//
	// Check Extensions
	//
	fplConsoleFormatOut("Validate instance extensions:\n");
	fplConsoleFormatOut("- Supported %s: %s\n", khrSurfaceName, (supportsKHRSurface ? "yes" : "no"));
	fplConsoleFormatOut("- Supported %s: %s\n", khrPlatformSurfaceName, (supportsKHRPlatformSurface ? "yes" : "no"));

	fplConsoleFormatOut("Validate instance layers:\n");
	fplConsoleFormatOut("- Supported %s: %s\n", validationLayerName, (supportsValidationLayer ? "yes" : "no"));

	if(!supportsKHRSurface || !supportsKHRPlatformSurface || khrPlatformSurfaceName == fpl_null) {
		fplConsoleFormatError("Not supported KHR platform!\n");
		VulkanDestroyInstance(coreApi, instance);
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
	appInfo->apiVersion = VK_API_VERSION_1_2;

	uint32_t enabledInstanceExtensionCount = 0;
	const char *enabledInstanceExtensions[8] = { 0 };
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = khrSurfaceName; // This is always supported
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = khrPlatformSurfaceName;
	if(useValidation) {
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME; // VK_EXT_debug_utils is always supported
	}

	uint32_t enabledInstanceLayerCount = 0;
	const char *enabledInstanceLayers[8] = { 0 };
	if(useValidation && supportsValidationLayer) {
		instance->hasValidationLayer = true;
		enabledInstanceLayers[enabledInstanceLayerCount++] = validationLayerName;
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = fplZeroInit;

	VkInstanceCreateInfo instanceCreateInfo = fplZeroInit;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = appInfo;
	instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	instanceCreateInfo.enabledLayerCount = enabledInstanceLayerCount;
	instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
	instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayers;
	if(useValidation && supportsValidationLayer) {
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
	res = coreApi->vkCreateInstance(&instanceCreateInfo, fpl_null, &instance->instanceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating Vulkan instance for application '%s'!\n", appInfo->pApplicationName);
		VulkanDestroyInstance(coreApi, instance);
		return(false);
	}
	fplConsoleFormatOut("Successfully created instance\n");
	fplConsoleFormatOut("\n");

	//
	// Load instance API
	//
	if(!LoadVulkanInstanceAPI(coreApi, instance->instanceHandle, &instance->instanceApi)) {
		fplConsoleFormatError("Failed to load the Vulkan instance API for instance '%p'!\n", instance->instanceHandle);
		VulkanDestroyInstance(coreApi, instance);
		return(false);
	}

	return(true);
}

bool IsVulkanExtensionSupported(const char **supportedExtensions, const uint32_t supportedExtensionCount, const char *extension) {
	for(uint32_t i = 0; i < supportedExtensionCount; ++i) {
		const char *supportedExt = supportedExtensions[i];
		if(fplIsStringEqual(supportedExt, extension)) {
			return(true);
		}
	}
	return(false);
}

typedef struct VulkanPhysicalDevice {
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	FIXED_TYPED_ARRAY(VkQueueFamilyProperties, queueFamilies);
	StringTable supportedExtensions;

	VkPhysicalDevice physicalDeviceHandle;
	const char *name;
} VulkanPhysicalDevice;

void VulkanDestroyPhysicalDevice(const VulkanCoreApi *coreApi, VulkanPhysicalDevice *physicalDevice) {
	if(coreApi == fpl_null || physicalDevice == fpl_null) return;
	FreeStringTable(&physicalDevice->supportedExtensions);
	FREE_FIXED_TYPED_ARRAY(&physicalDevice->queueFamilies);
	fplClearStruct(physicalDevice);
}

bool VulkanCreatePhysicalDevice(const VulkanCoreApi *coreApi, const VulkanInstanceApi *instanceApi, VkInstance instance, VulkanPhysicalDevice *physicalDevice) {
	VkResult res;

	//
	// Get Physical Devices
	//
	fplConsoleFormatOut("Enumerate physical devices for instance '%p'\n", instance);
	uint32_t physicalDeviceCount = 0;
	res = instanceApi->vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instance);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	if(physicalDevices == fpl_null) {
		fplConsoleFormatError("Failed allocating memory for %lu physical devices!\n", physicalDeviceCount);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	res = instanceApi->vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instance);
		VulkanDestroyPhysicalDevice(coreApi, physicalDevice);
		free(physicalDevices);
	}
	fplConsoleFormatOut("Successfully enumerated physical devices, got %lu physics devices\n", physicalDeviceCount);
	fplConsoleFormatOut("\n");

	//
	// Find physical device (Discrete GPU is preferred over integrated GPU)
	//
	VkPhysicalDevice foundGpu = VK_NULL_HANDLE;
	uint32_t gpuMatchDistance = 0;
	uint32_t foundGPUIndex = 0;
	for(uint32_t physicalDeviceIndex = 0; physicalDeviceIndex < physicalDeviceCount; ++physicalDeviceIndex) {
		VkPhysicalDevice physicalDevice = physicalDevices[physicalDeviceIndex];

		VkPhysicalDeviceProperties props;
		instanceApi->vkGetPhysicalDeviceProperties(physicalDevice, &props);

		uint32_t matchDistance = 0;
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			matchDistance = 100; // Prefer discrete GPU over integrated
		} else if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			matchDistance = 10;
		}

		if(matchDistance > gpuMatchDistance) {
			gpuMatchDistance = matchDistance;
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
	// Queue Families
	//

	// TODO(final): Make a function for getting the queue family properties

	fplConsoleFormatOut("Get queue family properties for Physical Device '%s'\n", physicalDevice->name);
	uint32_t queueFamilyPropertiesCount = 0;
	instanceApi->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDeviceHandle, &queueFamilyPropertiesCount, fpl_null);
	assert(queueFamilyPropertiesCount > 0);

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

	//
	// Device Extensions
	//
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

	return(true);
}

typedef struct VulkanLogicalDevice {
	VkPhysicalDeviceFeatures enabledFeatures;
	VulkanDeviceApi deviceApi;
	VkDevice logicalDeviceHandle;
	VkCommandPool graphicsCommandPoolHandle;
	uint32_t computeFamilyIndex;
	uint32_t transferFamilyIndex;
	uint32_t graphicsFamilyIndex;
} VulkanLogicalDevice;

void VulkanDestroyLogicalDevice(const VulkanInstanceApi *instanceApi, VulkanLogicalDevice *logicalDevice) {
	assert(instanceApi != fpl_null && logicalDevice != fpl_null);

	VulkanUnloadDeviceApi(&logicalDevice->deviceApi);

	if(logicalDevice->graphicsCommandPoolHandle != fpl_null) {
		instanceApi->vkDestroyCommandPool(logicalDevice->logicalDeviceHandle, logicalDevice->graphicsCommandPoolHandle, fpl_null);
	}

	if(logicalDevice->logicalDeviceHandle != fpl_null) {
		instanceApi->vkDestroyDevice(logicalDevice->logicalDeviceHandle, fpl_null);
	}

	fplClearStruct(logicalDevice);
	logicalDevice->computeFamilyIndex = UINT32_MAX;
	logicalDevice->graphicsFamilyIndex = UINT32_MAX;
	logicalDevice->transferFamilyIndex = UINT32_MAX;
}

bool VulkanCreateCommandPool(
	const VulkanInstanceApi *instanceApi,
	const VkDevice logicalDevice,
	const int32_t queueFamilyIndex,
	const VkCommandPoolCreateFlags createFlags,
	VkCommandPool *outCommandPool) {

	VkCommandPoolCreateInfo cmdPoolInfo = fplZeroInit;
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
	cmdPoolInfo.flags = createFlags;

	VkCommandPool pool;
	VkResult res = instanceApi->vkCreateCommandPool(logicalDevice, &cmdPoolInfo, fpl_null, &pool);
	if(res != VK_SUCCESS) {
		return(false);
	}
	*outCommandPool = pool;
	return(true);
}

bool VulkanCreateLogicalDevice(
	const VulkanCoreApi *coreApi,
	const VulkanInstanceApi *instanceApi,
	const VulkanPhysicalDevice *physicalDevice,
	const VkPhysicalDeviceFeatures *enabledFeatures,
	const VkInstance instanceHandle,
	const char **reqExtensions,
	const uint32_t reqExtensionCount,
	const bool useSwapChain,
	void *pNextChain,
	VulkanLogicalDevice *logicalDevice) {
	if(logicalDevice == fpl_null || physicalDevice == fpl_null)
		return(false);

	uint32_t queueCreationInfoCount = 0;
	VkDeviceQueueCreateInfo queueCreationInfos[4] = fplZeroInit;

	const float defaultQueuePriority = 0.0f;

	fplClearStruct(logicalDevice);
	logicalDevice->computeFamilyIndex = UINT32_MAX;
	logicalDevice->graphicsFamilyIndex = UINT32_MAX;
	logicalDevice->transferFamilyIndex = UINT32_MAX;

	//
	// Graphics queue
	//
	fplConsoleFormatOut("Detect queue families...\n");
	logicalDevice->graphicsFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	logicalDevice->computeFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	logicalDevice->transferFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, physicalDevice->queueFamilies.items, physicalDevice->queueFamilies.itemCount);
	if(logicalDevice->graphicsFamilyIndex == UINT32_MAX) {
		fplConsoleFormatError("No graphics queue family for physical device '%s' found!\n", physicalDevice->name);
		VulkanDestroyLogicalDevice(instanceApi, logicalDevice);
		return(false);
	}
	if(logicalDevice->computeFamilyIndex == UINT32_MAX) {
		// Use graphics queue for compute queue
		logicalDevice->computeFamilyIndex = logicalDevice->graphicsFamilyIndex;
	}
	if(logicalDevice->transferFamilyIndex == UINT32_MAX) {
		// Use graphics queue for transfer queue
		logicalDevice->transferFamilyIndex = logicalDevice->graphicsFamilyIndex;
	}
	assert(logicalDevice->graphicsFamilyIndex != UINT32_MAX && logicalDevice->computeFamilyIndex != UINT32_MAX && logicalDevice->transferFamilyIndex != UINT32_MAX);
	fplConsoleFormatOut("Successfully detected required queue families:\n");
	fplConsoleFormatOut("\tGraphics queue family: %d\n", logicalDevice->graphicsFamilyIndex);
	fplConsoleFormatOut("\tCompute queue family: %d\n", logicalDevice->computeFamilyIndex);
	fplConsoleFormatOut("\tTransfer queue family: %d\n", logicalDevice->transferFamilyIndex);
	fplConsoleOut("\n");

	// Add graphics queue family
	{
		assert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *queueCreateInfo = &queueCreationInfos[queueCreationInfoCount++];
		queueCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo->queueFamilyIndex = logicalDevice->graphicsFamilyIndex;
		queueCreateInfo->queueCount = 1;
		queueCreateInfo->pQueuePriorities = &defaultQueuePriority;
	}

	//
	// Add dedicated compute queue
	//
	if(logicalDevice->computeFamilyIndex != logicalDevice->graphicsFamilyIndex) {
		assert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *createInfo = &queueCreationInfos[queueCreationInfoCount++];
		createInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo->queueFamilyIndex = logicalDevice->computeFamilyIndex;
		createInfo->queueCount = 1;
		createInfo->pQueuePriorities = &defaultQueuePriority;
	}

	//
	// Add dedicated transfer queue
	//
	if(logicalDevice->transferFamilyIndex != logicalDevice->graphicsFamilyIndex) {
		assert(queueCreationInfoCount < fplArrayCount(queueCreationInfos));
		VkDeviceQueueCreateInfo *createInfo = &queueCreationInfos[queueCreationInfoCount++];
		createInfo->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo->queueFamilyIndex = logicalDevice->transferFamilyIndex;
		createInfo->queueCount = 1;
		createInfo->pQueuePriorities = &defaultQueuePriority;
	}

	// We don't allow more than 16 extensions for now
	uint32_t enabledDeviceExtensionCount = 0;
	const char *enabledDeviceExtensions[16] = fplZeroInit;
	const uint32_t maxEnableDeviceExtensionCount = fplArrayCount(enabledDeviceExtensions);
	for(uint32_t i = 0; i < reqExtensionCount; ++i) {
		assert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
		enabledDeviceExtensions[enabledDeviceExtensionCount++] = reqExtensions[i];
	}

	// Add SwapChain KHR extension when logical device will be used for a swap chain
	if(useSwapChain) {
		assert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
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
	if(IsVulkanExtensionSupported(physicalDevice->supportedExtensions.items, physicalDevice->supportedExtensions.count, VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
		assert(enabledDeviceExtensionCount < maxEnableDeviceExtensionCount);
		enabledDeviceExtensions[enabledDeviceExtensionCount++] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
	}

	if(enabledDeviceExtensionCount > 0) {
		fplConsoleFormatOut("Test %lu device extensions\n", enabledDeviceExtensionCount);
		for(uint32_t i = 0; i < enabledDeviceExtensionCount; ++i) {
			const char *enabledExt = enabledDeviceExtensions[i];
			if(IsVulkanExtensionSupported(physicalDevice->supportedExtensions.items, physicalDevice->supportedExtensions.count, enabledExt)) {
				fplConsoleFormatOut("[%lu] %s supported: yes\n", i, enabledExt);
			} else {
				fplConsoleFormatError("[%lu] %s supported: no (Warning!)\n", i, enabledExt);
			}
		}
		fplConsoleFormatOut("\n");
		deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
		deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions;
	}

	fplConsoleFormatOut("Creating Logical Device from physical device '%s'\n", physicalDevice->name);
	VkResult res = instanceApi->vkCreateDevice(physicalDevice->physicalDeviceHandle, &deviceCreateInfo, fpl_null, &logicalDevice->logicalDeviceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating the logical device from physical device '%s'!\n", physicalDevice->name);
		VulkanDestroyLogicalDevice(instanceApi, logicalDevice);
		return(false);
	}
	fplConsoleFormatOut("Successfully created logical device from physical device '%s'\n", physicalDevice->name);
	fplConsoleOut("\n");

	logicalDevice->enabledFeatures = *enabledFeatures;

	//
	// Command Pool
	// 
	fplConsoleFormatOut("Creating graphics command pool for logical device '%p' with queue %d...\n", logicalDevice->logicalDeviceHandle, logicalDevice->graphicsFamilyIndex);
	if(!VulkanCreateCommandPool(instanceApi, logicalDevice->logicalDeviceHandle, logicalDevice->graphicsFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, &logicalDevice->graphicsCommandPoolHandle)) {
		fplConsoleFormatError("Failed creating graphics command pool for logical device '%p' with queue %d!\n", logicalDevice->logicalDeviceHandle, logicalDevice->graphicsFamilyIndex);
		VulkanDestroyLogicalDevice(instanceApi, logicalDevice);
		return(false);
	}
	fplConsoleFormatOut("Successfully created graphics command pool for logical device '%p' with queue %d\n", logicalDevice->logicalDeviceHandle, logicalDevice->graphicsFamilyIndex);
	fplConsoleOut("\n");

	//
	// Load Device Api
	//
	fplConsoleFormatOut("Loading device API for device '%p'\n", logicalDevice->logicalDeviceHandle);
	if(!VulkanLoadDeviceApi(instanceApi, logicalDevice->logicalDeviceHandle, &logicalDevice->deviceApi)) {
		fplConsoleFormatError("Failed loading device API for device '%p'!\n", logicalDevice->logicalDeviceHandle);
		VulkanDestroyLogicalDevice(instanceApi, logicalDevice);
		return(false);
	}
	fplConsoleFormatOut("Successfully loaded device API for device '%p'\n", logicalDevice->logicalDeviceHandle);

	return(true);
}

typedef struct VulkanSurface {
	FIXED_TYPED_ARRAY(VkBool32, supportedQueuesForPresent);
	FIXED_TYPED_ARRAY(VkPresentModeKHR, presentationModes);
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceKHR surfaceHandle;
	VkFormat colorFormat;
	VkColorSpaceKHR colorSpace;
	uint32_t graphicsQueueFamilyIndex;
	uint32_t presentationQueueFamilyIndex;
} VulkanSurface;

void VulkanDestroySurface(const VulkanInstanceApi *instanceApi, const VkInstance instanceHandle, VulkanSurface *surface) {
	if(surface == fpl_null) return;
	FREE_FIXED_TYPED_ARRAY(&surface->presentationModes);
	FREE_FIXED_TYPED_ARRAY(&surface->supportedQueuesForPresent);
	if(surface->surfaceHandle != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan surface '%p'\n", surface->surfaceHandle);
		instanceApi->vkDestroySurfaceKHR(instanceHandle, surface->surfaceHandle, fpl_null);
	}

	fplClearStruct(surface);
	surface->graphicsQueueFamilyIndex = UINT32_MAX;
	surface->presentationQueueFamilyIndex = UINT32_MAX;
}

bool VulkanCreateSurface(const VulkanInstanceApi *instanceApi, const VkInstance instanceHandle, const VulkanPhysicalDevice *physicalDevice, VulkanSurface *surface) {
	if(instanceApi == fpl_null || instanceHandle == fpl_null || physicalDevice == fpl_null || surface == fpl_null)
		return(false);

	fplClearStruct(surface);
	surface->graphicsQueueFamilyIndex = UINT32_MAX;
	surface->presentationQueueFamilyIndex = UINT32_MAX;

	//
	// Create Surface KHR
	//
#if defined(FPL_PLATFORM_WINDOWS)
	// TODO(final): This is just temporary, until we can query the platform window informations from FPL
	HWND windowHandle = fpl__global__AppState->window.win32.windowHandle;
	HINSTANCE appHandle = GetModuleHandle(fpl_null);

	VkWin32SurfaceCreateInfoKHR createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = windowHandle;
	createInfo.hinstance = appHandle;

	fplConsoleFormatOut("Creating win32 surface KHR from window handle '%p'\n", createInfo.hwnd);
	VkResult res = instanceApi->vkCreateWin32SurfaceKHR(instanceHandle, &createInfo, fpl_null, &surface->surfaceHandle);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating win32 surface KHR!\n");
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	fplConsoleFormatOut("Successfully created win32 surface KHR -> '%p'\n", surface->surfaceHandle);
	fplConsoleFormatOut("\n");
#else
	fplConsoleFormatError("Unsupported Platform!\n");
	VulkanDestroySurface(instanceApi, instanceHandle, surface);
	return(false);
#endif

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
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;
	for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
		const VkQueueFamilyProperties *queueProps = physicalDevice->queueFamilies.items + queueFamilyIndex;
		if((queueProps[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if(graphicsQueueFamilyIndex == UINT32_MAX) {
				graphicsQueueFamilyIndex = queueFamilyIndex;
			}
			if(surface->supportedQueuesForPresent.items[queueFamilyIndex]) {
				graphicsQueueFamilyIndex = queueFamilyIndex;
				presentQueueFamilyIndex = queueFamilyIndex;
				break;
			}
		}
	}

	if(presentQueueFamilyIndex == UINT32_MAX) {
		// If there's no queue that supports both present and graphics, try to find a separate present queue
		for(uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; ++queueFamilyIndex) {
			if(surface->supportedQueuesForPresent.items[queueFamilyIndex]) {
				presentQueueFamilyIndex = queueFamilyIndex;
				break;
			}
		}
	}

	fplConsoleFormatOut("Graphics queue family: %lu\n", graphicsQueueFamilyIndex);
	fplConsoleFormatOut("Presentation queue family: %lu\n", presentQueueFamilyIndex);

	if(graphicsQueueFamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX) {
		fplConsoleFormatError("Could not find queue families for graphics or presentation!\n");
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	if(graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
		fplConsoleFormatError("Separate presentation queues are not supported!\n");
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}

	fplConsoleOut("\n");

	surface->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
	surface->presentationQueueFamilyIndex = presentQueueFamilyIndex;

	//
	// Find supported formats
	//
	fplConsoleFormatOut("Get surface formats for physical device '%s' and surface '%p'...\n", physicalDevice->name, surface->surfaceHandle);
	uint32_t formatCount = 0;
	res = instanceApi->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &formatCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface format count for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	assert(formatCount > 0);
	VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
	res = instanceApi->vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &formatCount, formats);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get %lu surface formats for physical device '%s' and surface '%p'!\n", formatCount, physicalDevice->name, surface->surfaceHandle);
		free(formats);
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}

	// Use first format initially (Worst case)
	surface->colorFormat = formats[0].format;
	surface->colorSpace = formats[0].colorSpace;

	bool found = false;
	for(uint32_t formatIndex = 0; formatIndex < formatCount; ++formatIndex) {
		const VkSurfaceFormatKHR *format = formats + formatIndex;
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

	free(formats);
	fplConsoleFormatOut("Successfully got %lu surface formats for physical device '%s' and surface '%p'\n", formatCount, physicalDevice->name, surface->surfaceHandle);
	fplConsoleOut("\n");

	//
	// Get Surface Capabilties
	//
	fplConsoleFormatOut("Get surface capabilities for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	res = instanceApi->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &surface->capabilities);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface capabilities for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	fplConsoleFormatOut("Successfully got surface capabilities for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	fplConsoleOut("\n");

	//
	// Get Presentation Modes
	//
	fplConsoleFormatOut("Get surface presentation modes for surface '%p' and physical device '%s'\n", surface->surfaceHandle, physicalDevice->name);
	uint32_t presentationModeCount = 0;
	res = instanceApi->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &presentationModeCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get surface presentation mode count for physical device '%s' and surface '%p'!\n", physicalDevice->name, surface->surfaceHandle);
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	ALLOC_FIXED_TYPED_ARRAY(&surface->presentationModes, VkPresentModeKHR, presentationModeCount);
	res = instanceApi->vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice->physicalDeviceHandle, surface->surfaceHandle, &presentationModeCount, surface->presentationModes.items);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed to get %lu surface presentation modes for physical device '%s' and surface '%p'!\n", presentationModeCount, physicalDevice->name, surface->surfaceHandle);
		VulkanDestroySurface(instanceApi, instanceHandle, surface);
		return(false);
	}
	for(uint32_t presentationModeIndex = 0; presentationModeIndex < presentationModeCount; ++presentationModeIndex) {
		VkPresentModeKHR presentMode = surface->presentationModes.items[presentationModeIndex];
		const char *presentationModeName = GetVulkanPresentModeKHRName(presentMode);
		fplConsoleFormatOut("[%lu] %s\n", presentationModeIndex, presentationModeName);
	}
	fplConsoleFormatOut("Successfully got %lu surface presentation modes for surface '%p' and physical device '%s'\n", presentationModeCount, surface->surfaceHandle, physicalDevice->name);
	fplConsoleOut("\n");

	return(true);
}

typedef struct VulkanSwapChain {
	int bla;
} VulkanSwapChain;

void VulkanDestroySwapChain(VulkanSwapChain *swapChain) {

}

bool VulkanCreateSwapChain(const VulkanDeviceApi *deviceApi, const VkDevice deviceHandle, const VulkanSurface *surface, VulkanSwapChain *swapChain) {
	// Get physical device surface properties and formats
	//deviceApi->
	//fpGetPhysicalDeviceSurfaceCapabilitiesKHR();
	//deviceApi->vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
	return(false);
}

typedef struct VulkanState {
	VulkanPhysicalDevice physicalDevice;

	VulkanLogicalDevice logicalDevice;

	VulkanCoreApi coreApi;

	VulkanInstance instance;

	VulkanSurface surface;

	VkDebugUtilsMessengerEXT debugMessenger;
	VkQueue graphicsQueueHandle;
	VkSemaphore presentCompleteSemaphoreHandle;
	VkSemaphore renderCompleteSemaphoreHandle;

	fpl_b32 isInitialized;
} VulkanState;

static void VulkanShutdown(VulkanState *state) {
	if(state == fpl_null) return;

	// Destroy Surface
	VulkanDestroySurface(&state->instance.instanceApi, state->instance.instanceHandle, &state->surface);

	// Destroy Logical Device
	VulkanDestroyLogicalDevice(&state->instance.instanceApi, &state->logicalDevice);

	// Destroy Physical device
	VulkanDestroyPhysicalDevice(&state->coreApi, &state->physicalDevice);

	// Destroy debug messenger
	if(state->instance.hasValidationLayer) {
		VulkanDestroyDebugMessenger(&state->coreApi, state->instance.instanceHandle, state->debugMessenger);
	}

	// Destroy Instance
	VulkanDestroyInstance(&state->coreApi, &state->instance);

	// Unload Core API
	VulkanUnloadCoreAPI(&state->coreApi);

	fplClearStruct(state);
}

static bool VulkanInitialize(VulkanState *state) {
	if(state == fpl_null)
		return(false);

	if(state->isInitialized) {
		fplConsoleError("Vulkan is already initialized!\n");
		return(false);
	}

	fplClearStruct(state);

	VulkanCoreApi *coreApi = &state->coreApi;
	VulkanInstanceApi *instanceApi = &state->instance.instanceApi;


	if(!VulkanLoadCoreAPI(coreApi)) {
		fplConsoleFormatError("Failed to load the Vulkan API!\n");
		goto failed;
	}
	fplConsoleFormatOut("\n");

	//
	// Create instance
	//
	if(!VulkanCreateInstance(coreApi, true, &state->instance)) {
		fplConsoleFormatError("Failed to create a Vulkan instance!\n");
		goto failed;
	}

	//
	// Debug messenger
	//
	if(state->instance.hasValidationLayer) {
		VkDebugUtilsMessengerCreateInfoEXT createInfo = MakeVulkanDebugMessengerCreateInfo();
		if(!VulkanCreateDebugMessenger(coreApi, state->instance.instanceHandle, &createInfo, &state->debugMessenger)) {
			fplConsoleFormatError("Failed to create the Vulkan debug messenger!\n");
		}
	}

	//
	// Physical Device (vkPhysicalDevice)
	//
	if(!VulkanCreatePhysicalDevice(coreApi, instanceApi, state->instance.instanceHandle, &state->physicalDevice)) {
		fplConsoleFormatError("Failed to find a physical device from instance '%p'!\n", state->instance.instanceHandle);
		goto failed;
	}

	//
	// Logical Device (vkDevice)
	//
	VkPhysicalDeviceFeatures enabledFeatures = fplZeroInit;
	if(!VulkanCreateLogicalDevice(coreApi, instanceApi, &state->physicalDevice, &enabledFeatures, state->instance.instanceHandle, fpl_null, 0, true, fpl_null, &state->logicalDevice)) {
		fplConsoleFormatError("Failed to create a logical device from physical device '%s'!\n", state->physicalDevice.name);
		goto failed;
	}

	//
	// Surface
	//
	if(!VulkanCreateSurface(instanceApi, state->instance.instanceHandle, &state->physicalDevice, &state->surface)) {
		fplConsoleFormatError("Failed to create surface for instance '%p' and physical device '%s'!\n", state->instance.instanceHandle, state->physicalDevice.name);
		goto failed;
	}

	// TODO(final): Find a suitable depth format

	// TODO(final): Swap-Chain!

	// TODO(final): Create synchronization objects

	goto success;

failed:
	VulkanShutdown(state);
	return(false);

success:
	return(true);
}

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_None;

	int appResult = -1;

	bool isPlatformInitialized = false;

	VulkanState *state = fpl_null;

	fplPlatformType platformType = fplGetPlatformType();
	const char *platformName = fplGetPlatformName(platformType);

	fplConsoleFormatOut("Initialize %s Platform\n", platformName);
	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_GameController | fplInitFlags_Console, &settings)) {
		fplPlatformResultType resultType = fplGetPlatformResult();
		const char *resultName = fplGetPlatformResultName(resultType);
		fplConsoleFormatError("Failed to initialize FPL '%s'!\n", resultName);
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized %s Platform\n", platformName);
	fplConsoleOut("\n");

	isPlatformInitialized = true;

	size_t stateSize = sizeof(VulkanState);
	state = (VulkanState *)fplMemoryAllocate(stateSize);
	if(state == fpl_null) {
		fplConsoleFormatError("Failed to allocate memory of size '%zu' for Vulkan state!", stateSize);
		goto cleanup;
	}

	fplConsoleFormatOut("Initialize Vulkan\n");
	if(!VulkanInitialize(state)) {
		fplConsoleFormatError("Failed to initialize Vulkan!\n");
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized Vulkan\n");
	fplConsoleOut("\n");

	appResult = 0;

	fplConsoleFormatOut("Run main loop\n");

	while(fplWindowUpdate()) {
		fplEvent ev;
		while(fplPollEvent(&ev)) {
			if(ev.type == fplEventType_Window) {
				if(ev.window.type == fplWindowEventType_Exposed) {

				}
			}
		}
		fplVideoFlip();
	}

cleanup:
	fplConsoleOut("\n");
	fplConsoleOut("Clean up\n");
	fplConsoleOut("\n");

	if(isPlatformInitialized) {
		if(state != fpl_null) {
			fplConsoleFormatOut("Shutdown Vulkan\n");
			VulkanShutdown(state);
			fplMemoryFree(state);
		}

#if 0
		fplConsoleFormatOut("Press any key to exit\n");
		fplConsoleWaitForCharInput();
#endif

		fplConsoleFormatOut("Shutdown Platform\n");
		fplPlatformRelease();
	}
	return(appResult);
}