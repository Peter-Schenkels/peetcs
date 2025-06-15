#pragma once



#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "imgui.hpp"
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

	struct render_target_id
	{
		texture_id render_texture_id;
		unsigned int frame_buffer_id;
		unsigned int depth_buffer_id;

		bool operator==(const render_target_id& other) const;
	};


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
	};


	struct transform_data
	{
		constexpr static uint16_t id = 1;

		float position[3] = {0, 0, 0};
		float scale[3] = { 0, 0, 0 };
		float rotation[3] = { 0, 0, 0 };

		peetcs::entity_id parent = std::numeric_limits<peetcs::entity_id>::max();

		glm::vec3 get_pos() const;
		glm::vec3 get_scale() const;
		glm::quat get_rotation() const;

		void set_pos(float x, float y, float z);
		void set_scale(float x, float y, float z);
		void set_rotation(float pitch, float yaw, float roll);

		glm::mat4 get_local_space() const;
		glm::mat4 get_world_space(peetcs::archetype_pool& pool) const;
	};

	struct mesh_renderer_data
	{
		constexpr static uint16_t id = 0;

		mesh_id mesh_id;
		bool visible;
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
	};

	struct lit_material_data
	{
		constexpr static uint16_t id = 4;

		texture_id main_texture;
		texture_id normal_texture;
	};

	struct unlit_material_data
	{
		constexpr static uint16_t id = 5;

		texture_id main_texture;

		static shader_id program;
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
		static void draw_line(glm::vec3 a, glm::vec3 b, glm::vec3 color);
		static void draw_sphere(glm::vec3 position, float radius, glm::vec3 color);
		static void draw_cube(glm::vec3 position, glm::vec3 scale, glm::quat rotation, glm::vec3 color);

		static void render_draw_calls(const camera_data& camera, const transform_data& camera_transform);
		static void clear_draw_calls();

		friend pipo;

	private:
		struct debug_draw_call
		{
			glm::vec3 color;
			std::vector<glm::vec4> vertices;
			std::vector<unsigned int> indices;

			glm::mat4 model_matrix;
		};

		static std::vector<debug_draw_call> queued_draw_calls;
		static shader_id debug_shader_id;
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

		static render_target_id create_render_target(const render_target::allocate_settings& settings);
		static void init_default_resources();

		static std::unordered_map<texture_id, texture>                textures;
		static std::unordered_map<mesh_id,    mesh>                   meshes;
		static std::unordered_map<shader_id,  shader>                 shaders;
		static std::unordered_map<render_target_id,  render_target>   render_targets;
		static void* window;
		static int width;
		static int height;

		static mesh_id quad_mesh;
		static shader_id render_texture_shader;

	};

	static bool init();
	static void deinit();

	// Create a glfw window and data representation and adds it to an entity
	static bool create_window(int width, int height, const char* title);

	// Create a glfw window and data representation and adds it to an entity
	static bool set_window_size(int width, int height);

	static bool bind_render_target(const render_target_id id);
	static void unbind_render_target();

	static void render_imgui(peetcs::archetype_pool& pool, std::vector<std::shared_ptr<gui_interface>> guis);
	static bool start_frame();

	static void render_frame(peetcs::archetype_pool& pool);

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

			static inline std::vector<unsigned int> indices{
				//Top
				2, 6, 7,
				2, 3, 7,

				//Bottom
				0, 4, 5,
				0, 1, 5,

				//Left
				0, 2, 6,
				0, 4, 6,

				//Right
				1, 3, 7,
				1, 5, 7,

				//Front
				0, 2, 3,
				0, 1, 3,

				//Back
				4, 6, 7,
				4, 5, 7
			};

			static inline std::vector<glm::vec3> vertices{
			{	-1, -1,  1, },
			{	 1, -1,  1, },
			{	-1,  1,  1, },
			{	 1,  1,  1, },
			{	-1, -1, -1, },
			{	 1, -1, -1, },
			{	-1,  1, -1, },
			{	 1,  1, -1, },
			};
		};
	};

private:
	static void render_meshes(peetcs::archetype_pool& pool);
	static void render_unlit_material_meshes(peetcs::archetype_pool& pool, const camera_data& camera,
	                                         const transform_data& camera_transform);
	static void render_misc_material_meshes(peetcs::archetype_pool& pool, const camera_data& camera,
		const transform_data& camera_transform);
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

template <>
struct std::hash<pipo::render_target_id>
{
	std::size_t operator()(const pipo::render_target_id& id) const noexcept;
};


/*#define DECLARE_TYPE_ID(type, idval) \
	struct type { \
		static constexpr HASH_VALUE_TYPE id() { return static_cast<HASH_VALUE_TYPE>(idval); } \
	};*/

/*DECLARE_TYPE_ID(pipo::transform_data, 022)
DECLARE_TYPE_ID(pipo::mesh_render_data, 1212)
DECLARE_TYPE_ID(pipo::material_data, 2112)
DECLARE_TYPE_ID(pipo::unlit_material_data, 312)
DECLARE_TYPE_ID(pipo::lit_material_data, 412)
DECLARE_TYPE_ID(pipo::camera_data, 5112)*/


