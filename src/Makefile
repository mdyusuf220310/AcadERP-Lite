CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -fno-exceptions

TARGET = acaderp
SOURCES = grp_92_main.cpp grp_92_erp.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

grp_92_main.o: grp_92_main.cpp grp_92_erp.h
	$(CXX) $(CXXFLAGS) -c grp_92_main.cpp -o grp_92_main.o

grp_92_erp.o: grp_92_erp.cpp grp_92_erp.h
	$(CXX) $(CXXFLAGS) -c grp_92_erp.cpp -o grp_92_erp.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
