//
// Created by nathan on 1/19/23.
//

#include "vulkan_errors.h"

namespace Vulkan
{

const char* vulkan_error_to_string(VkResult result)
{
	if (result == VK_SUCCESS)
		return "Command successfully completed";
	if (result == VK_NOT_READY)
		return "A fence or query has not yet completed";
	if (result == VK_TIMEOUT)
		return "A wait operation has not completed in the specified time";
	if (result == VK_EVENT_SET)
		return "An event is signaled";
	if (result == VK_EVENT_RESET)
		return "An event is unsignaled";
	if (result == VK_INCOMPLETE)
		return "A return array was too small for the result";
	if (result == VK_SUBOPTIMAL_KHR)
		return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
	if (result == VK_THREAD_IDLE_KHR)
		return "A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
	if (result == VK_THREAD_DONE_KHR)
		return "A deferred operation is not complete but there is no work remaining to assign to additional threads.";
	if (result == VK_OPERATION_DEFERRED_KHR)
		return "A deferred operation was requested and at least some of the work was deferred.";
	if (result == VK_OPERATION_NOT_DEFERRED_KHR)
		return "A deferred operation was requested and no operations were deferred.";
	if (result == VK_PIPELINE_COMPILE_REQUIRED)
		return "A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";
	if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
		return "A host memory allocation has failed.";
	if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		return "A device memory allocation has failed.";
	if (result == VK_ERROR_INITIALIZATION_FAILED)
		return "Initialization of an object could not be completed for implementation-specific reasons.";
	if (result == VK_ERROR_DEVICE_LOST)
		return "The logical or physical device has been lost. See Lost Device";
	if (result == VK_ERROR_MEMORY_MAP_FAILED)
		return "Mapping of a memory object has failed.";
	if (result == VK_ERROR_LAYER_NOT_PRESENT)
		return "A requested layer is not present or could not be loaded.";
	if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
		return "A requested extension is not supported.";
	if (result == VK_ERROR_FEATURE_NOT_PRESENT)
		return "A requested feature is not supported.";
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
		return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
	if (result == VK_ERROR_TOO_MANY_OBJECTS)
		return "Too many objects of the type have already been created.";
	if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
		return "A requested format is not supported on this device.";
	if (result == VK_ERROR_FRAGMENTED_POOL)
		return "A pool allocation has failed due to fragmentation of the pool’s memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
	if (result == VK_ERROR_SURFACE_LOST_KHR)
		return "A surface is no longer available.";
	if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
		return "The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
	if (result == VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
		return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
	if (result == VK_ERROR_INVALID_SHADER_NV)
		return "One or more shaders failed to compile or link. More details are reported back to the application via VK_EXT_debug_report if enabled.";
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
		return "A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
	if (result == VK_ERROR_INVALID_EXTERNAL_HANDLE)
		return "An external handle is not a valid handle of the specified type.";
	if (result == VK_ERROR_FRAGMENTATION)
		return "A descriptor pool creation has failed due to fragmentation.";
	if (result == VK_ERROR_INVALID_DEVICE_ADDRESS_EXT)
		return "A buffer creation failed because the requested address is not available.";
	if (result == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
		return "A buffer creation or memory allocation failed because the requested address is not available. A shader group handle assignment failed because the requested shader group handle information is no longer valid.";
	if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
		return "An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exclusive full-screen access. This may occur due to implementation-dependent reasons, outside of the application’s control.";
	if (result == VK_ERROR_COMPRESSION_EXHAUSTED_EXT)
		return "An image creation failed because internal resources required for compression are exhausted. This must only be returned when fixed-rate compression is requested.";
	if (result == VK_ERROR_UNKNOWN)
		return "An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
	return "VkResult returned an unknown error code!";
}

}
