BUILD_DIR = build
CXX = g++
CMAKE = cmake

all: build lint test

build:
	$(CMAKE) . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(BUILD_DIR)

lint:
	run-clang-tidy -p $(BUILD_DIR)

test:
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

.PHONY: build lint test clean
