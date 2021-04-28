#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

#if defined(FPL_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

//#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <malloc.h>

//
// Utils
//
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

typedef struct VulkanLayerName {
	char name[256];
} VulkanLayerName;

typedef struct VulkanExtensionName {
	char name[256];
} VulkanExtensionName;

typedef struct VulkanState {
	VulkanCoreApi coreApi;
	VulkanInstanceApi instanceApi;
	VkInstance instance;
	VkSurfaceKHR surface;
	fpl_b32 isInitialized;
} VulkanState;

typedef struct VulkanInstanceProperties {
	VulkanLayerName *layers;
	VulkanExtensionName *extensions;
	uint32_t layerCount;
	uint32_t extensionCount;
} VulkanInstanceProperties;

static void ReleaseVulkanInstanceProperties(VulkanInstanceProperties *instanceProperties) {
	if(instanceProperties->layers != fpl_null)
		free(instanceProperties->layers);
	if(instanceProperties->extensions != fpl_null)
		free(instanceProperties->extensions);
}

static bool LoadVulkanInstanceProperties(VulkanCoreApi *coreApi, VulkanInstanceProperties *outInstanceProperties) {
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

	instanceProperties.extensionCount = instanceExtensionCount;
	instanceProperties.extensions = (VulkanExtensionName *)malloc(sizeof(VulkanExtensionName) * instanceExtensionCount);

	fplConsoleFormatOut("Successfully got instance extension properties of %lu\n", instanceExtensionCount);
	for(uint32_t extensionIndex = 0; extensionIndex < instanceExtensionCount; ++extensionIndex) {
		const VkExtensionProperties *extProp = tempInstanceExtensions + extensionIndex;
		fplCopyString(extProp->extensionName, instanceProperties.extensions[extensionIndex].name, fplArrayCount(instanceProperties.extensions[extensionIndex].name));
		fplConsoleFormatOut("- %s\n", extProp->extensionName);
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
				instanceProperties.layerCount = instanceLayerCount;
				instanceProperties.layers = (VulkanLayerName *)malloc(sizeof(VulkanLayerName) * instanceLayerCount);
				for(uint32_t layerIndex = 0; layerIndex < instanceLayerCount; ++layerIndex) {
					VkLayerProperties *layerProp = tempInstanceLayers + layerIndex;
					fplCopyString(layerProp->layerName, instanceProperties.layers[layerIndex].name, fplArrayCount(instanceProperties.layers[layerIndex].name));
					fplConsoleFormatOut("- %s\n", layerProp->layerName);
				}
			}
			free(tempInstanceLayers);
		}
	}

	*outInstanceProperties = instanceProperties;
	return(true);
}

static void ShutdownVulkan(VulkanState *state) {
	if(state == fpl_null) return;

	if(state->surface != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan surface '%p'\n", state->surface);
		state->instanceApi.vkDestroySurfaceKHR(state->instance, state->surface, fpl_null);
	}

	UnloadVulkanInstanceAPI(&state->instanceApi);

	if(state->instance != VK_NULL_HANDLE) {
		fplConsoleFormatOut("Destroy Vulkan instance\n");
		state->coreApi.vkDestroyInstance(state->instance, fpl_null);
	}

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
	VulkanInstanceApi *instanceApi = &state->instanceApi;

	const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
	const char *khrSurfaceName = "VK_KHR_surface";

	const char *khrPlatformSurfaceName = fpl_null;
#if defined(FPL_PLATFORM_WINDOWS)
	khrPlatformSurfaceName = "VK_KHR_win32_surface";
#elif defined(FPL_SUBPLATFORM_X11)
	khrPlatformSurfaceName = "VK_KHR_xlib_surface";
#endif

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
	// Load properties
	//
	VulkanInstanceProperties instanceProperties = fplZeroInit;
	if(!LoadVulkanInstanceProperties(coreApi, &instanceProperties)) {
		fplConsoleFormatError("Failed loading instance properties!\n");
		goto failed;
	}

	//
	// Check and validate extensions and layers
	//
	bool supportsKHRSurface = false;
	bool supportsKHRPlatformSurface = false;
	for(uint32_t extensionIndex = 0; extensionIndex < instanceProperties.extensionCount; ++extensionIndex) {
		const char *extensionName = instanceProperties.extensions[extensionIndex].name;
		if(fplIsStringEqual(khrSurfaceName, extensionName)) {
			supportsKHRSurface = true;
		}
		if(fplIsStringEqual(khrPlatformSurfaceName, extensionName)) {
			supportsKHRPlatformSurface = true;
		}
	}

	bool supportsValidationLayer = false;
	for(uint32_t layerIndex = 0; layerIndex < instanceProperties.layerCount; ++layerIndex) {
		const char *layerName = instanceProperties.layers[layerIndex].name;
		if(fplIsStringEqual(layerName, validationLayerName)) {
			supportsValidationLayer = true;
		}
	}

	ReleaseVulkanInstanceProperties(&instanceProperties);

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
		goto failed;
	}

	fplConsoleFormatOut("\n");

	//
	// Vulkan Instance
	//
	VkApplicationInfo appInfo = fplZeroInit;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "FPL_Vulkan";
	appInfo.pEngineName = "FPL_Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

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
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	instanceCreateInfo.enabledLayerCount = enabledInstanceLayerCount;
	instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensions;
	instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayers;

	char appVersionName[100] = { 0 };
	char engineVersionName[100] = { 0 };
	char vulkanVersionName[100] = { 0 };
	VulkanVersionToString(appInfo.applicationVersion, fplArrayCount(appVersionName), appVersionName);
	VulkanVersionToString(appInfo.engineVersion, fplArrayCount(engineVersionName), engineVersionName);
	VulkanVersionToString(appInfo.apiVersion, fplArrayCount(vulkanVersionName), vulkanVersionName);

	fplConsoleFormatOut("Creating Vulkan instance for application '%s' v%s and engine '%s' v%s for Vulkan v%s...\n", appInfo.pApplicationName, appVersionName, appInfo.pEngineName, engineVersionName, vulkanVersionName);
	fplConsoleFormatOut("With %lu enabled extensions & %lu layers\n", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount);
	res = coreApi->vkCreateInstance(&instanceCreateInfo, fpl_null, &state->instance);
	VK_CHECK(res, "Failed creating Vulkan instance for application '%s'!\n", appInfo.pApplicationName);
	fplConsoleFormatOut("Successfully created instance\n");
	fplConsoleFormatOut("\n");

	//
	// Load instance API
	//
	if(!LoadVulkanInstanceAPI(coreApi, state->instance, instanceApi)) {
		fplConsoleFormatError("Failed to load the Vulkan instance API for instance '%p'!\n", state->instance);
		goto failed;
	}

#if defined(FPL_PLATFORM_WINDOWS)
	// TODO(final): This is just temporary, until we can query the platform window informations from FPL
	HWND windowHandle = fpl__global__AppState->window.win32.windowHandle;
	HINSTANCE appHandle = GetModuleHandle(fpl_null);

	VkWin32SurfaceCreateInfoKHR createInfo = fplZeroInit;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = windowHandle;
	createInfo.hinstance = appHandle;

	fplConsoleFormatOut("Creating win32 surface KHR from window handle '%p'\n", createInfo.hwnd);
	res = instanceApi->vkCreateWin32SurfaceKHR(state->instance, &createInfo, fpl_null, &state->surface);
	VK_CHECK(res, "Failed creating win32 surface KHR!\n");
	fplConsoleFormatOut("Successfully created win32 surface KHR\n");
	fplConsoleFormatOut("\n");
#endif

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

	size_t stateState = sizeof(VulkanState);
	state = (VulkanState *)fplMemoryAllocate(stateState);
	if(state == fpl_null) {
		fplConsoleFormatError("Failed to allocate memory of size '%zu' for vulkan state!", stateState);
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

		fplConsoleFormatOut("Shutdown Platform\n");
		fplPlatformRelease();
	}
	return(appResult);
}