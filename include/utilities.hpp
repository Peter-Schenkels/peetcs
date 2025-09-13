#pragma once
#include <chrono>
#include <string>

#include <nlohmann/json.hpp>
#include <type_traits>

struct time_info
{
	using clock = std::chrono::high_resolution_clock;
	using time_point = std::chrono::time_point<clock>;

	time_point last_tick_time;
	double delta_time; // Time in seconds since the last tick
	std::chrono::duration<double> delta_time_chrono; // Time in seconds since the last tick

	// Constructor initializes the tracker
	time_info() :
		last_tick_time(clock::now()),
		delta_time(0.0),
		delta_time_chrono()
	{
	}

	// Tick function updates delta_time and the last_tick_time
	void tick()
	{
		time_point current_time = clock::now();
		delta_time_chrono = std::chrono::duration<double>(current_time - last_tick_time);
		delta_time = delta_time_chrono.count();
		last_tick_time = current_time;
	}

	// Get the current delta time
	double get_delta_time() const
	{
		return delta_time;
	}

	double get_now() const
	{
		return std::chrono::duration<double>(last_tick_time.time_since_epoch()).count();
	}

	std::chrono::duration<double> get_delta_time_chrono_now()
	{
		time_point current_time = clock::now();
		return std::chrono::duration<double>(current_time - last_tick_time);
	}
};



template<typename T>
static void serialize_data(std::stringstream& stream, const T& data)
{
	
}

template<typename T>
static void deserialize_data(std::stringstream& stream, const T& data)
{

}

#define NAME_PAR(object, member) #member, object.member
using json = nlohmann::json;

template<typename Arg, typename...Args>
void to_json_value(json& out, Arg& arg, Args&... args)
{
	if constexpr (requires { out = arg; })
	{
		out = arg;
	}

	if constexpr (sizeof ...(Args) == 0)
	{
		return;
	}
}

template<typename Arg, typename...Args>
void to_json(json& out, Arg& arg, Args&... args)
{
	if constexpr (sizeof ...(Args) == 0)
	{
		return;
	}
	else
	{
		if constexpr (std::is_constructible_v<std::string, Arg>)
		{
			to_json_value(out[arg], args...);
		}

		to_json(out, args...);
	}
}

template<typename Arg, typename...Args>
void from_json_value(json& in, Arg& arg, Args&... args)
{
	if (in.is_array())
	{
		int i = 0;
		for (auto element : in)
		{
			if constexpr (requires { arg[i] = element; })
			{
				arg[i] = element;
			}

			i++;
		}
	}



	else if constexpr (requires { arg = in.get_ptr<Arg>(); })
	{
		arg = in.get_ptr<Arg>();
	}

	if constexpr (sizeof ...(Args) == 0)
	{
		return;
	}
}


template<typename Arg, typename...Args>
void from_json(json& in, Arg& arg, Args&... args)
{
	if constexpr (sizeof ...(Args) == 0)
	{
		return;
	}
	else
	{
		if constexpr (std::is_constructible_v<std::string, Arg>)
		{
			from_json_value(in[arg], args...);
		}

		from_json(in, args...);
	}
}
