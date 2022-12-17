
CPP = g++
CFLAGS = -I /home/steve/dev/CImg/ -std=c++11
LD     = -lX11 -lpthread
all: cpu_impl

cpu_impl: cpu_implementation.cpp  
	$(CPP) $(CFLAGS) -o cpu_impl cpu_implementation.cpp $(LD)
clean:
	rm -f cpu_impl *.dat *.o
