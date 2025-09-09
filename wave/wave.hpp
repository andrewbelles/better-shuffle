#ifndef __WAVE_HPP
#define __WAVE_HPP 

#include <string>
#include <optional>
#include <cstdint> 

class Wave {
public: 

  static std::optional<Wave> open(std::string path) noexcept; 

  std::string& path() noexcept { return path_; };
  uint32_t riff_size() noexcept { return riff_size_; }; 
  uint64_t offset() noexcept { return offset_; }; 
  uint32_t data_size() noexcept { return data_size_; };

private:
  
  Wave(std::string path, uint32_t riff_size, 
       uint64_t offset, uint32_t data_size) noexcept
    : path_(std::move(path)), riff_size_(riff_size), 
      offset_(offset), data_size_(data_size) {}

  std::string path_; 
  uint32_t riff_size_; 
  uint64_t offset_;
  uint32_t data_size_; 
};

#endif // __WAVE_HPP
