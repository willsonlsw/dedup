ded: dedup.cpp dedup.h gpu-process.h hash-search.h sha1-gpu.cu rabin-cpu.cpp
	nvcc -g -o ded -D_FILE_OFFSET_BITS=64 dedup.cpp sha1-gpu.cu rabin-cpu.cpp

clean:
	rm ded
