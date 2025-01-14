#pragma once

#include <core/filesystem/path.h>

NAMESPACE_BEGIN(filesystem)
/**
 * \brief Simple class for resolving paths on Linux/Windows/macOS
 *
 * This convenience class looks for a file or directory given its name
 * and a set of search paths. The implementation walks through the
 * search paths in order and stops once the file is found.
 */
class resolver {
public:
    typedef std::vector<path>::iterator iterator;             ///< Iterator type for mutable access to search paths
    typedef std::vector<path>::const_iterator const_iterator; ///< Iterator type for read-only access to search paths

    /**
     * \brief Constructor
     *
     * Initializes the resolver with the current working directory
     * and the project source directory.
     */
    resolver() {
        m_paths.emplace_back(path::getcwd());       // Add current working directory to search paths
        m_paths.emplace_back(M_PROJECT_SOURCE_DIR); // Add project source directory to search paths
    }

    /**
     * \brief Get the number of search paths
     *
     * \return The total number of search paths.
     */
    [[nodiscard]] size_t size() const { return m_paths.size(); }

    /**
     * \brief Get an iterator to the beginning of the search paths
     *
     * \return An iterator pointing to the first search path.
     */
    iterator begin() { return m_paths.begin(); }

    /**
     * \brief Get an iterator to the end of the search paths
     *
     * \return An iterator pointing past the last search path.
     */
    iterator end() { return m_paths.end(); }

    /**
     * \brief Get a constant iterator to the beginning of the search paths
     *
     * \return A constant iterator pointing to the first search path.
     */
    [[nodiscard]] const_iterator begin() const { return m_paths.begin(); }

    /**
     * \brief Get a constant iterator to the end of the search paths
     *
     * \return A constant iterator pointing past the last search path.
     */
    [[nodiscard]] const_iterator end() const { return m_paths.end(); }

    /**
     * \brief Erase a search path
     *
     * Removes the search path at the specified iterator.
     *
     * \param it Iterator pointing to the search path to be removed.
     */
    void erase(const iterator &it) { m_paths.erase(it); }

    /**
     * \brief Prepend a search path
     *
     * Adds a search path to the front of the list.
     *
     * \param path The search path to be added.
     */
    void prepend(const path &path) { m_paths.insert(m_paths.begin(), path); }

    /**
     * \brief Append a search path
     *
     * Adds a search path to the end of the list.
     *
     * \param path The search path to be added.
     */
    void append(const path &path) { m_paths.emplace_back(path); }

    /**
     * \brief Access a search path by index (const)
     *
     * \param index The index of the search path to access.
     * \return The search path at the specified index.
     */
    const path &operator[](size_t index) const { return m_paths[index]; }

    /**
     * \brief Access a search path by index (mutable)
     *
     * \param index The index of the search path to access.
     * \return The search path at the specified index.
     */
    path &operator[](size_t index) { return m_paths[index]; }

    /**
     * \brief Resolve a path
     *
     * Looks for the specified file or directory in the search paths.
     * Returns the first path where the file or directory exists.
     *
     * \param value The file or directory to resolve.
     * \return The resolved path, or the original path if not found.
     */
    [[nodiscard]] path resolve(const path &value) const {
        for (const auto &m_path : m_paths) {
            path combined = m_path / value; // Combine current search path with the input path
            if (combined.exists())          // Check if the combined path exists
                return combined;            // Return the resolved path if found
        }
        return value; // Return the original path if not found
    }

    /**
     * \brief Output stream operator
     *
     * Outputs the list of search paths to the given stream.
     *
     * \param os The output stream.
     * \param r The resolver instance.
     * \return The output stream with the search paths.
     */
    friend std::ostream &operator<<(std::ostream &os, const resolver &r) {
        os << "Resolver[" << std::endl;
        for (size_t i = 0; i < r.m_paths.size(); ++i) {
            os << "  \"" << r.m_paths[i] << "\""; // Output each search path
            if (i + 1 < r.m_paths.size())
                os << ",";
            os << std::endl;
        }
        os << "]";
        return os;
    }

private:
    std::vector<path> m_paths; ///< List of search paths
};

NAMESPACE_END(filesystem)
