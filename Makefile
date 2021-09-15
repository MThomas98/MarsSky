CXX = g++

SRCS := $(wildcard src/*.cpp src/imgui/*.cpp)
OBJS := $(patsubst src/%,obj/%,$(SRCS:.cpp=.o))

INCLUDE_PATHS = -Iinclude/
LIBRARY_PATHS = -Llib/
COMPILER_FLAGS = -std=c++11 -O3 -c -Wl,--subsystem,windows -static-libgcc -static-libstdc++
LINKER_FLAGS = -lm -lassimp -lglu32 -lglew32 -lglfw3 -lgdi32 -lopengl32

EXE_NAME = cs310

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS)
	$(CXX) $(LIBRARY_PATHS) -o $@ $^ $(LINKER_FLAGS)

obj/%.o: src/%.cpp
	$(CXX) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	$(RM) -r obj/*.o obj/imgui/*.o ./$(EXE_NAME)
