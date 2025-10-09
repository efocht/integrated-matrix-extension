CC  = gcc
CXX = g++

all : matmul matmul.s matmul.out packc packc.s packc.out

packc.out : packc
	perf stat -d -d -d numactl -C 8 ./packc 2>&1 | tee -a packc.out

matmul.out : matmul
	perf stat -d -d -d numactl -C 8 ./matmul 2>&1 | tee -a matmul.out

packc : packc.o
	$(CXX) $< -o $@

matmul : matmul.o
	$(CXX) $< -o $@

memory : memory.o
	$(CXX) $< -fopenmp -o $@

%.o : %.cc
	$(CXX) -O0 -fno-unroll-loops -fopenmp -c $<

%.s : %.o
	objdump -D $< > $@

clean :
	rm -rf matmul.out matmul.s matmul.o matmul memory.o memory memory.s packc.out packc packc.o
