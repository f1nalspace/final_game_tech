#define FPL_IMPLEMENTATION
#include <final_platform_layer.h>

//#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <malloc.h>

typedef struct VulkanApi {
	fplDynamicLibraryHandle libHandle;
	PFN_vkCreateInstance vkCreateInstance;
	PFN_vkDestroyInstance vkDestroyInstance;
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
	fpl_b32 isInitialized;
} VulkanApi;

void UnloadVulkanAPI(VulkanApi *api) {
	if(api->isInitialized) {
		fplConsoleFormatOut("Unload Vulkan API\n");
		fplDynamicLibraryUnload(&api->libHandle);
	}
	fplClearStruct(api);
}

bool LoadVulkanAPI(VulkanApi *api) {
	const char *vulkanLibraryFileName = "vulkan-1.dll";

	fplConsoleFormatOut("Load Vulkan API '%s'\n", vulkanLibraryFileName);
	if(!fplDynamicLibraryLoad(vulkanLibraryFileName, &api->libHandle)) {
		return(false);
	}

	fplDynamicLibraryHandle *handle = &api->libHandle;

#define VULKAN_GET_PROC_ADDRESS_BREAK(mod, libHandle, libName, target, type, name) \
	(target)->name = (type)fplGetDynamicLibraryProc(libHandle, #name); \
	if ((target)->name == fpl_null) { \
		FPL__WARNING(mod, "Failed getting procedure address '%s' from library '%s'", #name, libName); \
		break; \
	}

	bool result = false;
	do {

		VULKAN_GET_PROC_ADDRESS_BREAK("Vulkan", handle, vulkanLibraryFileName, api, PFN_vkCreateInstance, vkCreateInstance);
		VULKAN_GET_PROC_ADDRESS_BREAK("Vulkan", handle, vulkanLibraryFileName, api, PFN_vkDestroyInstance, vkDestroyInstance);
		VULKAN_GET_PROC_ADDRESS_BREAK("Vulkan", handle, vulkanLibraryFileName, api, PFN_vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceExtensionProperties);
		VULKAN_GET_PROC_ADDRESS_BREAK("Vulkan", handle, vulkanLibraryFileName, api, PFN_vkEnumerateInstanceLayerProperties, vkEnumerateInstanceLayerProperties);

		result = true;
	} while(0);

	if(!result) {
		UnloadVulkanAPI(api);
	}
	api->isInitialized = true;
	return(result);
}

static void VersionToString(const uint32_t versionNumber, const size_t outNameCapacity, char *outName) {
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

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_None;

	int appResult = -1;

	bool isPlatformInitialized = false;

	VulkanApi vapi = fplZeroInit;

	VkInstance instance = VK_NULL_HANDLE;

	fplConsoleFormatOut("Initialize Platform\n");
	if(!fplPlatformInit(fplInitFlags_Window | fplInitFlags_GameController | fplInitFlags_Console, &settings)) {
		fplPlatformResultType resultType = fplGetPlatformResult();
		const char *resultName = fplGetPlatformResultName(resultType);
		fplConsoleFormatError("Failed to initialize FPL '%s'!\n", resultName);
		goto cleanup;
	}

	isPlatformInitialized = true;

	if(!LoadVulkanAPI(&vapi)) {
		fplConsoleFormatError("Failed to load the Vulkan API!\n");
		goto cleanup;
	}
	fplConsoleFormatOut("\n");

#define VK_CHECK(res, emsg, ...) if(res != VK_SUCCESS) \
	{ \
		fplConsoleFormatError(emsg, __VA_ARGS__);\
		goto cleanup; \
	}

	VkResult functionResult;

	//
	// Instance extensions
	//
	bool supportsKHRSurface = false;
	bool supportsKHRPlatformSurface = false;
	const char *platformKHRExtensionName = fpl_null;

	fplConsoleFormatOut("Query instance extension count\n");
	uint32_t instanceExtensionCount = 0;
	functionResult = vapi.vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, fpl_null);
	VK_CHECK(functionResult, "Failed getting the instance extension count!\n");
	fplConsoleFormatOut("Got instance extension count of %lu\n\n", instanceExtensionCount);

	fplConsoleFormatOut("Get %lu instance extensions\n", instanceExtensionCount);
	VkExtensionProperties *availableInstanceExtensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	if(availableInstanceExtensions == fpl_null) {
		fplConsoleFormatError("Failed to allocate memory for %lu instance extensions!\n", instanceExtensionCount);
		goto cleanup;
	}
	functionResult = vapi.vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, availableInstanceExtensions);
	VK_CHECK(functionResult, "Failed getting %lu instance extensions!\n", instanceExtensionCount);
	fplConsoleFormatOut("Successfully got %lu instance extensions\n", instanceExtensionCount);

	for(uint32_t i = 0; i < instanceExtensionCount; ++i) {
		const VkExtensionProperties *p = availableInstanceExtensions + i;
		fplConsoleFormatOut("- %s\n", p->extensionName);
		if(fplIsStringEqual("VK_KHR_surface", p->extensionName)) {
			supportsKHRSurface = true;
		}
#if defined(FPL_PLATFORM_WINDOWS)
		if(fplIsStringEqual("VK_KHR_win32_surface", p->extensionName)) {
			platformKHRExtensionName = "VK_KHR_win32_surface";
			supportsKHRPlatformSurface = true;
		}
#elif defined(FPL_SUBPLATFORM_X11)
		if(fplIsStringEqual("VK_KHR_xlib_surface", p->extensionName)) {
			platformKHRExtensionName = "VK_KHR_xlib_surface";
			supportsKHRPlatformSurface = true;
		}
#endif
	}

	free(availableInstanceExtensions);

	//
	// Check Extensions
	//
	if(!supportsKHRSurface || !supportsKHRPlatformSurface) {
		fplConsoleFormatError("No supported KHR platform found!\n");
		goto cleanup;
	}

	assert(platformKHRExtensionName != fpl_null);

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
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = "VK_KHR_surface"; // This is always supported
	enabledInstanceExtensions[enabledInstanceExtensionCount++] = platformKHRExtensionName;
	if(useValidation) {
		enabledInstanceExtensions[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

	uint32_t enabledInstanceLayerCount = 0;
	const char *enabledInstanceLayers[8] = { 0 };
	if(useValidation) {
		const char *validationLayerName = "VK_LAYER_KHRONOS_validation";

		bool supportsValidationLayer = false;

		fplConsoleFormatOut("Query instance layer count\n");
		uint32_t availableInstanceLayerCount = 0;
		functionResult = vapi.vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, fpl_null);
		if(functionResult == VK_SUCCESS) {
			VkLayerProperties *availableInstanceLayers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * availableInstanceLayerCount);
			if(availableInstanceLayers != fpl_null) {
				functionResult = vapi.vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, availableInstanceLayers);
				if(functionResult == VK_SUCCESS) {
					fplConsoleFormatOut("Successfully got %lu instance layers\n", availableInstanceLayerCount);
					for(uint32_t layerIndex = 0; layerIndex < availableInstanceLayerCount; ++layerIndex) {
						VkLayerProperties *layer = availableInstanceLayers + layerIndex;
						fplConsoleFormatOut("- %s\n", layer->layerName);
						if(fplIsStringEqual(layer->layerName, validationLayerName)) {
							supportsValidationLayer = true;
						}
					}
				}
				free(availableInstanceLayers);
			}
		}

		if(supportsValidationLayer) {
			enabledInstanceLayers[enabledInstanceLayerCount++] = validationLayerName;
		} else {
			fplConsoleFormatError("The validation layer '%s' is not available at instance level!\n", validationLayerName);
		}
		fplConsoleFormatOut("\n");
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
	VersionToString(appInfo.applicationVersion, fplArrayCount(appVersionName), appVersionName);
	VersionToString(appInfo.engineVersion, fplArrayCount(engineVersionName), engineVersionName);
	VersionToString(appInfo.apiVersion, fplArrayCount(vulkanVersionName), vulkanVersionName);

	fplConsoleFormatOut("Creating Vulkan instance for application '%s' v%s and engine '%s' v%s for Vulkan v%s\n", appInfo.pApplicationName, appVersionName, appInfo.pEngineName, engineVersionName, vulkanVersionName);
	functionResult = vapi.vkCreateInstance(&instanceCreateInfo, fpl_null, &instance);
	VK_CHECK(functionResult, "Failed creating Vulkan instance for application '%s'!", appInfo.pApplicationName);
	fplConsoleFormatOut("Successfully created instance\n", instance);
	fplConsoleFormatOut("\n");

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
	if(isPlatformInitialized) {
		if(instance != VK_NULL_HANDLE) {
			fplConsoleFormatOut("Destroy Vulkan instance\n");
			vapi.vkDestroyInstance(instance, fpl_null);
		}
		UnloadVulkanAPI(&vapi);

		fplConsoleFormatOut("Shutdown platform\n");
		fplPlatformRelease();
	}
	return(appResult);
}