// SPDX-License-Identifier: GPL-2.0-or-later
// Minimal standalone stdio adapter for the vendored shadPS4 PKG parser.
// This avoids bringing the emulator's logging, Qt, and IO framework into PS4RT.
#pragma once

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace Common::FS {

// PKG files are well over 4 GiB. MinGW's off_t is 32-bit unless the whole build uses
// _FILE_OFFSET_BITS=64, so fseeko/ftello wrapped sizes and offsets. Use the 64-bit CRT calls.
#ifdef _WIN32
inline int SeekFile64(std::FILE* f, std::int64_t pos, int origin) { return ::_fseeki64(f, pos, origin); }
inline std::int64_t TellFile64(std::FILE* f) { return ::_ftelli64(f); }
#else
inline int SeekFile64(std::FILE* f, std::int64_t pos, int origin) { return ::fseeko(f, static_cast<off_t>(pos), origin); }
inline std::int64_t TellFile64(std::FILE* f) { return static_cast<std::int64_t>(::ftello(f)); }
#endif

enum class FileAccessMode { Read, Write };
enum class SeekOrigin { SetOrigin, CurrentPosition, End };

class IOFile {
public:
    IOFile() = default;
    explicit IOFile(const std::filesystem::path& path, FileAccessMode mode) {
        Open(path, mode);
    }
    IOFile(const IOFile&) = delete;
    IOFile& operator=(const IOFile&) = delete;
    ~IOFile() { Close(); }

    bool Open(const std::filesystem::path& path, FileAccessMode mode) {
        Close();
        // path::c_str() is wchar_t on Windows and char elsewhere.
#ifdef _WIN32
        stream_ = _wfopen(path.c_str(), mode == FileAccessMode::Read ? L"rb" : L"wb");
#else
        stream_ = std::fopen(path.c_str(), mode == FileAccessMode::Read ? "rb" : "wb");
#endif
        if (stream_) path_ = path;
        return stream_ != nullptr;
    }
    bool IsOpen() const { return stream_ != nullptr; }
    void Close() {
        if (stream_) {
            std::fclose(stream_);
            stream_ = nullptr;
        }
    }
    bool Seek(std::int64_t pos, SeekOrigin from = SeekOrigin::SetOrigin) {
        if (!stream_) return false;
        const int origin = from == SeekOrigin::SetOrigin ? SEEK_SET :
                           from == SeekOrigin::CurrentPosition ? SEEK_CUR : SEEK_END;
        return SeekFile64(stream_, pos, origin) == 0;
    }
    std::uint64_t Tell() const {
        if (!stream_) return 0;
        const auto value = TellFile64(stream_);
        return value < 0 ? 0 : static_cast<std::uint64_t>(value);
    }
    std::uint64_t GetSize() {
        if (!stream_) return 0;
        const auto cursor = TellFile64(stream_);
        if (cursor < 0 || SeekFile64(stream_, 0, SEEK_END) != 0) return 0;
        const auto size = TellFile64(stream_);
        SeekFile64(stream_, cursor, SEEK_SET);
        return size < 0 ? 0 : static_cast<std::uint64_t>(size);
    }
    template <typename T, typename U> std::size_t ReadRaw(U* data, std::size_t count) {
        if (!stream_ || (!data && count))
            throw std::runtime_error("PKG read attempted on an unopened file");
        const auto offset = TellFile64(stream_);
        const auto received = std::fread(data, sizeof(T), count, stream_);
        if (received != count) {
            const bool io_error = std::ferror(stream_) != 0;
            throw std::runtime_error(
                std::string(io_error ? "PKG input I/O error" : "Short PS4 PKG read") +
                " at byte " + std::to_string(offset) +
                ": wanted " + std::to_string(count * sizeof(T)) +
                " bytes, received " + std::to_string(received * sizeof(T)) +
                ", file size " + std::to_string(GetSize()) +
                " (" + path_.string() + ")");
        }
        return received;
    }
    template <typename T, typename U> std::size_t WriteRaw(const U* data, std::size_t count) {
        if (!stream_ || (!data && count)) return 0;
        const auto written = std::fwrite(data, sizeof(T), count, stream_);
        if (written != count) throw std::runtime_error("Short write in PS4 PKG extraction");
        return written;
    }
    template <typename T> std::size_t Read(T& value) {
        return ReadRaw<T>(&value, 1);
    }
    template <typename T, std::size_t N> std::size_t Read(std::array<T, N>& value) {
        return ReadRaw<T>(value.data(), N);
    }
    template <typename T, std::size_t N> std::size_t Read(T (&value)[N]) {
        return ReadRaw<T>(value, N);
    }
    template <typename T> std::size_t Read(std::vector<T>& value) {
        return ReadRaw<T>(value.data(), value.size());
    }
    template <typename T> std::size_t Write(const std::vector<T>& value) {
        return WriteRaw<T>(value.data(), value.size());
    }

private:
    std::FILE* stream_ = nullptr;
    std::filesystem::path path_;
};
} // namespace Common::FS
