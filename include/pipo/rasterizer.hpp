#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "include/archetype_pool.hpp"

#undef NDEBUG
#ifdef NDEBUG
#define GL_CALL(x) x
#else
#define GL_CALL(x)                          \
        do {                                   \
            x;                                \
            GLenum err;                        \
            while ((err = glGetError()) != GL_NO_ERROR) { \
                std::cerr << "OpenGL error 0x" << std::hex << err \
                          << " at " << __FILE__ << ":" << std::dec << __LINE__ << std::endl; \
						  	__debugbreak(); \
            }                                 \
        } while(0)
#endif


class pipo
{
public:
	struct mesh_id
	{
		unsigned int vertex_array_object;
		unsigned int vertex_buffer_object;
		unsigned int element_buffer_object;
		unsigned int nb_of_indices;


		bool operator==(const mesh_id& other) const;
	};

	using texture_id = unsigned int;
	using shader_id = unsigned int;

	enum class view_type
	{
		perspective,
		orthographic
	};

	// interface data
	struct camera_data
	{
		view_type type;
		float fov;
		float c_near;
		float c_far;
		float aspect;
		bool active;
	};


	struct transform_data
	{
		float position[3] = {0, 0, 0};
		float scale[3] = { 0, 0, 0 };
		float rotation[3] = { 0, 0, 0 };

		glm::vec3 get_pos() const;
		glm::vec3 get_scale() const;
		glm::quat get_rotation() const;

		void set_pos(float x, float y, float z);
		void set_scale(float x, float y, float z);
		void set_rotation(float pitch, float yaw, float roll);
	};

	struct mesh_render_data
	{
		mesh_id mesh_id;
		bool visible;
	};

	struct render_texture_data
	{
		texture_id main_texture;
	};

	struct lit_material_data
	{
		texture_id main_texture;
		texture_id normal_texture;
	};

	struct unlit_material_data
	{
		texture_id main_texture;

		static shader_id program;
	};

	struct material_data
	{
		shader_id program;
	};

	struct glfw_window_data
	{
		void* window;
		glm::vec2 size;
	};

	struct texture
	{
		struct load_settings
		{
			bool  clamp_borders;
			char* file_path;
		};

		struct allocate_settings
		{
			int            width;
			int            height;
			int            nb_of_channels;
			bool           generate_mipmaps;
			bool           clamp_borders;
			unsigned char* data; // optional
		};

		// Instance Data
		int            width;
		int            height;
		int            nb_of_channels;
		texture_id     id;
	};

	struct mesh
	{
		enum vertex_layout
		{
			vertex_2d,
			vertex_2d_normal,
			vertex_3d,
			vertex_3d_normal,
		};

		struct load_settings
		{
			char* file_path;
		};

		struct allocate_settings
		{
			unsigned char* vertices; // optional
			unsigned char* indices;
			int nb_of_vertices;
			int nb_of_indices;
			vertex_layout layout;
		};

		// Instance data
		mesh_id id;
		vertex_layout layout;

		static int get_stride(vertex_layout layout);
	};

	struct shader
	{
		struct load_settings
		{
			char* file_path;
		};

		struct allocate_settings
		{
			char* fragment_shader_code = nullptr;
			char* vertex_shader_code = nullptr;
		};

		shader_id id;
		int fragment_shader_id;
		int vertex_shader_id;
	};


	/// <summary>
	/// Resources that are allocated on the GPU
	/// </summary>
	struct resources
	{
		static texture_id load_texture_gpu(const texture::load_settings& settings);
		static texture_id allocate_texture_gpu(const texture::allocate_settings& settings);
		static bool       unload_texture_gpu(const texture_id& id);

		static void load_mesh_gpu(const mesh::load_settings& settings, std::vector<mesh_id>& loaded_meshes);
		static mesh_id allocate_mesh_gpu(const mesh::allocate_settings& settings);
		static bool    unload_mesh_gpu(const mesh_id& id);

		static shader_id load_shader_gpu(const shader::load_settings& settings);
		static shader_id create_shader_gpu(const shader::allocate_settings& settings);

		static void init_default_resources();

		static std::unordered_map<texture_id, texture>  textures;
		static std::unordered_map<mesh_id,    mesh>     meshes;
		static std::unordered_map<shader_id,  shader>   shaders;
		static void* window;
	};

	static bool init();
	static void deinit();

	// Create a glfw window and data representation and adds it to an entity
	static bool create_window(int width, int height, const char* title);

	// Create a glfw window and data representation and adds it to an entity
	static bool set_window_size(int width, int height, char* title);

	// Adds a camera data and transform data to an entity and more
	static bool create_camera_entity(const peetcs::entity_id entity);

	static void render_camera(const camera_data& camera, const render_texture_data& target_texture);

	static void render_texture_to_window(const glfw_window_data& window, const render_texture_data& target_texture);

	static bool start_frame();
	static void render_frame(peetcs::archetype_pool& pool);

private:
	static void render_misc_material_meshes(peetcs::archetype_pool& pool);
	static void render_unlit_material_meshes(peetcs::archetype_pool& pool);
};


inline void pipo::transform_data::set_rotation(float pitch, float yaw, float roll)
{
	rotation[0] = pitch;
	rotation[1] = yaw;
	rotation[2] = roll;
}

inline void pipo::transform_data::set_scale(float x, float y, float z)
{
	scale[0] = x;
	scale[1] = y;
	scale[2] = z;
}

inline void pipo::transform_data::set_pos(float x, float y, float z)
{
	position[0] = x;
	position[1] = y;
	position[2] = z;
}

inline glm::quat pipo::transform_data::get_rotation() const
{
	return glm::quat(glm::vec3(rotation[0], rotation[1], rotation[2]));
}

inline glm::vec3 pipo::transform_data::get_scale() const
{
	return glm::vec3(scale[0], scale[1], scale[2]);
}

inline glm::vec3 pipo::transform_data::get_pos() const
{
	return glm::vec3(position[0], position[1], position[2]);
}

template <>
struct std::hash<pipo::mesh_id>
{
	std::size_t operator()(const pipo::mesh_id& id) const noexcept;
};
