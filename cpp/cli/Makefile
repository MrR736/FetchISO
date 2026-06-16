CXX = g++
CXXFLAGS = -std=c++17 -I../../include -I..
LIBS = `pkg-config --cflags --libs ncurses`

CPP_SRC = $(wildcard *.cpp)
CPP_OBJ = $(CPP_SRC:.cpp=.o)
EXEC = FetchISO.cli

all: $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

$(EXEC): $(CPP_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $(EXEC) $(LIBS)

clean:
	rm -f $(CPP_OBJ) $(EXEC)
