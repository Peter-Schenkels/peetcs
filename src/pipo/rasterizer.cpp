
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

std::unordered_map<pipo::texture_id, pipo::texture> pipo::resources::textures = {};
std::unordered_map<pipo::mesh_id, pipo::mesh>       pipo::resources::meshes = {};
std::unordered_map<pipo::shader_id, pipo::shader>   pipo::resources::shaders = {};
void*   pipo::resources::window = {};
pipo::shader_id pipo::unlit_material_data::program = -1;


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

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	TexCoord = aTexCoord;
	Normal = aNormal;
})";

char fragment_shader_unlit_code[] = R"(	
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
uniform sampler2D main_texture;

void main()
{
    FragColor = texture(main_texture, TexCoord);
} )";




void compile_unlit_shader()
{
    pipo::shader::allocate_settings shader_settings = {};
    shader_settings.vertex_shader_code = vertex_shader_unlit_code;
    shader_settings.fragment_shader_code = fragment_shader_unlit_code;
    pipo::shader_id shader = pipo::resources::create_shader_gpu(shader_settings);

    pipo::unlit_material_data::program = shader;
}


void pipo::resources::init_default_resources()
{
    stbi_set_flip_vertically_on_load(true);
    compile_unlit_shader();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

bool pipo::init()
{
    resources::textures = {};
    resources::meshes = {};
    resources::shaders = {};

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }


	return true;
}

void pipo::deinit()
{
    glfwDestroyWindow((GLFWwindow*)pipo::resources::window);
    glfwTerminate();
}

bool pipo::create_window(int width, int height, const char* title)
{
    resources::window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!resources::window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent((GLFWwindow*)resources::window);

    if (glewInit() != GLEW_OK) {      // ✅ Loads OpenGL function pointers
        std::cerr << "GLEW init failed\n";
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // or ImGui::StyleColorsClassic();

    // Setup ImGui backends
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)resources::window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

}

bool pipo::set_window_size(int width, int height, char* title)
{
    return false;
}


bool pipo::create_camera_entity(const peetcs::entity_id entity)
{
    return false;
}

void pipo::render_camera(const camera_data& camera, const render_texture_data& target_texture)
{
    
}

void pipo::render_texture_to_window(const glfw_window_data& window, const render_texture_data& target_texture)
{
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
    glfwSwapBuffers((GLFWwindow*)pipo::resources::window);
    glfwPollEvents();

    if (!glfwWindowShouldClose((GLFWwindow*)pipo::resources::window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        return true;
    }

    return false;
}

void pipo::render_misc_material_meshes(peetcs::archetype_pool& pool)
{
    auto camera_query = pool.query<camera_data, transform_data>();
	for (auto camera_value : camera_query)
	{
        const camera_data& camera = camera_value.get<camera_data>();

        if (!camera.active)
        {
	        return;
        }

        const transform_data& camera_transform = camera_value.get<transform_data>();

        glm::mat4 view = get_view(camera_transform);
        glm::mat4 projection = get_projection(camera);
        bool camera_uniforms_set = false;

        auto query = pool.query<material_data, transform_data, mesh_render_data>();
        for (auto query_value : query)
        {
            const auto& mesh_renderer = query_value.get<mesh_render_data>();
            if (!mesh_renderer.visible)
            {
	            continue;
            }

            const auto& material = query_value.get<material_data>();
            GL_CALL(glUseProgram(pipo::unlit_material_data::program));
            int loc_model = glGetUniformLocation(material.program, "model");

            if (!camera_uniforms_set)
            {
                int loc_view = glGetUniformLocation(material.program, "view");
                int loc_proj = glGetUniformLocation(material.program, "projection");

                GL_CALL(glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(view)));
                GL_CALL(glUniformMatrix4fv(loc_proj, 1, GL_FALSE, glm::value_ptr(projection)));

                camera_uniforms_set = true;
            }

            const transform_data& transform = query_value.get<transform_data>();
            glm::mat4 model = get_model(transform);
            GL_CALL(glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(model)));


            GL_CALL(glBindVertexArray(mesh_renderer.mesh_id.vertex_array_object));
            glDrawElements(GL_TRIANGLES, mesh_renderer.mesh_id.nb_of_indices, GL_UNSIGNED_INT, 0);
            GL_CALL(glBindVertexArray(0));
        }
	}
}


void pipo::render_unlit_material_meshes(peetcs::archetype_pool& pool)
{
    auto camera_query = pool.query<camera_data, transform_data>();
    for (auto camera_value : camera_query)
    {
        const camera_data& camera = camera_value.get<camera_data>();

        if (!camera.active)
        {
            return;
        }

        const transform_data& camera_transform = camera_value.get<transform_data>();

        glm::mat4 view = get_view(camera_transform);
        glm::mat4 projection = get_projection(camera);
        bool camera_uniforms_set = false;

        auto query = pool.query<unlit_material_data, transform_data, mesh_render_data>();

        GL_CALL(glUseProgram(unlit_material_data::program));
        int loc_model = glGetUniformLocation(unlit_material_data::program, "model");
        int loc_view = glGetUniformLocation(unlit_material_data::program, "view");
        int loc_proj = glGetUniformLocation(unlit_material_data::program, "projection");
        int loc_main_texture = glGetUniformLocation(unlit_material_data::program, "main_texture");
        GL_CALL(glUniform1i(loc_main_texture, 0));

        for (auto query_value : query)
        {
            const auto& mesh_renderer = query_value.get<mesh_render_data>();
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
            glm::mat4 model = get_model(transform);
            GL_CALL(glUniformMatrix4fv(loc_model, 1, GL_FALSE, glm::value_ptr(model)));


            GL_CALL(glBindVertexArray(mesh_renderer.mesh_id.vertex_array_object));
            glDrawElements(GL_TRIANGLES, mesh_renderer.mesh_id.nb_of_indices, GL_UNSIGNED_INT, 0);
            GL_CALL(glBindVertexArray(0));
        }
    }
}

void pipo::render_frame(peetcs::archetype_pool& pool)
{
    render_misc_material_meshes(pool);
    render_unlit_material_meshes(pool);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::size_t std::hash<pipo::mesh_id>::operator()(const pipo::mesh_id& id) const noexcept
{
    return ((hash<unsigned int>()(id.element_buffer_object) ^ (hash<unsigned int>()(id.vertex_array_object) << 1)) >> 1)
        ^ (hash<unsigned int>()(id.vertex_buffer_object) << 1);
}


bool pipo::mesh_id::operator==(const mesh_id& other) const
{
    return element_buffer_object == other.element_buffer_object && 
        vertex_array_object == other.vertex_array_object && 
        vertex_buffer_object == other.vertex_buffer_object;
}

pipo::texture_id pipo::resources::load_texture_gpu(const texture::load_settings& settings)
{
    texture::allocate_settings allocate_settings = {};
    allocate_settings.generate_mipmaps = true;
    allocate_settings.clamp_borders = false;

    // load texture to cpu
    allocate_settings.data = stbi_load(settings.file_path, &allocate_settings.width, &allocate_settings.height, &allocate_settings.nb_of_channels, 0);

    // Load texture on gpu
    const texture_id id = allocate_texture_gpu(allocate_settings);

    // Deallocate cpu texture
    stbi_image_free(allocate_settings.data);

    return id;
}

pipo::texture_id pipo::resources::allocate_texture_gpu(const texture::allocate_settings& settings)
{
    texture loaded_texture = {};
    glGenTextures(1, &loaded_texture.id);
    glBindTexture(GL_TEXTURE_2D, loaded_texture.id);

    GLenum format;
    switch (settings.nb_of_channels) {
    case 1: format = GL_RED; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    default:
        std::cerr << "Unsupported number of channels: " << settings.nb_of_channels << std::endl;
        stbi_image_free(settings.data);
        return 0; // or an invalid texture ID
    }


    // set the texture wrapping/filtering options (on the currently bound texture object)
    if (settings.clamp_borders) {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    }

    if (settings.generate_mipmaps) {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    if (settings.data) {
        GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, format, settings.width, settings.height, 0, format, GL_UNSIGNED_BYTE, settings.data));

        if (settings.generate_mipmaps) {
            GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
        }
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }

    textures.try_emplace(loaded_texture.id, loaded_texture);

    return loaded_texture.id;
}

bool pipo::resources::unload_texture_gpu(const texture_id& id)
{
    if (textures.contains(id)) {
        glDeleteTextures(1, &id);
        textures.erase(id);
        return true;
    }

    return false;
}

void pipo::resources::load_mesh_gpu(const mesh::load_settings& settings, std::vector<mesh_id>& loaded_meshes)
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

pipo::mesh_id pipo::resources::allocate_mesh_gpu(const mesh::allocate_settings& settings)
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
    }

    // Vertex UV Texture coordinates
    GL_CALL(glEnableVertexAttribArray(index));
    GL_CALL(glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(offset * sizeof(float))));

    GL_CALL(glBindVertexArray(0));

    mesh allocated_mesh = {};
    allocated_mesh.id = id;
    allocated_mesh.layout = settings.layout;

    meshes.try_emplace(id, allocated_mesh);
    return allocated_mesh.id;
}

bool pipo::resources::unload_mesh_gpu(const mesh_id& id)
{
    GL_CALL(glDeleteVertexArrays(1, &id.vertex_array_object));
    GL_CALL(glDeleteBuffers(1, &id.element_buffer_object));
    GL_CALL(glDeleteBuffers(1, &id.vertex_buffer_object));

    return true;
}

pipo::shader_id pipo::resources::load_shader_gpu(const shader::load_settings& settings)
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

pipo::shader_id pipo::resources::create_shader_gpu(const shader::allocate_settings& settings)
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

    shaders.try_emplace(compiled_shader.id, compiled_shader);

    return compiled_shader.id;
}
