#include <iostream> 
#include <fstream> 
#include <cstring> 
#include <cstdint> 
#include <array> 
#include <cassert> 
#include "wave.hpp"

/************ local constants ******************************/
// static const int buffer_size = 256; 
static const int header_size = 12; 

/************ local-helper functions ***********************/
static constexpr uint32_t raw_4b_uint(const uint8_t b[4]) noexcept;

/*
 * Constructor for Wave struct 
 *
 * Input: 
 *   Path to .wav file (or invalid file)
 */

std::optional<Wave> Wave::open(std::string path) 
  noexcept 
{
  uint32_t riff_size(0), data_size(0), size(0); 
  uint64_t offset(0), payload(0), skip(0);
  uint8_t id[4], byte[4];
  std::array<uint8_t, header_size> header{};


  std::ifstream bin(path, std::ios::in | std::ios::binary);

  if ( !bin.is_open() ) {
    return std::nullopt; 
  }

  bin.read(reinterpret_cast<char*>(header.data()), header.size());

  // Compare read header to RIFF 
  if ( std::memcmp(header.data(), "RIFF", 4) != 0 && 
       std::memcmp(header.data(), "RF64", 4) != 0 ) {
    return std::nullopt; 
  }

  riff_size = raw_4b_uint(&header[4]);

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
    
    size = raw_4b_uint(byte);
    payload = static_cast<uint64_t>(bin.tellg()); // get current read pos  

    if ( std::memcmp(id, "data", 4) == 0 ) {
      offset = payload; 
      data_size = size; 
      break; 
    }

    skip = size + (size & 0b1); // pad with offset if odd  
    bin.seekg(static_cast<std::streamoff>(skip), std::ios::cur);
    
    if ( !bin ) {
      return std::nullopt;  
    } // we never will find the data start hence throw None
  }

  if ( offset == 0 || data_size == 0 ) {
    return std::nullopt; 
  }

  return Wave(std::move(path), riff_size, offset, data_size);
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
raw_4b_uint(const uint8_t b[4]) 
  noexcept 
{
  return uint32_t(b[0]) 
    | (uint32_t(b[1]) << 8)
    | (uint32_t(b[2]) << 16) 
    | (uint32_t(b[3]) << 24);
}

#ifdef UNIT_TEST 

int main(void) {
  
  if ( auto wav = Wave::open("sci-fi.wav")) {
    std::cout << "Path: " << wav->path()
             << " offset=" << wav->offset()
             << " data_size=" << wav->data_size() << '\n';
  } else {
    std::cout << "failure\n"; 
    exit(1); 
  }

  auto bad = Wave::open("wave.cpp");
  assert( !bad );
  std::cout << "unwrapped none and did not panic\n";

  exit(0);
}

#endif // UNIT_TEST  
