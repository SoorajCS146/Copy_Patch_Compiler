#ifndef STENCIL_HPP
#define STENCIL_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "codebuffer.hpp"

// Represents a machine-code stencil loaded from a .o file
struct Stencil {
    std::string name;
    std::vector<uint8_t> bytes;
    std::vector<size_t> patchOffsets;
};

// A stencil library loads all .o files from stencils/ directory
class StencilLibrary {
public:
    explicit StencilLibrary(const std::string& dir);

    // Load all .o files (add.o, const.o, print.o, etc.)
    void load_all();

    // Access stencil by name
    const Stencil& get(const std::string& name) const;

private:
    std::string directory;
    std::unordered_map<std::string, Stencil> stencils;

    // Helper to load a single .o file
    Stencil load_object_file(const std::string& path);
};

#endif // STENCIL_HPP
