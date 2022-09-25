CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK)/include -I/usr/local/include -I/opt/homebrew/include
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG

LDFLAGS = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan

debug: src/*.cpp src/*.h src/*.hpp src/engine/*.cpp src/engine/*.hpp
	clang++ $(CFLAGS_DEBUG) -o FrameTech-Debug src/*.cpp src/engine/*.cpp $(LDFLAGS)

release: src/*.cpp src/*.h src/*.hpp src/engine/*.cpp src/engine/*.hpp
	clang++ $(CFLAGS) -o FrameTech-Release src/*.cpp src/engine/*.cpp $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf FrameTech-Debug FrameTech-Release
