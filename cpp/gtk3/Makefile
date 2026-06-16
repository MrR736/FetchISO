CXX = g++

GTK_CFLAGS := $(shell pkg-config --cflags gtkmm-3.0)
GTK_LIBS   := $(shell pkg-config --libs gtkmm-3.0)

CXXFLAGS = -std=c++17 -I../../include -I.. $(GTK_CFLAGS)

CPP_SRC = ../gtk4/FetchISO.cpp
CPP_OBJ = FetchISO.o
EXEC = FetchISO.gtk3

all: $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(CPP_OBJ)
	$(CXX) $^ -o $@ $(GTK_LIBS)

clean:
	rm -f $(CPP_OBJ) $(EXEC)

.PHONY: all clean
