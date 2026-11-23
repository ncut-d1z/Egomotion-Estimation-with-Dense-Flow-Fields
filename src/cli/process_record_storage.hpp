#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace egomotion::cli::process_record {

class Fd {
public:
    explicit Fd(int value = -1) noexcept : value_(value) {}
    ~Fd();
    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;
    Fd(Fd&& other) noexcept;
    int get() const noexcept { return value_; }
    void reset() noexcept;
private:
    int value_;
};

class Directory {
public:
    explicit Directory(const std::filesystem::path& root);
    Fd newFile(const char* name) const;
    void publish(const char* name, const std::string& contents) const;
    int fd() const noexcept { return directory_.get(); }
    const std::filesystem::path& path() const noexcept { return path_; }
    const std::string& id() const noexcept { return id_; }
private:
    std::string id_;
    std::filesystem::path path_;
    Fd directory_;
};

void checked(bool success, const char* operation);
bool tryWriteAll(int fd, std::string_view contents) noexcept;
void writeAll(int fd, const std::string& contents);

}  // namespace egomotion::cli::process_record
