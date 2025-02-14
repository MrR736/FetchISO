CXX = g++
MOC = /lib/qt5/bin/moc

CXXFLAGS = -std=c++17 -fPIC -I../../include
LIBS = `pkg-config --cflags --libs Qt5Widgets Qt5Core Qt5Gui`

CPP_SRC = FetchISO.cpp
MOC_SRC = moc_FetchISO.cpp
OBJ_FILES = FetchISO.o moc_FetchISO.o
EXEC = FetchISO.qt5

all: $(MOC_SRC) $(EXEC)

$(MOC_SRC): FetchISO.h
	$(MOC) $< -o $@

FetchISO.o: FetchISO.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

moc_FetchISO.o: moc_FetchISO.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LIBS)

$(EXEC): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJ_FILES) $(EXEC) $(MOC_SRC)
