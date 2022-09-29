CFLAGS = -std=c++17 -I. -I$(VULKAN_SDK)/include -I/usr/local/include -I/opt/homebrew/include
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
CFLAGS_RELEASE = $(CFLAGS) -O2 -DNDEBUG

LDFLAGS = -L$(VULKAN_SDK)/lib `pkg-config --static --libs glfw3` -lvulkan

SOURCES = src/*.cpp src/engine/*.cpp src/engine/graphics/*.cpp
HEADERS = src/*.h src/*.hpp src/engine/*.hpp src/engine/graphics/*.hpp
ALL_SOURCES = $(SOURCES) $(HEADERS)

MKDIR_P = mkdir -p
OUTPUT_BASE = ./bin
BIN = FrameTech
OUTPUT_DEBUG = $(OUTPUT_BASE)/debug/$(BIN)
OUTPUT_RELEASE = $(OUTPUT_BASE)/release/$(BIN)

SHADERS_DIR = shaders

debug: clean_sources $(ALL_SOURCES)
	export VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation
	$(MKDIR_P) $(OUTPUT_BASE)/debug
	clang++ $(CFLAGS_DEBUG) -o $(OUTPUT_DEBUG) $(SOURCES) $(LDFLAGS)

release: clean_sources $(ALL_SOURCES)
	export VK_INSTANCE_LAYERS=None
	$(MKDIR_P) $(OUTPUT_BASE)/release
	clang++ $(CFLAGS_RELEASE) -o $(OUTPUT_RELEASE) $(SOURCES) $(LDFLAGS)

.PHONY: shaders clean clean_shaders clean_sources

shaders: $(SHADERS_DIR)/*
	for file in $^; do \
		glslc $${file} -o $${file}.spv; \
	done

clean: clean_shaders clean_sources

clean_shaders:
	rm -rf $(SHADERS_DIR)/*.spv

clean_sources:
	rm -rf $(OUTPUT_BASE)
