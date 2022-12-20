# source /opt/rocm/activate.sh
# make gpu_impl
# make clean

CPP = g++
HIP = hipcc

CFLAGS = -I /home/steve/dev/CImg/ -std=c++11
CFLAGS_CC = -I /home/stevecarlson/CImg/ -std=c++11
LD     = -lX11 -lpthread

all: cpu_impl
# all: gpu_impl

cpu_impl: cpu_implementation.cpp  
	$(CPP) $(CFLAGS) -o cpu_impl cpu_implementation.cpp $(LD)

gpu_impl: cpu_implementation.cpp  
	$(HIP) $(CFLAGS_CC) $(LD) -o gpu_impl gpu_implementation.cpp
	
clean:
	rm -f cpu_impl gpu_impl *.dat *.o
