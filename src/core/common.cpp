#include <core/common.h>
#include <chrono>
#include <filesystem>

std::chrono::system_clock file_time_to_system_clock(std::chrono::system_clock::time_point file_time) {
    // Compute the difference between the file_time and the current time in the filesystem clock.
    auto diff = file_time - std::filesystem::file_time_type::clock::now();
    
    // Add the difference to system_clock::now() to convert to system_clock.
    return std::chrono::system_clock::now() + diff;
}