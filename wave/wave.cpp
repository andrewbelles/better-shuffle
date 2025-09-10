#include <iostream> 
#include <fstream> 
#include <cstring> 
#include <cstdint> 
#include <array> 
#include <vector> 
#include <cassert> 
#include <optional>
#include "wave.hpp"

/************ local constants ******************************/
static constexpr int header_size = 12; 

/**** formating constants ****/
static constexpr uint8_t FMT_PCM[16] = { 
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 
  0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};
static constexpr uint8_t FMT_FLOAT[16] = {
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 
  0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};

/**** normalization constants ****/ 
static constexpr double BIT8  = 128.0;  
static constexpr double BIT16 = 32768.0;  
static constexpr double BIT24 = 8388608.0;  
static constexpr double BIT32 = 2147483648.0;  

/************ local-helper functions ***********************/
static constexpr uint32_t to_uint32(const uint8_t b[4]) noexcept;
static constexpr uint16_t to_uint16(const uint8_t b[2]) noexcept;
static constexpr int16_t to_int16(const uint8_t b[2]) noexcept;
static constexpr int32_t to_int24(const uint8_t b[4]) noexcept; 
static constexpr int32_t to_int32(const uint8_t b[4]) noexcept;
static float to_float(const uint8_t b[4]) noexcept;
static constexpr float normalize_pcm(int32_t n, int bit_ct) noexcept;
static inline uint64_t get_sample_size(uint16_t& block_align, uint32_t& data_size, 
                                       uint16_t& channels) noexcept;

/*
 * Wrapper over Constructor for Wave struct. 
 * Returns an option if .wav fails to open  
 *
 * Caller Provides  
 *   Path to file
 * 
 * We return: 
 *   An optional Wave struct. If fails to open file and parse to data 
 *     we return a nullopt. 
 */
std::optional<Wave> 
Wave::open(std::string path) 
{
  uint32_t sample_rate(0), data_size(0), size(0); 
  uint16_t audio_format(0), channels(0), block_align(0), bits_per_sample(0);
  uint64_t offset(0), payload(0), skip(0);
  uint8_t id[4], byte[4];
  bool fmtValid = false; 

  std::array<uint8_t, header_size> header{};
  std::ifstream bin(path, std::ios::in | std::ios::binary);

  if ( !bin.is_open() ) {
    return std::nullopt; 
  }

  bin.read(reinterpret_cast<char*>(header.data()), header.size());
  if ( bin.gcount() != static_cast<std::streamsize>(header.size()) ) {
    return std::nullopt; 
  }

  // Compare read header to RIFF 
  if ( std::memcmp(header.data(), "RIFF", 4) != 0 && 
       std::memcmp(header.data(), "RF64", 4) != 0 ) {
    return std::nullopt; 
  }

  // Check for WAVE in header 
  if ( std::memcmp(header.data() + 8, "WAVE", 4) ) {
    return std::nullopt; 
  }

  // parse subchunks of binary for start of data 
  while ( bin ) {

    bin.read(reinterpret_cast<char*>(id), 4);
    if ( !bin ) {
      break; 
    }

    bin.read(reinterpret_cast<char*>( byte), 4);
    if ( !bin ) {
      break; 
    }
    
    size = to_uint32(byte);
    payload = static_cast<uint64_t>(bin.tellg()); // get current read pos  
  
    if ( std::memcmp(id, "fmt ", 4) == 0) {
      if ( size < 16 ) {
        return std::nullopt; 
      }

      std::vector<uint8_t> bufr(size);  // init w/ runtime size 
      bin.read(reinterpret_cast<char*>(bufr.data()), size);
      if ( !bin ) {
        return std::nullopt; 
      }

      audio_format    = to_uint16(&bufr[0]); // (1,PCM), (3,FLOAT) 
      channels        = to_uint16(&bufr[2]);
      sample_rate     = to_uint32(&bufr[4]);
      block_align     = to_uint16(&bufr[12]);
      bits_per_sample = to_uint16(&bufr[14]);

      if ( audio_format == 0xFFFE && size >= 40 ) { // resolve extensible format 
        const uint8_t* guid = &bufr[24];
        // compare to each extensible fmt heading 
        if ( std::memcmp(guid, FMT_PCM, 16) == 0 ) {
          audio_format = 1; 
        } else if ( std::memcmp(guid, FMT_FLOAT, 16) == 0 ) {
          audio_format = 3;
        }
      }
      fmtValid = ( channels > 0 && sample_rate > 0 && 
                   block_align > 0 && bits_per_sample > 0);
    } else if ( std::memcmp(id, "data", 4) == 0 ) {

      offset = payload; 
      data_size = size; 
      break; 

    } else { 

      skip = size + (size & 0b1); // pad with offset if odd  
      bin.seekg(static_cast<std::streamoff>(skip), std::ios::cur);
    }
    
    if ( !bin ) {
      return std::nullopt;  
    } // we never will find the data start hence throw None
    
    if ( fmtValid && offset && data_size ) {
      break; 
    }
  }

  if ( !fmtValid || offset == 0 || data_size == 0 ) {
    return std::nullopt; 
  }

  // assertion that block_align == channels * bytes_per_sample
  if ( !(bits_per_sample / 8) || 
      block_align != channels * ( bits_per_sample / 8) ) {
    return std::nullopt; 
  } 

  // return constructor if valid wave 
  return Wave(std::move(path), sample_rate, channels, bits_per_sample, 
              block_align, audio_format, offset, data_size);
}



bool 
Wave::read(void)
{
  uint64_t sample_size(0); 
  const uint16_t bytes_per_sample = bits_per_sample_ / 8; 
  const bool isFloat = ( audio_format_ == 3 && bits_per_sample_ == 32);

  if ( channels_ == 0 || data_size_ == 0 ) {
    return false; 
  }

  // open bitstream for .wav 
  std::ifstream bin(path_, std::ios::binary); 
  if ( !bin.is_open() ) {
    return false; 
  }

  bin.seekg(static_cast<std::streamoff>(offset_), std::ios::beg);
  if ( !bin ) {
    return false; 
  }

  if ( block_align_ == 0 || data_size_ / block_align_ == 0 ) {
    return false; 
  }

  std::vector<uint8_t> raw(data_size_);
  bin.read(reinterpret_cast<char*>(raw.data()), raw.size());
  if ( static_cast<uint64_t>(bin.gcount()) != raw.size()) {
    return false; 
  }
  
  // Clear samples and set size once we are ready to proceed 
  sample_size = get_sample_size(block_align_, data_size_, channels_);
  samples.clear();
  samples.resize(sample_size);
  
  auto decode_pcm = [&](const uint8_t* ptr) -> float {
    int32_t n(0);

    switch ( bits_per_sample_ ) {
      case 8: 
        n = int32_t(int(*ptr) - 128);
        break; 
      case 16: 
        n = to_int16(ptr);
        break;
      case 24: 
        n = to_int24(ptr);
        break; 
      case 32: 
        n = to_int32(ptr);
        break;
      default: 
        return 0.0;
    }
    
    return normalize_pcm(uint32_t(n), bits_per_sample_);
  };

  // loops over blocks of data, converting raw bytes from stream into correct data
  for (uint64_t frame(0); frame < data_size_ / block_align_; frame++) {
    const uint64_t fidx = frame * block_align_; 
    const uint64_t out_idx = frame * channels_; 

    for (uint16_t channel(0); channel < channels_; channel++) {

      const uint8_t* ptr = raw.data() + fidx + channel * bytes_per_sample; 

      float y = isFloat ? to_float(ptr) : decode_pcm(ptr);
      samples[channel + out_idx] = y; 
    }
  }

  return true;
}

void 
Wave::separate(void) 
{


}


/*
 * Helper function to return the correct size for samples to be 
 *
 */
static inline uint64_t 
get_sample_size(uint16_t& block_align, uint32_t& data_size, uint16_t& channels)
  noexcept
{
  return (!block_align) ? 0 : ( data_size / block_align ) * channels; 
}

/************ byte packet to value helper functions ********/ 

/*
 * Wrapper over uint16 casting into int16  
 */
static constexpr int16_t 
to_int16(const uint8_t b[2]) noexcept { return int16_t(to_uint16(b)); }

/*
 * Convert read bytes from buffer into 4 byte integer, 
 * expect 3 byte packet and sign extend if necessary. 
 */ 
static constexpr int32_t 
to_int24(const uint8_t b[4]) // but really only 3 readable bytes 
  noexcept
{
  int32_t ext(uint32_t(b[0]) | (uint32_t(b[1]) << 8) | (uint32_t(b[2]) << 16));
  if ( ext & 0x00800000) {
    ext |= 0xFF000000;
  }
  return ext; 
}

/*
 * Wrapper over uint32 casting into int32  
 */
static constexpr int32_t 
to_int32(const uint8_t b[4]) noexcept { return int32_t( to_uint32(b) ); }

/*
 *
 *
 */ 
static float 
to_float(const uint8_t b[4])
  noexcept
{
  uint32_t raw = to_uint32(b);
  float f(0.0); 
  std::memcpy(&f, &raw, sizeof(float));
  return f; 
}

/*
 * Converts 4 raw bytes into a single uint32_t 
 *
 * Caller Provides:
 *   Four raw bytes from binary 
 * 
 * We return: 
 *   Single uint32_t 
 */
static constexpr uint32_t 
to_uint32(const uint8_t b[4]) 
  noexcept 
{
  return uint32_t(b[0]) 
    | (uint32_t(b[1]) << 8)
    | (uint32_t(b[2]) << 16) 
    | (uint32_t(b[3]) << 24);
}

/*
 * Identical to 4b but for 2b 
 */
static constexpr uint16_t 
to_uint16(const uint8_t b[2]) 
  noexcept 
{ 
  return uint16_t(b[0]) | (uint16_t(b[1]) << 8);
}


static constexpr float 
normalize_pcm(int32_t n, int bit_ct)
  noexcept
{
  switch ( bit_ct ) {
    case 8: 
      return float(n) / BIT8; 
    case 16: 
      return float(n) / BIT16;
    case 24: 
      return float(n) / BIT24; 
    case 32: 
    default: 
      return float( (double(n)) / BIT32);
  }
}

#ifdef UNIT_TEST 

int main(void) {
  auto wav = Wave::open("sci-fi.wav"); 

  if ( !wav ) {
    std::cout << "failure to read\n"; 
    return 1; 
  }

  std::cout << "{path="            << wav->path()             << "},"
            << "{sample_rate="     << wav->sample_rate()      << "},"
            << "{channels="        << wav->channels()         << "},"
            << "{bits_per_sample=" << wav->bits_per_sample()  << "},"
            << "{block_align="     << wav->block_align()      << "},"
            << "{audio_format="    << wav->audio_format()     << "},"
            << "{offset="         << wav->offset()            << "},"
            << "{data_size="      << wav->data_size()         << "}\n";

  // read samples into Wave
  wav->read();
  assert( wav->samples.size() ); 

  auto bad = Wave::open("wave.cpp");
  assert( !bad );
  std::cout << "unwrapped none and did not panic\n";

  return 0; 
}

#endif // UNIT_TEST  
