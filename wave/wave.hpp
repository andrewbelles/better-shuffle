#ifndef __WAVE_HPP
#define __WAVE_HPP 

#include <string>
#include <optional>
#include <cstdint> 
#include <vector> 

class Wave {
public: 
  std::vector<float> samples; 

  static std::optional<Wave> open(std::string path); 
  bool read(void); 
  void separate(void);

  /**** getters **********/
  std::string& path() noexcept { return path_; };
  uint32_t sample_rate() noexcept { return sample_rate_; };
  uint16_t channels() noexcept { return channels_; };
  uint16_t bits_per_sample() noexcept { return bits_per_sample_; };
  uint16_t block_align() noexcept { return block_align_; };
  uint16_t audio_format() noexcept { return audio_format_; };
  uint64_t offset() noexcept { return offset_; };
  uint32_t data_size() noexcept { return data_size_; };

  /**** interface for separated signals ****/ 

private:
  float* sep_bufr_; // de-interleaved samples

  /**** constructor ******/
  Wave(std::string path, uint32_t sample_rate, uint16_t channels, 
       uint16_t bits_per_sample, uint16_t block_align, uint16_t audio_format, 
       uint64_t offset, uint32_t data_size) noexcept 
  : path_(std::move(path)), 
    sample_rate_(sample_rate),
    channels_(channels),
    bits_per_sample_(bits_per_sample),
    block_align_(block_align),
    audio_format_(audio_format),
    offset_(offset),
    data_size_(data_size) {}

  /**** file metadata ****/ 
  std::string path_; 
  uint32_t sample_rate_;
  uint16_t channels_;
  uint16_t bits_per_sample_;
  uint16_t block_align_;
  uint16_t audio_format_;
  uint64_t offset_;
  uint32_t data_size_; 
};

#endif // __WAVE_HPP
