MAKE = mingw32-make

CXX = g++
CXXFLAGS = -c -Wall -std=c++17
LDFLAGS = -lgdi32 -luser32

SRC_DIR = src
GR_3_DIR = $(SRC_DIR)/GR_3

GRAPHIC_3 = $(GR_3_DIR)/Graphic_3.cpp

SRC = $(SRC_DIR)/main.cpp $(GRAPHIC_3)
OBJ = $(SRC:.cpp=.o)
TARGET = main.exe

ifeq ($(OS),Windows_NT)
    RM = del /Q /F
    RM_OBJ = del /Q /F $(subst /,\,$(OBJ))
else
    RM = rm -f
    RM_OBJ = $(RM) $(OBJ)
endif

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET)
	$(RM_OBJ)