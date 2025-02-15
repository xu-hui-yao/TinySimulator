#pragma once

#include <filesystem>
#include <vector>

/**
 * \brief Simple class for resolving paths on Linux/Windows/macOS
 *
 * This convenience class looks for a file or directory given its name
 * and a set of search paths. The implementation walks through the
 * search paths in order and stops once the file is found.
 */
class Resolver {
public:
    typedef std::vector<std::filesystem::path>::iterator iterator;
    typedef std::vector<std::filesystem::path>::const_iterator const_iterator;

    /**
     * \brief Constructor
     *
     * Initializes the resolver with the current working directory
     * and the project source directory.
     */
    Resolver();

    /**
     * \brief Get the number of search paths
     *
     * \return The total number of search paths.
     */
    [[nodiscard]] size_t size() const;

    /**
     * \brief Get an iterator to the beginning of the search paths
     *
     * \return An iterator pointing to the first search std::filesystem::path.
     */
    iterator begin();

    /**
     * \brief Get an iterator to the end of the search paths
     *
     * \return An iterator pointing past the last search std::filesystem::path.
     */
    iterator end();

    /**
     * \brief Get a constant iterator to the beginning of the search paths
     *
     * \return A constant iterator pointing to the first search std::filesystem::path.
     */
    [[nodiscard]] const_iterator begin() const;

    /**
     * \brief Get a constant iterator to the end of the search paths
     *
     * \return A constant iterator pointing past the last search std::filesystem::path.
     */
    [[nodiscard]] const_iterator end() const;

    /**
     * \brief Erase a search std::filesystem::path
     *
     * Removes the search std::filesystem::path at the specified iterator.
     *
     * \param it Iterator pointing to the search std::filesystem::path to be removed.
     */
    void erase(const iterator &it);

    /**
     * \brief Prepend a search std::filesystem::path
     *
     * Adds a search std::filesystem::path to the front of the list.
     *
     * \param path The search std::filesystem::path to be added.
     */
    void prepend(const std::filesystem::path &path);

    /**
     * \brief Append a search std::filesystem::path
     *
     * Adds a search std::filesystem::path to the end of the list.
     *
     * \param path The search std::filesystem::path to be added.
     */
    void append(const std::filesystem::path &path);

    /**
     * \brief Access a search std::filesystem::path by index (const)
     *
     * \param index The index of the search std::filesystem::path to access.
     * \return The search std::filesystem::path at the specified index.
     */
    const std::filesystem::path &operator[](size_t index) const;

    /**
     * \brief Access a search std::filesystem::path by index (mutable)
     *
     * \param index The index of the search std::filesystem::path to access.
     * \return The search std::filesystem::path at the specified index.
     */
    std::filesystem::path &operator[](size_t index);

    /**
     * \brief Resolve a std::filesystem::path
     *
     * Looks for the specified file or directory in the search paths.
     * Returns the first std::filesystem::path where the file or directory exists.
     *
     * \param value The file or directory to resolve.
     * \return The resolved std::filesystem::path and whether path exists.
     */
    [[nodiscard]] std::pair<std::filesystem::path, bool> resolve(const std::filesystem::path &value) const;

    /**
     * \brief Output stream operator
     *
     * Outputs the list of search paths to the given stream.
     *
     * \param os The output stream.
     * \param r The resolver instance.
     * \return The output stream with the search paths.
     */
    friend std::ostream &operator<<(std::ostream &os, const Resolver &r);

private:
    std::vector<std::filesystem::path> m_paths; ///< List of search paths
};
