#include "codebuffer.hpp"
#include <cstring>
#include <stdexcept>

void CodeBuffer::append(const uint8_t* data, size_t size) {
    buffer.insert(buffer.end(), data, data + size);
}

void CodeBuffer::append_vec(const std::vector<uint8_t>& v) {
    if (!v.empty()) buffer.insert(buffer.end(), v.begin(), v.end());
}

void CodeBuffer::push(uint8_t b) {
    buffer.push_back(b);
}

void CodeBuffer::write32_le(int32_t v) {
    uint8_t tmp[4];
    tmp[0] = (uint8_t)(v & 0xff);
    tmp[1] = (uint8_t)((v >> 8) & 0xff);
    tmp[2] = (uint8_t)((v >> 16) & 0xff);
    tmp[3] = (uint8_t)((v >> 24) & 0xff);
    append(tmp, 4);
}

void CodeBuffer::write64_le(int64_t v) {
    uint8_t tmp[8];
    tmp[0] = (uint8_t)(v & 0xff);
    tmp[1] = (uint8_t)((v >> 8) & 0xff);
    tmp[2] = (uint8_t)((v >> 16) & 0xff);
    tmp[3] = (uint8_t)((v >> 24) & 0xff);
    tmp[4] = (uint8_t)((v >> 32) & 0xff);
    tmp[5] = (uint8_t)((v >> 40) & 0xff);
    tmp[6] = (uint8_t)((v >> 48) & 0xff);
    tmp[7] = (uint8_t)((v >> 56) & 0xff);
    append(tmp, 8);
}

void CodeBuffer::patch8(size_t offset, uint8_t v) {
    if (offset >= buffer.size()) throw std::out_of_range("patch8 offset");
    buffer[offset] = v;
}

void CodeBuffer::patch32(size_t offset, int32_t v) {
    if (offset + 4 > buffer.size()) throw std::out_of_range("patch32 offset");
    // little endian
    buffer[offset + 0] = (uint8_t)(v & 0xff);
    buffer[offset + 1] = (uint8_t)((v >> 8) & 0xff);
    buffer[offset + 2] = (uint8_t)((v >> 16) & 0xff);
    buffer[offset + 3] = (uint8_t)((v >> 24) & 0xff);
}

void CodeBuffer::patch64(size_t offset, int64_t v) {
    if (offset + 8 > buffer.size()) throw std::out_of_range("patch64 offset");
    buffer[offset + 0] = (uint8_t)(v & 0xff);
    buffer[offset + 1] = (uint8_t)((v >> 8) & 0xff);
    buffer[offset + 2] = (uint8_t)((v >> 16) & 0xff);
    buffer[offset + 3] = (uint8_t)((v >> 24) & 0xff);
    buffer[offset + 4] = (uint8_t)((v >> 32) & 0xff);
    buffer[offset + 5] = (uint8_t)((v >> 40) & 0xff);
    buffer[offset + 6] = (uint8_t)((v >> 48) & 0xff);
    buffer[offset + 7] = (uint8_t)((v >> 56) & 0xff);
}

uint8_t* CodeBuffer::data() {
    return buffer.data();
}

const uint8_t* CodeBuffer::data() const {
    return buffer.data();
}

size_t CodeBuffer::size() const {
    return buffer.size();
}
