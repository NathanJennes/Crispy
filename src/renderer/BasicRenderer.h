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
	class Model
	{
	public:
		Model()	= default;

		explicit Model(u32 new_id);
		Model(const Model& other);
		Model(Model&& other) noexcept;
		Model& operator=(const Model& other);
		Model& operator=(Model&& other) noexcept;
		~Model();

		void	reset_id();
		void	unload();

		//----
		// Getters
		//----
		[[nodiscard]] const std::optional<u32>&	get_id()	const { return id; }

	private:
		std::optional<u32>	id;
	};

public:		// Methods
	static bool	initialize();
	static void	shutdown();

	//----
	// Drawing
	//----
	static void	begin_frame();
	static void	draw(const Model& model, const glm::vec3& pos);
	static void	draw(const Model& model, const glm::vec3& pos, const glm::vec3& rotation);
	static void	draw(const Model& model, const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3& scale);
	static void	end_frame();

	//----
	// Model management
	//----
	static Model	load_model(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
	static bool		unload_model(Model& model);

private:	// Types
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

	class ModelImpl
	{
	public:
		ModelImpl()											= default;
		ModelImpl(const ModelImpl& other)					= default;
		ModelImpl(ModelImpl&& other) noexcept				= default;
		ModelImpl& operator=(const ModelImpl& other)		= default;
		ModelImpl& operator=(ModelImpl&& other) noexcept	= default;
		~ModelImpl()										= default;

		explicit ModelImpl(u32 new_id);

		void	release_ressources();
		void	add_reference()	{ reference_count++; }
		void	remove_reference()	{ reference_count--; if (reference_count == 0) BasicRenderer::remove_model_from_list(*this); }

		//----
		// Mesh management
		//----
		void	add_mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);

		//----
		// Getters
		//----
		[[nodiscard]] const std::optional<u32>&	get_id()		const { return id; }
		[[nodiscard]] const std::vector<Mesh>&	get_meshes()	const { return meshes; }

	private:
		std::vector<Mesh>	meshes;
		std::optional<u32>	id;
		u32					reference_count;
	}; // ModelImpl

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

	//----
	// Model management
	//----
	static void					remove_model_from_list(ModelImpl& model);
	static void					load_default_model();
	static u32					find_next_available_model_id();
	static ModelImpl&			find_model_by_id(u32 id);
	static bool					model_exists(u32 id);
	static bool					add_reference_to_model(u32 id);
	static bool					remove_reference_to_model(u32 id);

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
	static bool			alive;
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

	//----
	// Model management
	//----
	static ModelImpl				default_model;
	static std::vector<ModelImpl>	loaded_models;
};

}

#endif //BASICRENDERER_H
