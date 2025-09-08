/*
 * 
 * fft.cpp  Andrew Belles  Sept 8th, 2025 
 *
 * Self-made/designed Fast Fourier Transform algorithm 
 * to be used to separate audio signals into principal signals 
 *
 * File from command line should be .wav format 
 * 
 */

#include <iostream> 
#include <fstream> 
#include <memory> 
#include <complex> 
#include <vector> 

/************ local constants ******************************/
const int buffer_size = 256; 

typedef struct wave wave_t;

/************ local helper functions ***********************/
static bool parseArgs(int argc, char* argv[], char** wavFile, 
                      std::unique_ptr<wave> data);
static constexpr uint32_t raw_4b_uint(const uint8_t b[4]) noexcept;

/************ structure definitions ************************/
struct wave {
  const std::string path;   // original path to .wav

  wave(const std::string& audiowav) : path(audiowav) {
    std::ifstream bin(audiowav, std::ios::in | std::ios::binary);

    if ( !bin.is_open() ) {
      std::cerr << "File: " << audiowav << " failed to open as binary file\n";
      exit( 99 );
    }

    std::vector<char> buffer(buffer_size);
    bin.read(buffer.data(), 12);           // read wave header 
    
    // step through first 12 bytes to confirm head and wave format 
    uint32_t riff[4];                     // four bytes for riff
    for (int b = 0; b < 4; b++) {
      riff[b] = buffer[b];
    }

#ifdef DEBUG 
    fputs("Expect:", stdout);
    for (int b = 0; b < 4; b++) {
      std::cout << " " << char(riff[b]);
    }
    fputc('\n', stdout);
#endif 

    uint8_t size[4]; 
    for (int b = 4; b < 8; b++) {
      size[b - 4] = uint8_t(buffer[b]);
    }


    uint32_t binsize = raw_4b_uint(size);


    // placeholder for filesize (ignore?)
    uint32_t wave[4];
    for (int b = 0; b < 4; b++) {
      wave[b] = buffer[b];
    }
  }
}; 

int main(int argc, char* argv[]) {

}


static bool 
parseArgs(int argc, char* argv[], char** wavFile, 
          std::unique_ptr<wave> data)
{
  if ( argc != 2 ) {
    return false; 
  } 
  
  // call to wave constructor, handles exit on failure   
  data = std::unique_ptr<wave>(new wave(argv[1]));


  return true; 
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
