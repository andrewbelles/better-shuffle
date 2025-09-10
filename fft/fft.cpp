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
#include "wave.hpp"

/************ local helper functions ***********************/
static bool parseArgs(int argc, char* argv[], char** wavFile, 
                      std::unique_ptr<wave> data);

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
