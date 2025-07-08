
#include "include/pipo/rasterizer.hpp"

#include "stb_image.h" // TODO move to a seperate header

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include "OBJ_Loader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "include/pipo/imgui.hpp"

#include <iostream>
#include <sstream>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <utility>


pipo::shader_id pipo::unlit_material_data::program = {};

/*std::unordered_map<pipo::texture_id, pipo::texture>               pipo::resources::textures = {};
std::unordered_map<pipo::mesh_id, pipo::mesh>                     pipo::resources::meshes = {};
std::unordered_map<pipo::shader_id, pipo::shader>                 pipo::resources::shaders = {};
std::unordered_map<pipo::render_target_id, pipo::render_target>   pipo::resources::render_targets = {};
void*pipo::resources::window = {};
int   pipo::resources::height = {};
int   pipo::resources::width = {};
pipo::shader_id pipo::unlit_material_data::program = -1;
pipo::mesh_id pipo::resources::quad_mesh = {};
pipo::shader_id pipo::resources::render_texture_shader = -1;

std::vector<pipo::debug::debug_draw_call> pipo::debug::queued_draw_calls = {};*/
pipo::shader_id pipo::debug::debug_shader_id = -1;


char vertex_shader_unlit_code[] =
R"(	#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	Normal =  mat3(transpose(inverse(model))) * aNormal;  ;
    FragPos = vec3(model * vec4(aPos, 1.0));
})";

char fragment_shader_unlit_code[] = R"(	
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;  
uniform sampler2D main_texture;

float lerp(float v0, float v1, float t) {
  return v0 + t * (v1 - v0);
}



void main()
{
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(vec3(-1,-1,0));  
    float diff = max(dot(norm, lightDir), 0.0);
	float intensity = lerp(0.0, 1, diff);
	vec3 color = vec3(texture(main_texture, TexCoord));
	vec3 ambient_color = color * vec3(94f/255f, 156f/255f, 1);
	
    FragColor = vec4(lerp(color.r, ambient_color.r, intensity), lerp(color.g, ambient_color.g, intensity), lerp(color.b, ambient_color.b, intensity), 1);

} )";


char vertex_shader_render_texture[] = R"(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord; 

void main(){
    gl_Position = vec4(aPos, 1.0);
	TexCoord = aTexCoord;

}
)";

char fragment_shader_render_texture[] = R"(
#version 330 core

in vec2 TexCoord;
out vec4 color;

uniform sampler2D renderedTexture;

void main(){
    color = texture(renderedTexture, TexCoord);
	
}
)";

char vertex_shader_debug[] = R"(#version 330 core

layout (location = 0) in vec4 aPos;

void main(){
    gl_Position = aPos; 
}
)";

char fragment_shader_debug[] = R"(
#version 330 core

uniform vec3 aColor;
out vec4 color;


void main(){
    color = vec4(aColor, 1);
	
}
)";


bool pipo::mesh_id::operator==(const mesh_id& other) const
{
    return element_buffer_object == other.element_buffer_object &&
        vertex_array_object == other.vertex_array_object &&
        vertex_buffer_object == other.vertex_buffer_object;
}

bool pipo::render_target_id::operator==(const render_target_id& other) const
{
    return other.depth_buffer_id == depth_buffer_id &&
        other.frame_buffer_id == frame_buffer_id &&
        other.render_texture_id == render_texture_id;
}


void pipo::init_default_resources()
{
    stbi_set_flip_vertically_on_load(true);

    // Compile unlit shader
    {
        pipo::shader::allocate_settings shader_settings = {};
        shader_settings.vertex_shader_code = vertex_shader_unlit_code;
        shader_settings.fragment_shader_code = fragment_shader_unlit_code;
        pipo::unlit_material_data::program = pipo::create_shader_gpu(shader_settings);
    }

    // Create render texture display shader
    {
        pipo::shader::allocate_settings shader_settings = {};
        shader_settings.vertex_shader_code = vertex_shader_render_texture;
        shader_settings.fragment_shader_code = fragment_shader_render_texture;
		_resources.render_texture_shader = pipo::create_shader_gpu(shader_settings);
    }

    // Compile debug shader
    {
        pipo::shader::allocate_settings shader_settings = {};
        shader_settings.vertex_shader_code = vertex_shader_debug;
        shader_settings.fragment_shader_code = fragment_shader_debug;
		debug::debug_shader_id = pipo::create_shader_gpu(shader_settings);
    }

    static float vertices[] = {
        // x,     y,     z,     u,    v
        -1.f,  1.f,  0.0f,  0.0f, 1.0f,  // v0 - top-left
         1.f,  1.f,  0.0f,  1.0f, 1.0f,  // v1 - top-right
         1.f, -1.f,  0.0f,  1.0f, 0.0f,  // v2 - bottom-right
        -1.f, -1.f,  0.0f,  0.0f, 0.0f   // v3 - bottom-left
    };

    static unsigned int indices[] = {
        0, 3, 1,   // Triangle 1
        1, 3, 2    // Triangle 2
    };
    mesh::allocate_settings settings;
    settings.layout = mesh::vertex_3d;
    settings.indices = (unsigned char*)indices;
    settings.vertices = (unsigned char*)vertices;
    settings.nb_of_indices = 6;
    settings.nb_of_vertices = 4;

    _resources.quad_mesh = allocate_mesh_gpu(settings);


    {
        mesh::load_settings settings;
        char filepath[] = "Assets//Models//quad.obj";
        settings.file_path = filepath;

        std::vector<pipo::mesh_id> meshes;
		load_mesh_gpu(settings, meshes);

        _resources.plane_mesh = meshes.front();
    }

	{
        mesh::load_settings settings;
        char filepath[] = "Assets//Models//cube.obj";
        settings.file_path = filepath;

        std::vector<pipo::mesh_id> meshes;
		load_mesh_gpu(settings, meshes);

        _resources.cube_mesh = meshes.front();
    }

	{
        mesh::load_settings settings;
        char filepath[] = "Assets//Models//sphere.obj";
        settings.file_path = filepath;

        std::vector<pipo::mesh_id> meshes;
		load_mesh_gpu(settings, meshes);

        _resources.sphere_mesh = meshes.front();
    }

    // Set GL render state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

bool pipo::init()
{
    _resources.textures = {};
    _resources.meshes = {};
    _resources.shaders = {};

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }


	return true;
}

void pipo::deinit()
{
    glfwDestroyWindow((GLFWwindow*)_resources.window);
    glfwTerminate();
}

bool pipo::create_window(int width, int height, const char* title)
{
    _resources.window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!_resources.window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    _resources.width = width;
    _resources.height = height;

    glfwMakeContextCurrent((GLFWwindow*)_resources.window);
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();

    // Setup ImGui backends
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)_resources.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //glfwSwapInterval(0);
    glClearColor(94/255.f, 156 / 255.f, 255 / 255.f, 1);

    return true;
}

bool pipo::set_window_size(int width, int height)
{
    _resources.width = width;
    _resources.height = height;

    glfwSetWindowSize((GLFWwindow*)_resources.window, width, height);

    return false;
}

bool pipo::bind_render_target(const render_target_id id)
{
    if (auto result = std::find_if(_resources.render_targets.begin(), _resources.render_targets.end(), [id] (auto& a){ return a.id == id;}); result != _resources.render_targets.end())
    {
        const render_target& render_target_to_bind = *result;
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, render_target_to_bind.id.frame_buffer_id));
        GL_CALL(glViewport(0, 0, render_target_to_bind.width, render_target_to_bind.height));

        return true;
    }

    return false;
}

void pipo::unbind_render_target()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CALL(glViewport(0, 0, _resources.width, _resources.height));
}

glm::mat4 get_view(const pipo::transform_data& transform)
{
    // Construct camera world matrix (T * R * S)
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform.get_pos());
    glm::mat4 rotation_mat = glm::toMat4(transform.get_rotation());

    glm::mat4 world_matrix = translation * rotation_mat;

    // View matrix is the inverse of world transform
    glm::mat4 view_matrix = glm::inverse(world_matrix);

    return view_matrix;
}

glm::mat4 get_projection(const pipo::camera_data& camera_data)
{
	switch (camera_data.type)
	{
	case pipo::view_type::perspective:
        return glm::perspective(glm::radians(camera_data.fov), camera_data.aspect, camera_data.c_near, camera_data.c_far);
		break;
	case pipo::view_type::orthographic:
        return glm::ortho(
            -1 * camera_data.fov * 1.f / camera_data.aspect,
            1 * camera_data.fov * 1.f / camera_data.aspect,
            camera_data.fov * camera_data.aspect * -1.f,
            camera_data.fov * camera_data.aspect);
	}
    return {};
}

glm::mat4 get_model(const pipo::transform_data& transform)
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform.get_pos());
    glm::mat4 rotation_mat = glm::toMat4(transform.get_rotation());
    glm::mat4 scale_mat = glm::scale(glm::mat4(1.0f), transform.get_scale());

    glm::mat4 model_matrix = translation * rotation_mat * scale_mat;

    return model_matrix;
}

void pipo::render_imgui(peetcs::archetype_pool& pool, std::vector<std::shared_ptr<gui_interface>> guis)
{
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

	// GUI Content
    for (const auto& gui : guis)
    {
        gui->draw(pool);
    }

	// Render
	ImGui::Render();
}

bool pipo::start_frame()
{
    glfwSwapBuffers((GLFWwindow*)_resources.window);
    glfwPollEvents();

    if (!glfwWindowShouldClose((GLFWwindow*)_resources.window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        return true;
    }

    return false;
}

void pipo::render_misc_material_meshes(peetcs::archetype_pool& pool, const pipo::camera_data& camera, const pipo::transform_data& camera_transform)
{
	glm::mat4 view = get_view(camera_transform);
	glm::mat4 projection = get_projection(camera);
	bool camera_uniforms_set = false;

	auto query = pool.query<material_data, transform_data, mesh_renderer_data>();
	for (auto query_value : query)
	{
		const auto& mesh_renderer = query_value.get<mesh_renderer_data>();
		if (!mesh_renderer.visible)
		{
			continue;
		}

		const auto& material = query_value.get<material_data>();
		GL_CALL(glUseProgram(pipo::unlit_material_data::program));
		int loc_model = glGetUniformLocation(material.program, "model");

		if (!camera_uniforms_set)
		if (!camera_uniforms_set)
		{
			int loc_view = glGetUniformLocation(material.program, "view");
			int loc_proj = glGetUniformLocation(material.program, "projection");

			GL_CALL(glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(view)));
			GL_CALL(glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(projection)));

			camera_uniforms_set = true;
		}

		const transform_data& transform = query_value.get<transform_data>();
        glm::mat4 model = transform.get_world_space(pool);
		GL_CALL(glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(model)));


		GL_CALL(glBindVertexArray(mesh_renderer.mesh_id.vertex_array_object));
		glDrawElements(GL_TRIANGLES, mesh_renderer.mesh_id.nb_of_indices, GL_UNSIGNED_INT, 0);
		GL_CALL(glBindVertexArray(0));
	}
}

void pipo::render_unlit_material_meshes(peetcs::archetype_pool& pool, const pipo::camera_data& camera, const pipo::transform_data& camera_transform)
{
	glm::mat4 view = get_view(camera_transform);
	glm::mat4 projection = get_projection(camera);
	bool camera_uniforms_set = false;

	auto query = pool.query<unlit_material_data, transform_data, mesh_renderer_data>();

	GL_CALL(glUseProgram(unlit_material_data::program));
	int loc_model = glGetUniformLocation(unlit_material_data::program, "model");
	int loc_view = glGetUniformLocation(unlit_material_data::program, "view");
	int loc_proj = glGetUniformLocation(unlit_material_data::program, "projection");
	int loc_main_texture = glGetUniformLocation(unlit_material_data::program, "main_texture");
	GL_CALL(glUniform1i(loc_main_texture, 0));

	for (auto query_value : query)
	{
		const auto& mesh_renderer = query_value.get<mesh_renderer_data>();
		if (!mesh_renderer.visible)
		{
			continue;
		}

		if (!camera_uniforms_set)
		{
			GL_CALL(glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(view)));
			GL_CALL(glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(projection)));

			camera_uniforms_set = true;
		}

		const auto& material = query_value.get<unlit_material_data>();
		GL_CALL(glActiveTexture(GL_TEXTURE0));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, material.main_texture));


		const transform_data& transform = query_value.get<transform_data>();
		glm::mat4 model = transform.get_world_space(pool);
		GL_CALL(glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(model)));


		GL_CALL(glBindVertexArray(mesh_renderer.mesh_id.vertex_array_object));
		glDrawElements(GL_TRIANGLES, mesh_renderer.mesh_id.nb_of_indices, GL_UNSIGNED_INT, 0);
		GL_CALL(glBindVertexArray(0));
	}
}


void pipo::render_meshes(peetcs::archetype_pool& pool)
{
    auto camera_query = pool.query<camera_data, transform_data>();
    for (auto camera_value : camera_query)
    {
        const camera_data& camera = camera_value.get<camera_data>();

        if (!camera.active)
        {
            return;
        }

        if (camera.render_target == render_target_id{})
        {
            pipo::unbind_render_target();
        }
        else
        {
            bind_render_target(camera.render_target);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        const transform_data& camera_transform = camera_value.get<transform_data>();

        render_misc_material_meshes(pool, camera, camera_transform);
        render_unlit_material_meshes(pool, camera, camera_transform);
        render_debug_draw_calls(camera, camera_transform);
    }

    _debug.clear_draw_calls();
    pipo::unbind_render_target();
}

void pipo::render_frame(peetcs::archetype_pool& pool)
{
    render_meshes(pool);

    // Render textures to screen
    GL_CALL(glBindVertexArray(_resources.quad_mesh.vertex_array_object));
    GL_CALL(glUseProgram(_resources.render_texture_shader));

    auto render_target_query = pool.query<pipo::render_target_renderer_data>();
    for (auto render_target_value : render_target_query)
    {
        render_target_renderer_data& render_target_data = render_target_value.get<render_target_renderer_data>();
        GL_CALL(glViewport(render_target_data.x, render_target_data.y, render_target_data.width, render_target_data.height));

        int loc_main_texture = glGetUniformLocation(unlit_material_data::program, "renderedTexture");
        GL_CALL(glUniform1i(loc_main_texture, 0));

        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, render_target_data.target_id.render_texture_id));

        glDrawElements(GL_TRIANGLES, _resources.quad_mesh.nb_of_indices, GL_UNSIGNED_INT, 0);
        GL_CALL(glBindVertexArray(0));
    }
    GL_CALL(glViewport(0, 0, _resources.width, _resources.height));

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


glm::vec3 pipo::transform_data::get_euler_rotation() const
{
    return glm::eulerAngles(get_rotation());
}

void pipo::transform_data::set_pos(glm::vec3 pos)
{
    position[0] = pos.x;
    position[1] = pos.y;
    position[2] = pos.z;
}

void pipo::transform_data::set_rotation(const glm::quat& quat)
{
    rotation[0] = quat.w;
    rotation[1] = quat.x;
    rotation[2] = quat.y;
    rotation[3] = quat.z;
}

glm::mat4 pipo::transform_data::get_local_space() const
{
    return get_model(*this);
}

glm::mat4 pipo::transform_data::get_world_space(peetcs::archetype_pool& pool) const
{
    if (parent == std::numeric_limits<peetcs::entity_id>::max())
    {
        return get_local_space();
    }

    transform_data* parent_transform = pool.get_from_owner<transform_data>(parent);
    if (!parent_transform) // safety check
    {
        return get_local_space(); // fallback: act as root if parent is invalid
    }

    glm::mat4 parent_ws_matrix = parent_transform->get_world_space(pool);

    return parent_ws_matrix * get_local_space();
}


void pipo::draw_line_gizmo(const glm::vec3 a, const glm::vec3 b, const glm::vec3 color)
{
    debug::draw_call draw_call = {};

    draw_call.color = color;
    draw_call.vertices = { glm::vec4(a, 1.0f), glm::vec4(b, 1.0f)};
    draw_call.indices = { 0, 1, 0 };
    draw_call.model_matrix = glm::mat4(1.0);

    _debug.queued_draw_calls.push_back(draw_call);
}


void replace(void* calls)
{

}

void pipo::draw_vertices(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, glm::vec3 color)
{
    debug::draw_call draw_call = {};

    draw_call.color = color;
    draw_call.vertices.reserve(vertices.size());

    for (auto& vertex : vertices)
    {
        draw_call.vertices.emplace_back(vertex, 1.0f);
    }

    draw_call.indices = indices;
    draw_call.model_matrix = glm::mat4(1);
    _debug.queued_draw_calls.push_back(draw_call);
}

void pipo::draw_aabb(const glm::vec3& min, glm::vec3& max, glm::vec3 color)
{
    std::vector<glm::vec3> vertices = {
        { min.x, min.y, min.z }, // v0
        { max.x, min.y, min.z }, // v1
        { max.x, max.y, min.z }, // v2
        { min.x, max.y, min.z }, // v3
        { min.x, min.y, max.z }, // v4
        { max.x, min.y, max.z }, // v5
        { max.x, max.y, max.z }, // v6
        { min.x, max.y, max.z }  // v7
    };

    std::vector<unsigned int> indices = {
        // Front face (z = max)
        4, 5, 6,
        6, 7, 4,

        // Back face (z = min)
        0, 3, 2,
        2, 1, 0,

        // Left face (x = min)
        0, 4, 7,
        7, 3, 0,

        // Right face (x = max)
        1, 2, 6,
        6, 5, 1,

        // Top face (y = max)
        3, 7, 6,
        6, 2, 3,

        // Bottom face (y = min)
        0, 1, 5,
        5, 4, 0
    };

    draw_vertices(vertices, indices, color);
}

void pipo::draw_cube_gizmo(glm::vec3 position, glm::vec3 scale, glm::quat rotation, glm::vec3 color)
{
    debug::draw_call draw_call = {};

    draw_call.color = color;

    transform_data transform = {};
    transform.set_pos(position.x, position.y, position.z);
    glm::vec3 euler_rotation = glm::eulerAngles(rotation);
    transform.set_rotation(euler_rotation.x, euler_rotation.y, euler_rotation.z);
    transform.set_scale(scale.x, scale.y, scale.z);
    glm::mat4 model_matrix = get_model(transform);

    draw_call.vertices.reserve(primitives::cube::vertices.size());
    for (auto& vertex : primitives::cube::vertices)
    {
        draw_call.vertices.emplace_back(vertex, 1.0f);
    }
    draw_call.indices = primitives::cube::indices;
    draw_call.model_matrix = model_matrix;
    _debug.queued_draw_calls.push_back(draw_call);
}

void pipo::render_debug_draw_calls(const camera_data& camera, const transform_data& camera_transform)
{
    glm::mat4 view = get_view(camera_transform);
    glm::mat4 projection = get_projection(camera);
    bool camera_uniforms_set = false;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    GL_CALL(glUseProgram(pipo::debug::debug_shader_id));
    int color_uniform = glGetUniformLocation(pipo::debug::debug_shader_id, "aColor");

	for (auto& queued_draw_call : _debug.queued_draw_calls)
	{
        // Transform vertex to camera space
		for (auto& vertex : queued_draw_call.vertices)
		{
            vertex = projection * view * queued_draw_call.model_matrix * vertex;
		}

        mesh::allocate_settings settings;
        settings.layout = mesh::debug;
        settings.indices = (unsigned char*)queued_draw_call.indices.data();
        settings.vertices = (unsigned char*)queued_draw_call.vertices.data();
        settings.nb_of_indices = queued_draw_call.indices.size();
        settings.nb_of_vertices = queued_draw_call.vertices.size();

        mesh_id debug_mesh = allocate_mesh_gpu(settings);

        GL_CALL(glUniform3f(color_uniform, queued_draw_call.color.r, queued_draw_call.color.g, queued_draw_call.color.b));
        GL_CALL(glBindVertexArray(debug_mesh.vertex_array_object));
        glDrawElements(GL_LINES, debug_mesh.nb_of_indices, GL_UNSIGNED_INT, 0);
        GL_CALL(glBindVertexArray(0));

        unload_mesh_gpu(debug_mesh);
	}

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_DEPTH_TEST);
}

void pipo::debug::clear_draw_calls()
{
    queued_draw_calls.clear();
}

pipo::texture_id pipo::load_texture_gpu(const texture::load_settings& settings)
{
    texture::allocate_settings allocate_settings = {};
    allocate_settings.generate_mipmaps = true;
    allocate_settings.clamp_borders = settings.clamp_borders;
    allocate_settings.nearest_neighbour = settings.nearest_neighbour;

    // load texture to cpu
    allocate_settings.data = stbi_load(settings.file_path, &allocate_settings.width, &allocate_settings.height, &allocate_settings.nb_of_channels, 0);

    // Load texture on gpu
    const texture_id id = allocate_texture_gpu(allocate_settings);

    // Deallocate cpu texture
    stbi_image_free(allocate_settings.data);

    return id;
}

pipo::texture_id pipo::allocate_texture_gpu(const texture::allocate_settings& settings)
{
    texture loaded_texture = {};
    glGenTextures(1, &loaded_texture.id);
    glBindTexture(GL_TEXTURE_2D, loaded_texture.id);

    GLenum format;
    switch (settings.nb_of_channels) {
    case 1: format = GL_DEPTH; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default:
        std::cerr << "Unsupported number of channels: " << settings.nb_of_channels << std::endl;
        stbi_image_free(settings.data);
        return 0; // or an invalid texture ID
    }


    // set the texture wrapping/filtering options (on the currently bound texture object)
    if (settings.clamp_borders) 
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    }
    else 
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    }

    if (settings.generate_mipmaps) 
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.nearest_neighbour ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
    }
    else 
    {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.nearest_neighbour ? GL_NEAREST : GL_LINEAR));
    }
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.nearest_neighbour ? GL_NEAREST : GL_LINEAR));


    if (settings.data)
    {
        GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    }

    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, format, settings.width, settings.height, 0, format, GL_UNSIGNED_BYTE, settings.data));

    if (settings.generate_mipmaps) 
    {
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    }

    _resources.textures.try_emplace(loaded_texture.id, loaded_texture);

    return loaded_texture.id;
}

bool pipo::unload_texture_gpu(const texture_id& id)
{
    if (_resources.textures.contains(id)) {
        glDeleteTextures(1, &id);
        _resources.textures.erase(id);
        return true;
    }

    return false;
}

void pipo::load_mesh_gpu(const mesh::load_settings& settings, std::vector<mesh_id>& loaded_meshes)
{
    // TAKEN FROM EXAMPLES https://github.com/Bly7/OBJ-Loader/blob/master/examples/1%20-%20LoadAndPrint/e1_loadandprint.cpp
    
    // Initialize Loader
    objl::Loader loader;

    // Load .obj File
    bool load_out = loader.LoadFile(settings.file_path);

    if (load_out)
    {
        loaded_meshes.reserve(loader.LoadedMeshes.size());

        // Go through each loaded mesh and out its contents
        for (auto& loaded_mesh : loader.LoadedMeshes)
        {
	        mesh::allocate_settings allocate_settings = {};

            allocate_settings.vertices = reinterpret_cast<unsigned char*>(loaded_mesh.Vertices.data());
            auto nb_of_vertices_check = loaded_mesh.Vertices.size();
            if (nb_of_vertices_check > INT32_MAX)
            {
                std::cout << "ERROR: Buffer overflow \"vertex container\" while loading mesh on CPU" << std::endl;
                return;
            }
            allocate_settings.nb_of_vertices = static_cast<int>(nb_of_vertices_check);

            allocate_settings.indices = reinterpret_cast<unsigned char*>(loaded_mesh.Indices.data());
            auto nb_of_indices_check = loaded_mesh.Indices.size();
            if (nb_of_vertices_check > INT32_MAX)
            {
                std::cout << "ERROR: Buffer overflow \"index container\" while loading mesh on CPU" << std::endl;
                return;
            }
            allocate_settings.nb_of_indices = static_cast<int>(nb_of_indices_check);

            // Set default vertex layout to position 3d + normal 3d + uv
            allocate_settings.layout = mesh::vertex_3d_normal;

            loaded_meshes.emplace_back(allocate_mesh_gpu(allocate_settings));
        }

        return;
    }

    std::cout << "ERROR: Failed to load OBJ File" << std::endl;;
}

pipo::mesh_id pipo::allocate_mesh_gpu(const mesh::allocate_settings& settings)
{
    mesh_id id = {};

	int vertex_size = 0;
    switch (settings.layout)
    {
    case mesh::vertex_2d:
        vertex_size = sizeof(float) * 4;
        break;
    case mesh::vertex_2d_normal:
        vertex_size = sizeof(float) * 6;
        break;
    case mesh::vertex_3d:
        vertex_size = sizeof(float) * 5;
        break;
    case mesh::vertex_3d_normal:
        vertex_size = sizeof(float) * 8;
        break;
    case mesh::debug:
        vertex_size = sizeof(float) * 4;
    }

    // Check buffer overflow vertex container
    unsigned long long vertex_container_size_long = settings.nb_of_vertices * vertex_size;
    if (vertex_container_size_long > INT32_MAX)
    {
        std::cout << "ERROR: Buffer overflow \"vertex container\" while allocating mesh on GPU" << std::endl;
        return {};
    }
    unsigned int vertex_container_size = static_cast<unsigned int>(vertex_container_size_long);

    // Check buffer overflow index container
    unsigned long long index_container_size_long = settings.nb_of_indices * sizeof(unsigned int);
    if (index_container_size_long > INT32_MAX)
    {
        std::cout << "ERROR: Buffer overflow \"index container\" while allocating mesh on GPU" << std::endl;
        return {};
    }
    unsigned int index_container_size = static_cast<unsigned int>(index_container_size_long);

    id.nb_of_indices = settings.nb_of_indices;

    GL_CALL(glGenVertexArrays(1, &id.vertex_array_object));
    GL_CALL(glGenBuffers(1, &id.vertex_buffer_object));
    GL_CALL(glGenBuffers(1, &id.element_buffer_object));

    GL_CALL(glBindVertexArray(id.vertex_array_object));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id.vertex_buffer_object));

    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertex_container_size, settings.vertices, GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id.element_buffer_object));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_container_size, settings.indices, GL_STATIC_DRAW));

    int index = 0;
    int offset = 0;
    switch (settings.layout)
    {
    case mesh::vertex_2d:
        // vertex positions
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, nullptr));

        index++;
        offset += 2;
        break;
    case mesh::vertex_2d_normal:
        // vertex positions
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, nullptr));

        index++;
        offset += 2;

        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(offset * sizeof(float))));

        index++;
        offset += 2;
        break;
    case mesh::vertex_3d:
        // vertex positions
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, nullptr));

        // Set index and offset for the UV coordinate parameter;
        index++;
        offset += 3;

        break;
    case mesh::vertex_3d_normal:
        // vertex positions
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, nullptr));

        index++;
        offset += 3;

        // vertex normals
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)(offset * sizeof(float))));

        // Set index and offset for the UV coordinate parameter;
        index++;
        offset += 3;

        break;

    case mesh::debug:
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, vertex_size, nullptr));
        break;
    }

    if (settings.layout != mesh::debug)
    {
        // Vertex UV Texture coordinates
        GL_CALL(glEnableVertexAttribArray(index));
        GL_CALL(glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(offset * sizeof(float))));

        GL_CALL(glBindVertexArray(0));
    }

    mesh allocated_mesh = {};
    allocated_mesh.id = id;
    allocated_mesh.layout = settings.layout;

    _resources.meshes.push_back(allocated_mesh);
    return allocated_mesh.id;
}

pipo::mesh_id pipo::get_quad() const
{
    return _resources.quad_mesh;
}

pipo::mesh_id pipo::get_plane() const
{
    return _resources.plane_mesh;
}

pipo::mesh_id pipo::get_sphere() const
{
    return _resources.sphere_mesh;
}

pipo::mesh_id pipo::get_cube() const
{
    return _resources.cube_mesh;
}

bool pipo::unload_mesh_gpu(const mesh_id& id)
{
    GL_CALL(glDeleteVertexArrays(1, &id.vertex_array_object));
    GL_CALL(glDeleteBuffers(1, &id.element_buffer_object));
    GL_CALL(glDeleteBuffers(1, &id.vertex_buffer_object));

    erase_if(_resources.meshes, [id](auto& a) { return a.id == id; });

    return true;
}

pipo::shader_id pipo::load_shader_gpu(const shader::load_settings& settings)
{
    shader::allocate_settings allocate_settings = {};

    std::fstream stream;
    stream.open(settings.file_path);
    std::stringstream vertex_shader_code = {};
    std::stringstream fragment_shader_code = {};



    vertex_shader_code << stream.rdbuf();
    allocate_settings.vertex_shader_code = vertex_shader_code.str().data();
    

    return {};
}

pipo::shader_id pipo::create_shader_gpu(const shader::allocate_settings& settings)
{
    shader compiled_shader = {};

    // 2. compile shaders
    int success;
    char infoLog[512];

    // shader Program
    compiled_shader.id = glCreateProgram();

    if (settings.vertex_shader_code != nullptr)
    {
        // vertex Shader
        compiled_shader.vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GL_CALL(glShaderSource(compiled_shader.vertex_shader_id, 1, &settings.vertex_shader_code, NULL));
        GL_CALL(glCompileShader(compiled_shader.vertex_shader_id));
        // print compile errors if any
        glGetShaderiv(compiled_shader.vertex_shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(compiled_shader.vertex_shader_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << '\n';
            __debugbreak();
        }

        GL_CALL(glAttachShader(compiled_shader.id, compiled_shader.vertex_shader_id));
    }

    if (settings.fragment_shader_code != nullptr)
    {
        // Fragment Shader
        compiled_shader.fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        GL_CALL(glShaderSource(compiled_shader.fragment_shader_id, 1, &settings.fragment_shader_code, NULL));
        GL_CALL(glCompileShader(compiled_shader.fragment_shader_id));
        // print compile errors if any
        GL_CALL(glGetShaderiv(compiled_shader.fragment_shader_id, GL_COMPILE_STATUS, &success));
        if (!success)
        {
            GL_CALL(glGetShaderInfoLog(compiled_shader.fragment_shader_id, 512, NULL, infoLog));
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << '\n';
            __debugbreak();
        }

        GL_CALL(glAttachShader(compiled_shader.id, compiled_shader.fragment_shader_id));
    }


    GL_CALL(glLinkProgram(compiled_shader.id));
    // print linking errors if any
    GL_CALL(glGetProgramiv(compiled_shader.id, GL_LINK_STATUS, &success));
    if (!success)
    {
        glGetProgramInfoLog(compiled_shader.id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    GL_CALL(glDeleteShader(compiled_shader.vertex_shader_id));
    GL_CALL(glDeleteShader(compiled_shader.fragment_shader_id));

    return compiled_shader.id;
}

pipo::render_target_id pipo::create_render_target(const render_target::allocate_settings& settings)
{
    render_target_id render_target_id = {};
    glGenFramebuffers(1, &render_target_id.frame_buffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, render_target_id.frame_buffer_id);

    texture::allocate_settings texture_allocate_settings;
    texture_allocate_settings.width = settings.width;
    texture_allocate_settings.height = settings.height;
    texture_allocate_settings.nearest_neighbour = true;
    texture_allocate_settings.clamp_borders = false;

    switch (settings.target_type)
    {
    case render_target::type::color:
        texture_allocate_settings.nb_of_channels = 3;

        // Generate a standard depth buffer for rendering used to render to texture for depth testing
        GL_CALL(glGenRenderbuffers(1, &render_target_id.depth_buffer_id));
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, render_target_id.depth_buffer_id));
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, settings.width, settings.height));
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_target_id.depth_buffer_id));
        break;
    case render_target::type::color_depth:
        std::cout << "ERROR: Color depth not yet supported use the depth buffer instead";
        break;
    case render_target::type::depth:
        texture_allocate_settings.nb_of_channels = 1;

        render_target_id.depth_buffer_id = -1;
        break;
    }

    render_target_id.render_texture_id = pipo::allocate_texture_gpu(texture_allocate_settings);

    GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render_target_id.render_texture_id, 0));

    GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
    GL_CALL(glDrawBuffers(1, draw_buffers));

    
    if (auto error = glCheckFramebufferStatus(GL_FRAMEBUFFER); error != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: failed to allocate FrameBuffer / Rendertarget" << std::endl;
        GL_CALL(error);
    }

    render_target render_target_instanced;
    render_target_instanced.id = render_target_id;
    render_target_instanced.height = settings.height;
    render_target_instanced.width = settings.width;

    // Set Frame buffer back to default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _resources.render_targets.push_back(render_target_instanced);

    return render_target_id;
}

