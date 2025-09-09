#
# Andrew Belles  Sept 8th, 2025 
#
# Makefile for wave class and unit test  
#

CLANG 	 = clang++
OBJS 		 = wave.o 
CPPFLAGS = -Wall -Wextra -Werror -O2 -pedantic -std=c++20 -ggdb
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

.PHONY: all test clean

all: $(OBJS) 

wave: wave.o 
	$(CLANG) $(CPPFLAGS) $^ -o $@

wave.o: wave.cpp wave.hpp  
	$(CLANG) $(CPPFLAGS) -c $< -o $@

wave_test.o: CPPFLAGS += -DUNIT_TEST
wave_test.o: wave.cpp wave.hpp 
	$(CLANG) $(CPPFLAGS) -c $< -o $@

wave_test: wave_test.o 
	$(CLANG) $(CPPFLAGS) $^ -o $@

test: wave_test
	$(VALGRIND) ./wave_test

clean:
	rm -f wave wave_test *.o  
