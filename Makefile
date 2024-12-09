all: io_perf
io_perf: io_perf.c
	gcc -o io_perf io_perf.c

clean:
	rm -f io_perf

run: io_perf
	./io_perf 16000000 1 && echo && ./io_perf 16000000 2 && echo && ./io_perf 16000000 4 && echo && ./io_perf 160000000 2