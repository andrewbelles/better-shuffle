# .wav reader and fft  

## Goal 

Implement my own fast fourier transform algorithm to convert native `.wav` files into their discrete fourier transform to separate frequencies. 

## Wave Module 

The wave module provides interface into the `Wave` class. The wave class implements several methods that aim to provide clear and safe file access. The public methods of the `Wave` class are: 
+ `Wave::open()`: Performs the task of opening a given file path, confirming it is a `.wav` file by checking the header, and loading the necessary metadata to ensure we can successfully read the data later. Returns an `std::optional<Wave>`, allowing invalid files to not fail, instead cleanup resources and allow for skipping, programming exit, etc. without having to throw an exception.  
+ `Wave::read()`: For a valid `.wav` file, reads the data into the public samples array. This data is interleaved, hence all channels are overlayed upon each other. 

TODO Methods: 
+ `Wave::separate()`: Separates the interleaved samples array into a contiguous 2D buffer that represents each independent channel. This will be the target of the FFT program.

## FFT Module 

TODO. My plan is to implement standard FFT for contiguous arrays. There will not be a class interface, rather a simple function interface for performing the fourier transform on templated float types (float or double). 

## Build process 

To build, simply run the Makefile in root. Each submodule has its own Makefile and unit test functionality. `make valgrind` and `make test` in root will be added later to perform all available unit and valgrind tests for each module. 

The programs are compiled with `clang++` and are on `c++ 20` standard. The program does not have any external library dependencies. 
