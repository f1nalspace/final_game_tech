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
static const char *VulkanPhysicalDeviceTypeToStríng(const VkPhysicalDeviceType type) {
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

static int32_t GetVulkanQueueFamilyIndex(const VkQueueFlagBits flags, const VkQueueFamilyProperties *families, const uint32_t familyCount) {
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
			if(((familyFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) && ((familyFlags & VK_QUEUE_GRAPHICS_BIT) == 0)&& ((familyFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
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

	return(-1);
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

	fpl_b32 isInitialized;
} VulkanCoreApi;

void UnloadVulkanCoreAPI(VulkanCoreApi *api) {
	if(api->isInitialized) {
		fplConsoleFormatOut("Unload Vulkan API\n");
		fplDynamicLibraryUnload(&api->libHandle);
	}
	fplClearStruct(api);
}

bool LoadVulkanCoreAPI(VulkanCoreApi *api) {
	//const char *vulkanLibraryFileName = "libvulkan.so";
	const char *vulkanLibraryFileName = "vulkan-1.dll";

	fplConsoleFormatOut("Load Vulkan API '%s'\n", vulkanLibraryFileName);
	if(!fplDynamicLibraryLoad(vulkanLibraryFileName, &api->libHandle)) {
		return(false);
	}

	fplDynamicLibraryHandle *handle = &api->libHandle;

#define VULKAN_LIBRARY_GET_PROC_ADDRESS(libHandle, libName, target, type, name) \
	(target)->name = (type)fplGetDynamicLibraryProc(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		break; \
	}

	bool result = false;
	do {

		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, api, PFN_vkCreateInstance, vkCreateInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, api, PFN_vkDestroyInstance, vkDestroyInstance);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, api, PFN_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, api, PFN_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);
		VULKAN_LIBRARY_GET_PROC_ADDRESS(handle, vulkanLibraryFileName, api, PFN_vkGetInstanceProcAddr, vkGetInstanceProcAddr);

		result = true;
	} while(0);

#undef VULKAN_LIBRARY_GET_PROC_ADDRESS

	if(!result) {
		UnloadVulkanCoreAPI(api);
	}
	api->isInitialized = true;
	return(result);
}

typedef struct VulkanInstanceApi {
	VkInstance instance;

	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
	PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;


#if defined(FPL_PLATFORM_WINDOWS)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif

} VulkanInstanceApi;

bool LoadVulkanInstanceAPI(const VulkanCoreApi *coreApi, VkInstance instance, VulkanInstanceApi *outInstanceApi) {
	if(instance == VK_NULL_HANDLE)
		return(false);
	if(coreApi == fpl_null || coreApi->vkGetInstanceProcAddr == fpl_null)
		return(false);
	fplClearStruct(outInstanceApi);

#define VULKAN_INSTANCE_GET_PROC_ADDRESS(target, type, name) \
	(target)->name = (type)coreApi->vkGetInstanceProcAddr(instance, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING("Vulkan", "Failed getting instance procedure address '%s'", #name); \
		break; \
	}

	bool result = false;
	do {

		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkDestroySurfaceKHR, vkDestroySurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkEnumeratePhysicalDevices, vkEnumeratePhysicalDevices);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkGetPhysicalDeviceFeatures, vkGetPhysicalDeviceFeatures);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkGetPhysicalDeviceQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkEnumerateDeviceExtensionProperties, vkEnumerateDeviceExtensionProperties);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkEnumerateDeviceLayerProperties, vkEnumerateDeviceLayerProperties);

#if defined(FPL_PLATFORM_WINDOWS)
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkCreateWin32SurfaceKHR, vkCreateWin32SurfaceKHR);
		VULKAN_INSTANCE_GET_PROC_ADDRESS(outInstanceApi, PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR, vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif

		result = true;
	} while(0);

#undef VULKAN_INSTANCE_GET_PROC_ADDRESS

	if(!result) {
		fplClearStruct(outInstanceApi);
	}

	outInstanceApi->instance = instance;

	return(result);
}

void UnloadVulkanInstanceAPI(VulkanInstanceApi *api) {
	fplClearStruct(api);
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

typedef struct VulkanInstance {
	VulkanInstanceProperties properties;
	VulkanInstanceApi instanceApi;
	VkApplicationInfo appInfo;
	VkInstance instance;
} VulkanInstance;

static void DestroyVulkanInstance(const VulkanCoreApi *coreApi, VulkanInstance *instance) {
	if(coreApi == fpl_null || instance == fpl_null) return;

	// Unload Instance API
	UnloadVulkanInstanceAPI(&instance->instanceApi);

	// Destroy Vulkan instance
	if(instance->instance != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan instance\n");
		coreApi->vkDestroyInstance(instance->instance, fpl_null);
	}

	// Destroy instance properties
	DestroyVulkanInstanceProperties(&instance->properties);

	fplClearStruct(instance);
}

static bool CreateVulkanInstance(const VulkanCoreApi *coreApi, VulkanInstance *instance) {
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
		DestroyVulkanInstance(coreApi, instance);
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

	bool useValidation = true;

	uint32_t enabledInstanceExtensionCount = 0;
	const char *enabledInstanceExtensions[8] = { 0 };
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = khrSurfaceName; // This is always supported
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = khrPlatformSurfaceName;
	if(useValidation) {
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	uint32_t enabledInstanceLayerCount = 0;
	const char *enabledInstanceLayers[8] = { 0 };
	if(useValidation) {
		if(supportsValidationLayer) {
			enabledInstanceLayers[enabledInstanceLayerCount++] = validationLayerName;
		}
	}

	VkInstanceCreateInfo instanceCreateInfo = fplZeroInit;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = appInfo;
	instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	instanceCreateInfo.enabledLayerCount = enabledInstanceLayerCount;
	instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
	instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayers;

	char appVersionName[100] = { 0 };
	char engineVersionName[100] = { 0 };
	char vulkanVersionName[100] = { 0 };
	VulkanVersionToString(appInfo->applicationVersion, fplArrayCount(appVersionName), appVersionName);
	VulkanVersionToString(appInfo->engineVersion, fplArrayCount(engineVersionName), engineVersionName);
	VulkanVersionToString(appInfo->apiVersion, fplArrayCount(vulkanVersionName), vulkanVersionName);

	fplConsoleFormatOut("Creating Vulkan instance for application '%s' v%s and engine '%s' v%s for Vulkan v%s...\n", appInfo->pApplicationName, appVersionName, appInfo->pEngineName, engineVersionName, vulkanVersionName);
	fplConsoleFormatOut("With %lu enabled extensions & %lu layers\n", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount);
	res = coreApi->vkCreateInstance(&instanceCreateInfo, fpl_null, &instance->instance);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed creating Vulkan instance for application '%s'!\n", appInfo->pApplicationName);
		DestroyVulkanInstance(coreApi, instance);
		return(false);
	}
	fplConsoleFormatOut("Successfully created instance\n");
	fplConsoleFormatOut("\n");

	//
	// Load instance API
	//
	if(!LoadVulkanInstanceAPI(coreApi, instance->instance, &instance->instanceApi)) {
		fplConsoleFormatError("Failed to load the Vulkan instance API for instance '%p'!\n", instance->instance);
		DestroyVulkanInstance(coreApi, instance);
		return(false);
	}

	return(true);
}

typedef struct VulkanLogicalDevice {
	VkDevice logicalDevice;
	int32_t computeFamilyIndex;
	int32_t transferFamilyIndex;
	int32_t graphicsFamilyIndex;
} VulkanLogicalDevice;

bool CreateVulkanLogicalDevice(VkPhysicalDevice physicalDevice, VulkanLogicalDevice *outLogicalDevice) {
	return(false);
}

void DestroyVulkanLogicalDevice(VulkanLogicalDevice *logicalDevice) {
	fplClearStruct(logicalDevice);
	logicalDevice->computeFamilyIndex = -1;
	logicalDevice->graphicsFamilyIndex = -1;
	logicalDevice->transferFamilyIndex = -1;
}

typedef struct VulkanPhysicalDevice {
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	FIXED_TYPED_ARRAY(VkQueueFamilyProperties, queueFamilies);
	StringTable supportedExtensions;

	VkPhysicalDevice physicalDevice;
	const char *name;
} VulkanPhysicalDevice;

void DestroyVulkanPhysicalDevice(const VulkanCoreApi *coreApi, VulkanPhysicalDevice *physicalDevice) {
	if(coreApi == fpl_null || physicalDevice == fpl_null) return;
	FreeStringTable(&physicalDevice->supportedExtensions);
	FREE_FIXED_TYPED_ARRAY(&physicalDevice->queueFamilies);
	fplClearStruct(physicalDevice);
}

bool CreateVulkanPhysicalDevice(const VulkanCoreApi *coreApi, const VulkanInstanceApi *instanceApi, VkInstance instance, VulkanPhysicalDevice *physicalDevice) {
	VkResult res;

	//
	// Get Physical Devices
	//
	fplConsoleFormatOut("Enumerate physical devices for instance '%p'\n", instance);
	uint32_t physicalDeviceCount = 0;
	res = instanceApi->vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, fpl_null);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instance);
		DestroyVulkanPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	if(physicalDevices == fpl_null) {
		fplConsoleFormatError("Failed allocating memory for %lu physical devices!\n", physicalDeviceCount);
		DestroyVulkanPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}
	res = instanceApi->vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
	if(res != VK_SUCCESS) {
		fplConsoleFormatError("Failed enumerating physical instances for instance '%p'!\n", instance);
		DestroyVulkanPhysicalDevice(coreApi, physicalDevice);
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
		fplConsoleFormatOut("[%lu] Physical Device '%s' (%s)\n", physicalDeviceIndex, physicalDeviceProps.deviceName, VulkanPhysicalDeviceTypeToStríng(physicalDeviceProps.deviceType));

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
		DestroyVulkanPhysicalDevice(coreApi, physicalDevice);
		return(false);
	}

	fplConsoleOut("\n");

	instanceApi->vkGetPhysicalDeviceProperties(foundGpu, &physicalDevice->properties);
	instanceApi->vkGetPhysicalDeviceFeatures(foundGpu, &physicalDevice->features);
	instanceApi->vkGetPhysicalDeviceMemoryProperties(foundGpu, &physicalDevice->memoryProperties);
	physicalDevice->physicalDevice = foundGpu;
	physicalDevice->name = physicalDevice->properties.deviceName;

	fplConsoleFormatOut("Using [%lu] Physical Device '%s' (%s)\n", foundGPUIndex, physicalDevice->name, VulkanPhysicalDeviceTypeToStríng(physicalDevice->properties.deviceType));
	fplConsoleOut("\n");

	//
	// Queue Families
	//

	// TODO(final): Make a function for getting the queue family properties

	fplConsoleFormatOut("Get queue family properties for Physical Device '%s'\n", physicalDevice->name);
	uint32_t queueFamilyPropertiesCount = 0;
	instanceApi->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDevice, &queueFamilyPropertiesCount, fpl_null);
	assert(queueFamilyPropertiesCount > 0);

	ALLOC_FIXED_TYPED_ARRAY(&physicalDevice->queueFamilies, VkQueueFamilyProperties, queueFamilyPropertiesCount);
	instanceApi->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->physicalDevice, &queueFamilyPropertiesCount, physicalDevice->queueFamilies.items);
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
	instanceApi->vkEnumerateDeviceExtensionProperties(physicalDevice->physicalDevice, fpl_null, &deviceExtensionCount, fpl_null);
	if(deviceExtensionCount > 0) {
		VkExtensionProperties *extensionProperties = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
		instanceApi->vkEnumerateDeviceExtensionProperties(physicalDevice->physicalDevice, fpl_null, &deviceExtensionCount, extensionProperties);
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

typedef struct VulkanState {
	VulkanPhysicalDevice physicalDevice;

	VulkanCoreApi coreApi;

	VulkanLogicalDevice logicalDevice;

	VulkanInstance instance;

	VkSurfaceKHR surface;

	fpl_b32 isInitialized;
} VulkanState;

static void ShutdownVulkan(VulkanState *state) {
	if(state == fpl_null) return;

	// Destroy Logical Device
	DestroyVulkanLogicalDevice(&state->logicalDevice);
	
	// Destroy Physical device
	DestroyVulkanPhysicalDevice(&state->coreApi, &state->physicalDevice);

	// Destroy Surface KHR
	if(state->surface != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan surface '%p'\n", state->surface);
		state->instance.instanceApi.vkDestroySurfaceKHR(state->instance.instance, state->surface, fpl_null);
	}

	// Destroy Instance
	DestroyVulkanInstance(&state->coreApi, &state->instance);

	// Unload Core API
	UnloadVulkanCoreAPI(&state->coreApi);

	fplClearStruct(state);
}

static bool InitializeVulkan(VulkanState *state) {
	if(state == fpl_null)
		return(false);

	if(state->isInitialized) {
		fplConsoleError("Vulkan is already initialized!\n");
		return(false);
	}

	fplClearStruct(state);

	VulkanCoreApi *coreApi = &state->coreApi;
	VulkanInstanceApi *instanceApi = &state->instance.instanceApi;


	if(!LoadVulkanCoreAPI(coreApi)) {
		fplConsoleFormatError("Failed to load the Vulkan API!\n");
		goto failed;
	}
	fplConsoleFormatOut("\n");

	VkResult res;

#define VK_CHECK(res, emsg, ...) if(res != VK_SUCCESS) \
	{ \
		fplConsoleFormatError(emsg, __VA_ARGS__);\
		goto failed; \
	}

	//
	// Create instance
	//
	if(!CreateVulkanInstance(coreApi, &state->instance)) {
		fplConsoleFormatError("Failed to create a Vulkan instance!\n");
		goto failed;
	}

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
	res = instanceApi->vkCreateWin32SurfaceKHR(state->instance.instance, &createInfo, fpl_null, &state->surface);
	VK_CHECK(res, "Failed creating win32 surface KHR!\n");
	fplConsoleFormatOut("Successfully created win32 surface KHR\n");
	fplConsoleFormatOut("\n");
#endif

	//
	// Physical Device
	//
	if(!CreateVulkanPhysicalDevice(coreApi, instanceApi, state->instance.instance, &state->physicalDevice)) {
		fplConsoleFormatError("Failed to find a physical device from instance '%p'!\n", state->instance.instance);
		goto failed;
	}

	//
	// Find queue families
	//
	fplConsoleFormatOut("Detect queue families...\n");
	state->logicalDevice.graphicsFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, state->physicalDevice.queueFamilies.items, state->physicalDevice.queueFamilies.itemCount);
	state->logicalDevice.computeFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, state->physicalDevice.queueFamilies.items, state->physicalDevice.queueFamilies.itemCount);
	state->logicalDevice.transferFamilyIndex = GetVulkanQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, state->physicalDevice.queueFamilies.items, state->physicalDevice.queueFamilies.itemCount);
	if(state->logicalDevice.graphicsFamilyIndex == -1) {
		fplConsoleFormatError("No queue family for graphics found!\n");
		goto failed;
	}
	if(state->logicalDevice.computeFamilyIndex == -1) {
		// Use graphics queue for compute queue
		state->logicalDevice.computeFamilyIndex = state->logicalDevice.graphicsFamilyIndex;
	}
	if(state->logicalDevice.transferFamilyIndex == -1) {
		// Use graphics queue for transfer queue
		state->logicalDevice.transferFamilyIndex = state->logicalDevice.graphicsFamilyIndex;
	}
	assert(state->logicalDevice.graphicsFamilyIndex > -1 && state->logicalDevice.computeFamilyIndex > -1 && state->logicalDevice.transferFamilyIndex > -1);

	fplConsoleFormatOut("Successfully detected all queue families:\n");
	fplConsoleFormatOut("\tGraphics queue family: %d\n", state->logicalDevice.graphicsFamilyIndex);
	fplConsoleFormatOut("\tCompute queue family: %d\n", state->logicalDevice.computeFamilyIndex);
	fplConsoleFormatOut("\tTransfer queue family: %d\n", state->logicalDevice.transferFamilyIndex);
	fplConsoleOut("\n");

	//
	// TODO(final): Logical Device (vkDevice)
	//

	goto success;

failed:
	ShutdownVulkan(state);
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

	fplConsoleFormatOut("Initialize Platform\n");
	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_GameController | fplInitFlags_Console, &settings)) {
		fplPlatformResultType resultType = fplGetPlatformResult();
		const char *resultName = fplGetPlatformResultName(resultType);
		fplConsoleFormatError("Failed to initialize FPL '%s'!\n", resultName);
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized Platform\n");

	isPlatformInitialized = true;

	size_t stateSize = sizeof(VulkanState);
	state = (VulkanState *)fplMemoryAllocate(stateSize);
	if(state == fpl_null) {
		fplConsoleFormatError("Failed to allocate memory of size '%zu' for Vulkan state!", stateSize);
		goto cleanup;
	}

	fplConsoleFormatOut("Initialize Vulkan\n");
	if(!InitializeVulkan(state)) {
		fplConsoleFormatError("Failed to initialize Vulkan!\n");
		goto cleanup;
	}
	fplConsoleFormatOut("Successfully initialized Vulkan\n");

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
			ShutdownVulkan(state);
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