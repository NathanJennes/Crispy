//
// Created by nathan on 1/19/23.
//

#ifndef BASICRENDERER_H
#define BASICRENDERER_H

#include <vector>
#include "Vertex.h"
#include "defines.h"
#include "vulkan/Buffer.h"

namespace Vulkan
{

class BasicRenderer
{
public:	// Types

	class Mesh
	{
	public:		// Methods
		Mesh();
		~Mesh() = default;
		Mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
		Mesh(const Mesh& other) = default;
		Mesh(Mesh&& other) noexcept;

		Mesh& operator=(const Mesh& other);
		Mesh& operator=(Mesh&& other) noexcept;

		void	release_resources()	{ vertex_buffer.release_resources(); index_buffer.release_resources(); };

		const Buffer&			get_vertex_buffer()		const	{ return vertex_buffer; }
		const Buffer&			get_index_buffer()		const	{ return index_buffer; }

		u64						get_vertex_count()		const	{ return vertex_count; }
		u64						get_index_count()		const	{ return index_count; }

	private:	// Methods

	private:	// Members
		Buffer	vertex_buffer;
		Buffer	index_buffer;

		u64		vertex_count;
		u64		index_count;
	}; // Mesh

public:		// Methods
	static bool	initialize();
	static void	shutdown();

	//----
	// Drawing
	//----
	static void	begin_frame();
	static void	draw(const Mesh& mesh, const glm::vec3& pos);
	static void	draw(const Mesh& mesh, const glm::vec3& pos, const glm::vec3& rotation);
	static void	draw(const Mesh& mesh, const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3& scale);
	static void	end_frame();

private:	// Methods

	//----
	// Initialization
	//----
	static bool	create_sync_objects();
	static bool	create_descriptor_pool();
	static bool	create_camera_descriptor_set();
	static void create_uniform_buffer();
	static bool	create_command_pool();
	static bool	create_command_buffer();

	//----
	// Shutdown
	//----
	static void	destroy_sync_objects();
	static void	destroy_descriptor_pool();
	static void	destroy_command_pool();

	//----
	// Drawing
	//----
	static void					wait_for_last_frame_finished();
	static std::optional<u32>	get_swapchain_image();
	static bool					begin_command_buffer();
	static void					begin_renderpass();
	static void					setup_viewport();
	static void					setup_camera_ubo();

	static void					end_renderpass();
	static bool					end_command_buffer();
	static bool					submit_command_buffer();
	static bool					present_frame();

private:	// Members
	//----
	// Synchronization
	//----
	static VkSemaphore	image_available_semaphore;
	static VkSemaphore	render_finished_semaphore;
	static VkFence		last_frame_presented_fence;

	//----
	// State
	//----
	static bool			frame_started;
	static bool			frame_available;
	static u32			current_image_index;

	//----
	// Uniform variables
	//----
	static Buffer			camera_uniform_buffer;
	static VkDescriptorPool	descriptor_pool;
	static VkDescriptorSet	camera_descriptor_set;

	//----
	// Command buffers
	//----
	static VkCommandPool	command_pool;
	static VkCommandBuffer	command_buffer;
};

}

#endif //BASICRENDERER_H
