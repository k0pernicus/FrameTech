CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK)/include -I/usr/local/include -I/opt/homebrew/include
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS) -O2

LDFLAGS = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan

SOURCES = src/*.cpp src/engine/*.cpp
HEADERS = src/*.h src/*.hpp src/engine/*.hpp
ALL_SOURCES = $(SOURCES) $(HEADERS)

MKDIR_P = mkdir -p
OUTPUT_BASE = ./bin
BIN = FrameTech
OUTPUT_DEBUG = $(OUTPUT_BASE)/debug/$(BIN)
OUTPUT_RELEASE = $(OUTPUT_BASE)/release/$(BIN)

debug: $(ALL_SOURCES)
	$(MKDIR_P) $(OUTPUT_BASE)/debug
	clang++ $(CFLAGS_DEBUG) -o $(OUTPUT_DEBUG) $(SOURCES) $(LDFLAGS)

release: $(ALL_SOURCES)
	$(MKDIR_P) $(OUTPUT_BASE)/release
	clang++ $(CFLAGS_RELEASE) -o $(OUTPUT_RELEASE) $(SOURCES) $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OUTPUT_BASE)
