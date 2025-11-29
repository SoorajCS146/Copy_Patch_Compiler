#include "stencil.hpp"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>
using namespace std;
namespace fs = std::filesystem;

StencilLibrary::StencilLibrary(const string& dir)
    : directory(dir) {}

void StencilLibrary::load_all() {
    if (!fs::exists(directory)) {
        throw runtime_error("Stencil directory not found: " + directory);
    }

    for (auto& entry : fs::directory_iterator(directory)) {
        auto ext = entry.path().extension().string();

        // Accepting .bin first priority, fallback to .o else.
        
        if (ext == ".bin") {
        // if (ext == ".bin" || ext == ".o") {
        // if (entry.path().extension() == ".o") {
            string fname = entry.path().stem().string();
            string fullpath = entry.path().string();
            
            Stencil st = load_object_file(fullpath);
            st.name = fname;
            
            stencils[fname] = st;

            cout << "Loaded stencil: " << fname 
                 << " (" << st.bytes.size() << " bytes)\n";
        }
    }
}

Stencil StencilLibrary::load_object_file(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) {
        throw runtime_error("Failed to open stencil file: " + path);
    }

    vector<uint8_t> bytes(
        (istreambuf_iterator<char>(f)),
        istreambuf_iterator<char>()
    );

    Stencil st;
    st.bytes = move(bytes);
    return st;
}

const Stencil& StencilLibrary::get(const string& name) const {
    auto it = stencils.find(name);
    if (it == stencils.end()) {
        throw runtime_error("Stencil not found: " + name);
    }
    return it->second;
}
