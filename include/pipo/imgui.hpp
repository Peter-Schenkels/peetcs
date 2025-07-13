#pragma once

#include <algorithm>

#include "imgui.h"
#include "include/archetype_pool.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "include/utilities.hpp"

class gui_interface
{
public:
	virtual ~gui_interface() = default;
	virtual void draw(peetcs::archetype_pool& pool) = 0;
};



class performance_profiler final : public gui_interface
{
	char text_buffer[64];
	time_info time;

public:
	void draw(peetcs::archetype_pool& pool) override;
};

template<class ... T>
class dll_reloader final : public gui_interface
{
	static bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}


public:

	template<class DLL, class ... OTHERS>
	static void draw_dll()
	{
		std::string load_header = "Load DLL: ";
		std::string release_header = "Release DLL: ";
		std::string name = typeid(DLL).name();
		replace(name, "struct ", "");

		std::string load_text = load_header + name;
		std::string release_text = release_header + name;

		if (!DLL::loaded)
		{
			if (ImGui::Button(load_text.c_str()))
			{
				DLL::load_dll();
			}
		}
		else
		{
			if (ImGui::Button(release_text.c_str()))
			{
				DLL::release_dll();
			}
		}

		if constexpr (sizeof ...(OTHERS) != 0)
		{
			draw_dll<OTHERS...>();
		}
	}

	void inline draw(peetcs::archetype_pool& pool) override
	{
		ImGui::Begin("Hot reload");

		draw_dll<T...>();

		ImGui::End();
	}
};


inline void draw_json_imgui(json& j, const std::string& key)
{
	if (j.is_object()) 
	{
		if (ImGui::TreeNode(key.c_str())) 
		{
			for (auto& el : j.items()) 
			{
				draw_json_imgui(el.value(), el.key());
			}
			ImGui::TreePop();
		}
	}
	else if (j.is_array()) {
		if (ImGui::TreeNode((key + " [array]").c_str())) 
		{
			for (size_t i = 0; i < j.size(); ++i) 
			{
				draw_json_imgui(j[i], "[" + std::to_string(i) + "]");
			}
			ImGui::TreePop();
		}
	}
	else 
	{
		std::string value_str;

		if (j.is_string()) value_str = j.get<std::string>();
		else if (j.is_boolean()) value_str = j.get<bool>() ? "true" : "false";
		else if (j.is_number_float()) value_str = std::to_string(j.get<double>());
		else if (j.is_number_integer()) value_str = std::to_string(j.get<int>());
		else if (j.is_number_unsigned()) value_str = std::to_string(j.get<unsigned>());
		else if (j.is_null()) value_str = "null";
		else value_str = "<unknown>";

		ImGui::Text("%s: %s", key.c_str(), value_str.c_str());
	}
}


inline bool draw_json_imgui_editable(nlohmann::json& j, const std::string& key = "root")
{
	using json = nlohmann::json;

	bool changed = false;

	if (j.is_object()) 
	{
		if (ImGui::TreeNode(key.c_str())) {
			for (auto& el : j.items()) {
				changed |= draw_json_imgui_editable(el.value(), el.key());
			}
			ImGui::TreePop();
		}
	}
	else if (j.is_array()) 
	{
		if (ImGui::TreeNode((key + " [array]").c_str())) {
			for (size_t i = 0; i < j.size(); ++i) {
				changed |= draw_json_imgui_editable(j[i], "[" + std::to_string(i) + "]");
			}
			ImGui::TreePop();
		}
	}
	else if (j.is_string()) 
	{
		std::string buffer = j.get<std::string>();
		char temp[256];
		strncpy(temp, buffer.c_str(), sizeof(temp));
		temp[sizeof(temp) - 1] = 0;

		if (ImGui::InputText(key.c_str(), temp, sizeof(temp))) 
		{
			j = std::string(temp);
			changed = true;
		}
	}
	else if (j.is_boolean()) 
	{
		bool value = j.get<bool>();
		if (ImGui::Checkbox(key.c_str(), &value)) 
		{
			j = value;
			changed = true;
		}
	}
	else if (j.is_number_float()) 
	{
		float value = static_cast<float>(j.get<double>());
		if (ImGui::InputFloat(key.c_str(), &value)) 
		{
			j = static_cast<double>(value);
			changed = true;
		}
	}
	else if (j.is_number_integer()) 
	{
		int value = j.get<int>();
		if (ImGui::InputInt(key.c_str(), &value)) 
		{
			j = value;
			changed = true;
		}
	}
	else if (j.is_number_unsigned()) 
	{
		unsigned int value = j.get<unsigned int>();
		if (ImGui::InputScalar(key.c_str(), ImGuiDataType_U32, &value)) 
		{
			j = value;
			changed = true;
		}
	}
	else if (j.is_null()) 
	{
		ImGui::Text("%s: null", key.c_str());
	}
	else 
	{
		ImGui::Text("%s: <unknown type>", key.c_str());
	}

	return changed;
}


template<typename Arg, typename... Args>
bool draw_imgui_val_editable(const char* key, Arg& arg, Args&... args)
{
	bool changed = false;
	if constexpr (std::is_array_v<Arg>)
	{
		if constexpr (std::extent_v<Arg> <= 4 && std::is_same_v<std::remove_extent_t<Arg>, float>)
		{
			if constexpr (std::extent_v<Arg> == 4) { changed |= ImGui::InputFloat4(key, arg); }
			else if constexpr (std::extent_v<Arg> == 3) { changed |= ImGui::InputFloat3(key, arg); }
			else if constexpr (std::extent_v<Arg> == 2) { changed |= ImGui::InputFloat2(key, arg); }
			else if constexpr (std::extent_v<Arg> == 1) { changed |= ImGui::InputFloat(key, arg); }
		}
		else if (ImGui::TreeNode(key))
		{
			for (size_t i = 0; i < std::extent_v<Arg>; ++i) 
			{
				std::string name_str = {};
				if constexpr (std::is_same_v< std::remove_extent_t<Arg>, float>)
				{
					if constexpr (std::extent_v<Arg> > 3)
					{
						char denom = char('x' + i + ( 3 - std::extent_v<Arg>));
						name_str += denom;
					}
					else
					{
						char denom = char('x' + i);
						name_str += denom;
					}
				}
				else
				{
					name_str = { "[" + std::to_string(i) + "]" };
				}
				const char* name = name_str.c_str();
				changed |= draw_imgui_val_editable(name, arg[i]);
			}
			ImGui::TreePop();
		}
	}
	else if constexpr (std::is_same_v<Arg, bool>)
	{
		changed |= ImGui::Checkbox(key, &arg);
	}
	else if constexpr (std::is_same_v<Arg, float>)
	{
		changed |= ImGui::InputFloat(key, &arg);
	}
	else if constexpr (std::is_same_v<Arg, int>)
	{
		changed |= ImGui::InputInt(key, &arg);
	}
	else if constexpr (std::is_same_v<Arg, unsigned>)
	{
		changed |= ImGui::InputScalar(key, ImGuiDataType_U32, &arg);
	}
	else
	{
		ImGui::Text("%s: <unknown type>", key);
	}

	if constexpr (sizeof ...(Args) == 0)
	{
		return changed;
	}
	else
	{
		return draw_imgui_val_editable(args...);
	}
}



template<typename Arg, typename... Args>
static bool draw_imgui_editable(const char* key, Arg& arg, Args&... args)
{
	bool changed = false;
	if (ImGui::TreeNode(key)) 
	{
		if constexpr (std::is_constructible_v<std::string, Arg>)
		{
			changed |= draw_imgui_val_editable(arg, args...);
		}

		ImGui::TreePop();
	}



	return changed;
}
