#pragma once
#include <chrono>

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