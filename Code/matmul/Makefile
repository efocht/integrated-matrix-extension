CC  = gcc
CXX = g++

all : matmul matmul.s matmul.out

matmul.out : matmul
	perf stat -d -d -d numactl -C 8 ./matmul 2>&1 | tee -a matmul.out

matmul : matmul.o
	$(CXX) $< -o $@

memory : memory.o
	$(CXX) $< -o $@

%.o : %.cc
	$(CXX) -O2 -fno-unroll-loops -c $<

%.s : %.o
	objdump -D $< > $@

clean :
	rm -rf matmul.out matmul.s matmul.o matmul
