CXXFLAGS := -Wall -Werror -O3 -std=c++11
LDFLAGS := -lprimesieve

EXECUTABLE = bf.out

all: $(EXECUTABLE)

bf.out: bf.o
	$(CXX) -o $@ $^ $(LDFLAGS)

bf.o: bf.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -f *~ *.out *.o
