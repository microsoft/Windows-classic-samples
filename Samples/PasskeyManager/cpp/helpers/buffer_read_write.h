#pragma once

#include <span>
#include <vector>
#include <wil/result_macros.h>

static inline void add_buffer_bytes(std::vector<uint8_t>& storage, void const* data, size_t size)
{
    storage.insert(storage.end(), reinterpret_cast<uint8_t const*>(data), reinterpret_cast<uint8_t const*>(data) + size);
}

static inline void add_buffer_bytes(std::vector<uint8_t>& storage, std::span<uint8_t const> data)
{
    storage.insert(storage.end(), data.begin(), data.end());
}

struct buffer_writer
{
    buffer_writer(std::span<UCHAR> fullData) : remaining(fullData)
    {
    }

    std::span<UCHAR> remaining;

    void add(std::span<UCHAR const> content)
    {
        THROW_HR_IF(E_INVALIDARG, content.size() > remaining.size());
        memcpy(remaining.data(), content.data(), content.size());
        remaining = remaining.subspan(content.size());
    }

    template <typename T>
    T* reserve_space()
    {
        THROW_HR_IF(E_INVALIDARG, sizeof(T) > remaining.size());
        auto result = reinterpret_cast<T*>(remaining.data());
        remaining = remaining.subspan(sizeof(T));
        return result;
    }
};

struct buffer_reader
{
    std::span<UCHAR const> data;

    size_t remaining() const
    {
        return data.size();
    }

    std::span<UCHAR const> read(size_t size)
    {
        THROW_HR_IF(E_INVALIDARG, size > data.size());
        auto result = data.first(size);
        data = data.subspan(size);
        return result;
    }

    std::span<UCHAR const> read_remaining()
    {
        return std::exchange(data, {});
    }

    template <typename T>
    T const* read()
    {
        return reinterpret_cast<T const*>(read(sizeof(T)).data());
    }
};
