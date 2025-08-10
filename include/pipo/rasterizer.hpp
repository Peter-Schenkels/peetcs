#pragma once

#include <vector>


using texture_id = unsigned int;
using shader_id = unsigned int;

struct render_target_id
{
	texture_id render_texture_id;
	unsigned int frame_buffer_id;
	unsigned int depth_buffer_id;

	bool operator==(const render_target_id& other) const;
};


struct hash_render_target_id
{
	size_t operator()(const render_target_id& k) const;
};

struct mesh_id
{
	unsigned int vertex_array_object;
	unsigned int vertex_buffer_object;
	unsigned int element_buffer_object;
	unsigned int nb_of_indices;


	bool operator==(const mesh_id& other) const;
};

struct hash_mesh_id
{
	size_t operator()(const mesh_id& k) const;
};



#include <numbers>
#include <array>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.hpp"
#include "include/archetype_pool.hpp"

#undef max

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
	        switch (err) { \
            case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM"; break; \
            case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE"; break; \
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION"; break; \
            case GL_STACK_OVERFLOW: std::cerr << "GL_STACK_OVERFLOW"; break; \
            case GL_STACK_UNDERFLOW: std::cerr << "GL_STACK_UNDERFLOW"; break; \
            case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY"; break; \
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION"; break; \
            default: std::cerr << "Unknown error"; break; \
        } \
        std::cerr << std::endl; \
						  	__debugbreak(); \
            }                                 \
        } while(0)
#endif





class pipo
{
public:


	enum class view_type
	{
		perspective,
		orthographic
	};

	// interface data
	struct camera_data
	{
		constexpr static uint16_t id = 2;

		view_type type;
		render_target_id render_target;
		float fov;
		float c_near;
		float c_far;
		float aspect;
		bool active;

		static void load_component(json& component_json, camera_data& data, pipo& gpu_context);
	};


	struct transform_data
	{
		constexpr static uint16_t id = 1;

		float position[3] = {0, 0, 0};
		float scale[3] = { 1, 1, 1 };
		float rotation[4] = { 0, 0, 0 };

		bool dirty = true;
		glm::mat4 transform_matrix;

		peetcs::entity_id parent = std::numeric_limits<peetcs::entity_id>::max();

		glm::vec3 get_pos() const;
		glm::vec3 get_ws_pos(peetcs::archetype_pool& pool);
		glm::vec3 get_scale() const;
		glm::quat get_rotation() const;
		glm::vec3 get_euler_rotation() const;

		void set_pos(float x, float y, float z);
		void set_pos(glm::vec3 pos);
		void set_scale(float x, float y, float z);
		void set_rotation(float pitch, float yaw, float roll);
		void set_rotation(const glm::quat& quat);
		void set_rotation(const glm::vec3& euler);

		glm::mat4 get_local_space();
		glm::mat4 get_world_space(peetcs::archetype_pool& pool);

		transform_data& get_root_transform(peetcs::archetype_pool& pool);

		static void load_component(json& component_json, transform_data& data, const pipo& gpu_context);
	};

	struct mesh_renderer_data
	{
		constexpr static uint16_t id = 8 ;

		mesh_id mesh;
		bool visible;

		static void load_component(json& component_json, mesh_renderer_data& data, pipo& gpu_context);
	};

	struct render_target_renderer_data
	{
		constexpr static uint16_t id = 3;

		int x;
		int y;
		int height;
		int width;
		render_target_id target_id;
		bool visible;

		static void load_component(json& component_json, render_target_renderer_data& data, pipo& gpu_context);
	};

	struct lit_material_data
	{
		constexpr static uint16_t id = 4;

		texture_id main_texture;
		texture_id normal_texture;

		static void load_component(json& component_json, lit_material_data& data, pipo& gpu_context);
	};

	struct unlit_material_data
	{
		constexpr static uint16_t id = 5;

		texture_id main_texture;

		static shader_id program;

		static void load_component(json& component_json, unlit_material_data& data, pipo& gpu_context);
	};

	struct material_data
	{
		constexpr static uint16_t id = 6;

		shader_id program;
	};

	struct glfw_window_data
	{
		constexpr static uint16_t id = 7;

		void* window;
		glm::vec2 size;
	};

	struct directional_light_data
	{
		
	};

	struct texture
	{
		struct load_settings
		{
			bool  clamp_borders = false;
			bool nearest_neighbour = false;
			char* file_path;
		};

		struct allocate_settings
		{
			int            width;
			int            height;
			int            nb_of_channels;
			bool           generate_mipmaps = false;
			bool           clamp_borders = false;
			unsigned char* data = nullptr; // optional
			bool nearest_neighbour = false;
			std::string_view name;
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
			debug,
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

	struct render_target
	{
		enum class type
		{
			color,
			color_depth,
			depth,
		};

		struct allocate_settings
		{
			std::string_view name;
			type target_type;
			int width;
			int height;
		};

		render_target_id id;
		int width;
		int height;
	};


	struct debug
	{
		void clear_draw_calls();

		friend pipo;

	private:
		struct draw_call
		{
			glm::vec3 color;
			std::vector<glm::vec4> vertices;
			std::vector<unsigned int> indices;

			glm::mat4 model_matrix;
		};

		std::vector<draw_call> queued_draw_calls;
		static shader_id debug_shader_id;
	};

	/// <summary>
	/// Resources that are allocated on the GPU
	/// </summary>
	struct resources
	{
		std::unordered_map<texture_id, texture> textures;
		std::vector<mesh> meshes;
		std::unordered_map<shader_id, shader> shaders;
		std::vector<render_target> render_targets;

		// Loaded resources name mapping
		std::unordered_map<std::string_view, texture_id> texture_names;
		std::unordered_map<texture_id, std::string_view> texture_id_names;
		std::unordered_map<std::string_view, std::vector<mesh_id>> mesh_names;
		std::unordered_map<mesh_id, std::string_view, hash_mesh_id> mesh_id_names;
		std::unordered_map<render_target_id, std::string_view, hash_render_target_id> render_target_names;
		std::unordered_map<std::string_view, render_target_id> name_render_targets;


		void* window;
		int width;
		int height;

		mesh_id quad_mesh;
		mesh_id plane_mesh;
		mesh_id cube_mesh;
		mesh_id sphere_mesh;
		shader_id render_texture_shader;

	};

	void draw_line_gizmo(glm::vec3 a, glm::vec3 b, glm::vec3 color);
	void draw_sphere(glm::vec3 position, float radius, glm::vec3 color);
	void draw_vertices(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, glm::vec3 color);
	void draw_aabb(const glm::vec3& min, glm::vec3& max, glm::vec3 color);
	void draw_cube_gizmo(glm::vec3 position, glm::vec3 scale, glm::quat rotation, glm::vec3 color);

	texture_id try_load_texture_gpu(const texture::load_settings& settings);
	texture_id load_texture_gpu(const texture::load_settings& settings);
	texture_id allocate_texture_gpu(const texture::allocate_settings& settings);
	bool       unload_texture_gpu(const texture_id& id);

	void try_load_mesh_gpu(const mesh::load_settings& settings, std::vector<mesh_id>& loaded_meshes);
	void load_mesh_gpu(const mesh::load_settings& settings, std::vector<mesh_id>& loaded_meshes);
	mesh_id allocate_mesh_gpu(const mesh::allocate_settings& settings);
	mesh_id get_quad() const;
	mesh_id get_plane() const;
	mesh_id get_sphere() const;
	mesh_id get_cube() const;
	bool    unload_mesh_gpu(const mesh_id& id);

	shader_id load_shader_gpu(const shader::load_settings& settings);
	shader_id create_shader_gpu(const shader::allocate_settings& settings);

	render_target_id create_render_target(const render_target::allocate_settings& settings);
	void init_default_resources();

	bool is_key_pressed(int key);


	bool init();
	void deinit();

	// Create a glfw window and data representation and adds it to an entity
	bool create_window(int width, int height, const char* title);

	// Create a glfw window and data representation and adds it to an entity
	bool set_window_size(int width, int height);

	bool bind_render_target(const render_target_id id);
	void unbind_render_target();

	void start_imgui();
	void end_imgui();
	void render_imgui(peetcs::archetype_pool& pool, std::vector<std::shared_ptr<gui_interface>> guis);
	bool start_frame();

	void render_frame(peetcs::archetype_pool& pool);
	void render_debug_draw_calls(const camera_data& camera, const transform_data& camera_transform);


	struct primitives
	{
		struct cube
		{
			static inline glm::vec3 s_axes[] =
			{
				glm::vec3{1.f, 0.f, 0.f}, // X-axis
				glm::vec3{0.f, 1.f, 0.f}, // Y-axis
				glm::vec3{0.f, 0.f, 1.f},  // Z-axis
				glm::vec3{-1.f, 0.f, 0.f}, // X-axis
				glm::vec3{0.f, -1.f, 0.f}, // Y-axis
				glm::vec3{0.f, 0.f, -1.f}  // Z-axis
			};

			static inline mesh::vertex_layout layout = mesh::vertex_layout::debug;

			static inline std::vector<unsigned int> indices{
								0, 1, 1, 2, 2, 3, 3, 0,     // fronte
								0, 6, 6, 7, 7, 1, 1, 0,     // sotto
								3, 2, 2, 5, 5, 4, 4, 3,     // sopra
								4, 5, 5, 7, 7, 6, 6, 4,     // retro
								2, 5, 5, 7, 7, 1, 1, 2,     // destra
								3, 4, 4, 6, 6, 0, 0, 3      // sinistra
			};

			static inline std::array<glm::vec3, 8> vertices = {
						  glm::vec3{ -1.f, -1.f,  1.f},      //0
						  { 1.f, -1.f,  1.f},      //1
						  { 1.f,  1.f,  1.f},      //2
						  {-1.f,  1.f,  1.f},      //3
						  {-1.f,  1.f, -1.f},      //4
						  { 1.f,  1.f, -1.f},      //5
						  {-1.f, -1.f, -1.f},      //6
				{ 1.f, -1.f, -1.f}       //7
			};
		};
	};

private:
	resources _resources;
	debug _debug = {};


	void render_meshes(peetcs::archetype_pool& pool);
	void render_unlit_material_meshes(peetcs::archetype_pool& pool, const camera_data& camera,
	                                         transform_data& camera_transform);
	void render_misc_material_meshes(peetcs::archetype_pool& pool, const camera_data& camera,
		transform_data& camera_transform);
};



inline void pipo::transform_data::set_rotation(const glm::vec3& euler)
{
	set_rotation(euler.x, euler.y, euler.z);
}


inline void pipo::transform_data::set_rotation(float pitch, float yaw, float roll)
{
	glm::vec3 eulerAngles = { fmod(pitch, std::numbers::pi * 2.f), fmod(yaw, std::numbers::pi * 2.f), fmod(roll, std::numbers::pi * 2.f) };
	auto quat = glm::quat(eulerAngles);
	set_rotation(quat);
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
	return glm::quat( rotation[0], rotation[1], rotation[2], rotation[3]);
}

inline glm::vec3 pipo::transform_data::get_scale() const
{
	return glm::vec3(scale[0], scale[1], scale[2]);
}

inline glm::vec3 pipo::transform_data::get_pos() const
{
	return glm::vec3(position[0], position[1], position[2]);
}

