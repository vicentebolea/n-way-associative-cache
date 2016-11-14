all:
	gcc -Wall -std=c99 -pedantic -O3 cache_sim.c -o cache_sim -lm
	
debug:
	gcc -Wall -std=c99 -pedantic -g cache_sim.c -o cache_sim_debug -lm

clean:
	rm -f cache_sim
	rm -f cache_sim_debug
