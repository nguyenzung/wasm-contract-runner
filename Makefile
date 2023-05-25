BUILD_DIR=build
TARGET= $(BUILD_DIR)/host
WASM_TARGET=module.wasm
PROJECT_TARGET= $(TARGET) $(WASM_TARGET)

CXX = g++
CXXFLAGS += -O0 -g
LIBRARY_PATH = /usr/local/lib 
LIBRARY_NAME = -lwasmer -lcrypto

CPP_HEAD_FILES := $(shell find . -name "*.h")
CPP_SOURCE_FILES := $(shell find . -name "*.cpp")
CPP_OBJ_FILES := $(patsubst ./%.cpp, $(BUILD_DIR)/%.o, $(CPP_SOURCE_FILES))

.PHONY: all build wasm clean

all: $(TARGET)  $(WASM_TARGET)
	./$(TARGET)

$(CPP_OBJ_FILES): $(BUILD_DIR)/%.o : %.cpp 
	@mkdir -pv $(dir $@)
	$(CXX) $(CXXFLAGS)  -c $< -o $@

$(TARGET): $(CPP_OBJ_FILES)
	# @echo $@ $<
	$(CXX) $(CPP_OBJ_FILES) -L $(LIBRARY_PATH) $(LIBRARY_NAME) -o $@

clean: 
	rm -rf $(BUILD_DIR)/* $(WASM_TARGET)

build: $(TARGET)

$(WASM_TARGET): module.c
	emcc -O3 $< -o $@ -s WASM=1 -s SIDE_MODULE=1

wasm: $(WASM_TARGET)
