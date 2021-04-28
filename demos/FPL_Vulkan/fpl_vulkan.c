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

typedef struct VulkanLayerName {
	char name[256];
} VulkanLayerName;

typedef struct VulkanExtensionName {
	char name[256];
} VulkanExtensionName;

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

static bool LoadVulkanInstanceProperties(VulkanApi *vapi, VulkanInstanceProperties *outInstanceProperties) {
	VulkanInstanceProperties instanceProperties = fplZeroInit;

	VkResult res;

	//
	// Extensions
	//
	fplConsoleFormatOut("Enumerate instance extension properties...\n");
	uint32_t instanceExtensionCount = 0;
	res = vapi->vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, fpl_null);
	if(res != VK_SUCCESS) {
		return(false);
	}

	VkExtensionProperties *tempInstanceExtensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	if(tempInstanceExtensions == fpl_null) {
		return(false);
	}
	res = vapi->vkEnumerateInstanceExtensionProperties(fpl_null, &instanceExtensionCount, tempInstanceExtensions);
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
	res = vapi->vkEnumerateInstanceLayerProperties(&instanceLayerCount, fpl_null);
	if(res == VK_SUCCESS) {
		VkLayerProperties *tempInstanceLayers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * instanceLayerCount);
		if(tempInstanceLayers != fpl_null) {
			res = vapi->vkEnumerateInstanceLayerProperties(&instanceLayerCount, tempInstanceLayers);

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

int main(int argc, char **argv) {
	fplSettings settings = fplMakeDefaultSettings();
	settings.video.driver = fplVideoDriverType_None;

	int appResult = -1;

	bool isPlatformInitialized = false;

	VulkanApi vapi = fplZeroInit;

	VkInstance instance = VK_NULL_HANDLE;

	const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
	const char *khrSurfaceName = "VK_KHR_surface";

	const char *khrPlatformSurfaceName = fpl_null;
#if defined(FPL_PLATFORM_WINDOWS)
	khrPlatformSurfaceName = "VK_KHR_win32_surface";
#elif defined(FPL_SUBPLATFORM_X11)
	khrPlatformSurfaceName = "VK_KHR_xlib_surface";
#endif

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
	// Load properties
	//
	VulkanInstanceProperties instanceProperties = fplZeroInit;
	if(!LoadVulkanInstanceProperties(&vapi, &instanceProperties)) {
		fplConsoleFormatError("Failed loading instance properties!\n");
		goto cleanup;
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
	fplConsoleFormatOut("Validate extensions:\n");
	fplConsoleFormatOut("- Supported %s: %s\n", khrSurfaceName, (supportsKHRSurface ? "yes" : "no"));
	fplConsoleFormatOut("- Supported %s: %s\n", khrPlatformSurfaceName, (supportsKHRPlatformSurface ? "yes" : "no"));

	if(!supportsKHRSurface || !supportsKHRPlatformSurface || khrPlatformSurfaceName == fpl_null) {
		fplConsoleFormatError("Not supported KHR platform!\n");
		goto cleanup;
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
		} else {
			fplConsoleFormatError("The validation layer '%s' is not available at instance level!\n", validationLayerName);
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
	VersionToString(appInfo.applicationVersion, fplArrayCount(appVersionName), appVersionName);
	VersionToString(appInfo.engineVersion, fplArrayCount(engineVersionName), engineVersionName);
	VersionToString(appInfo.apiVersion, fplArrayCount(vulkanVersionName), vulkanVersionName);

	fplConsoleFormatOut("Creating Vulkan instance for application '%s' v%s and engine '%s' v%s for Vulkan v%s...\n", appInfo.pApplicationName, appVersionName, appInfo.pEngineName, engineVersionName, vulkanVersionName);
	fplConsoleFormatOut("With %lu enabled extensions & %lu layers\n", instanceCreateInfo.enabledExtensionCount, instanceCreateInfo.enabledLayerCount);
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