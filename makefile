all: runner

runner: ./src/schedule_algo.cpp ./src/compute_unit.cpp ./src/framework.cpp
	g++ ./src/schedule_algo.cpp ./src/compute_unit.cpp ./src/framework.cpp -o ./bin/runner

run:
	./bin/runner

clean:
	rm ./bin/runner