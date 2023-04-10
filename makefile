RCM_SUBMODULE = ./src/Parallel-Implementation-Reverse-Cuthill-Mckee-Algorithm

all: runner

runner: ./src/schedule_algo.cpp ./src/compute_unit.cpp ./src/framework.cpp
	g++ ./src/schedule_algo.cpp ./src/compute_unit.cpp ./src/framework.cpp -o ./bin/runner

run:
	./bin/runner

gen_rand: ./src/gen_random_jobs.cpp
	g++ -o ./bin/gen_random_jobs ./src/gen_random_jobs.cpp
	./bin/gen_random_jobs

gen_rcm: ./src/gen_rcm_jobs.cpp $(RCM_SUBMODULE)/src/functions.c $(RCM_SUBMODULE)/src/queue.c
	g++ -o ./bin/gen_rcm_jobs ./src/gen_rcm_jobs.cpp $(RCM_SUBMODULE)/src/functions.c $(RCM_SUBMODULE)/src/queue.c -I $(RCM_SUBMODULE)/inc/ -O2 -g -Wall -fopenmp -lm
	./bin/gen_rcm_jobs

clean:
	rm ./bin/runner