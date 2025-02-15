#include <scene/resource/resource.h>

Resource::Resource(std::filesystem::path path) : m_path(std::move(path)) {}

[[nodiscard]] std::filesystem::path Resource::get_path() const { return m_path; }