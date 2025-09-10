.PHONY: all clean

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
	make -C wave 
	make -C fft

############## clean  ##########
clean:
	rm -f *~
	make -C wave clean
	make -C fft clean
