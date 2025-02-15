#include <core/filesystem/resolver.h>
#include <core/fwd.h>

Resolver::Resolver() {
    m_paths.emplace_back(std::filesystem::current_path()); // Add current working directory to search paths
    m_paths.emplace_back(M_PROJECT_SOURCE_DIR);            // Add project source directory to search paths
}

size_t Resolver::size() const { return m_paths.size(); }

Resolver::iterator Resolver::begin() { return m_paths.begin(); }

Resolver::iterator Resolver::end() { return m_paths.end(); }

Resolver::const_iterator Resolver::begin() const { return m_paths.begin(); }

Resolver::const_iterator Resolver::end() const { return m_paths.end(); }

void Resolver::erase(const iterator &it) { m_paths.erase(it); }

void Resolver::prepend(const std::filesystem::path &path) { m_paths.insert(m_paths.begin(), path); }

void Resolver::append(const std::filesystem::path &path) { m_paths.emplace_back(path); }

const std::filesystem::path &Resolver::operator[](size_t index) const { return m_paths[index]; }

std::filesystem::path &Resolver::operator[](size_t index) { return m_paths[index]; }

std::pair<std::filesystem::path, bool> Resolver::resolve(const std::filesystem::path &value) const {
    if (value.is_absolute()) {
        return std::make_pair(value, exists(value));
    }

    bool exist = false;
    std::filesystem::path path;
    for (const auto &dir : m_paths) {
        auto combined = dir / value;
        if (exists(combined)) {
            if (exist) {
                get_logger()->warn("Resolver::resolve() more than one path are valid.");
            }
            path  = combined;
            exist = true;
        }
    }
    return std::make_pair(path, exist);
}

std::ostream &operator<<(std::ostream &os, const Resolver &r) {
    os << "Resolver[" << std::endl;
    for (size_t i = 0; i < r.m_paths.size(); ++i) {
        os << "  \"" << r.m_paths[i] << "\""; // Output each search std::filesystem::path
        if (i + 1 < r.m_paths.size())
            os << ",";
        os << std::endl;
    }
    os << "]";
    return os;
}