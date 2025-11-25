#ifndef CODEBUFFER_HPP
#define CODEBUFFER_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

class CodeBuffer {
public:
    CodeBuffer() = default;

    // Append raw bytes (ptr form)
    void append(const uint8_t* data, size_t size);

    // Append bytes (vector form)
    void append_vec(const std::vector<uint8_t>& v);

    // Write a single byte
    void push(uint8_t b);

    // Write little-endian 32-bit and 64-bit values into buffer (append)
    void write32_le(int32_t v);
    void write64_le(int64_t v);

    // Patch an existing location (overwrite) - safe when called with valid offsets
    void patch8(size_t offset, uint8_t v);
    void patch32(size_t offset, int32_t v);
    void patch64(size_t offset, int64_t v);

    // Access raw pointer for patching / copying
    uint8_t* data();
    const uint8_t* data() const;

    // Current size
    size_t size() const;

private:
    std::vector<uint8_t> buffer;
};

#endif // CODEBUFFER_HPP
