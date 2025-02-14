CXX = g++
MOC = /usr/lib/qt6/libexec/moc

CXXFLAGS = -std=c++17 -I../../include
LIBS = `pkg-config --cflags --libs Qt6Widgets Qt6Core Qt6Gui`

CPP_SRC = FetchISO.cpp
MOC_SRC = moc_FetchISO.cpp
OBJ_FILES = FetchISO.o moc_FetchISO.o
EXEC = FetchISO.qt6

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
