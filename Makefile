all: build lint test

build:
	cmake . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release
	cmake --build build

format:
	clang-format -i src/*.cpp src/*.hpp

lint: build
	run-clang-tidy -p build

test: build
	ctest --test-dir build --output-on-failure

bench:
	@set -e; \
	for dir in \
		satBenchmarks/uf50-218 \
		satBenchmarks/uf75-325 \
		satBenchmarks/uuf50-218/UUF50.218.1000 \
		satBenchmarks/uuf75-325 \
		satBenchmarks/flat50-115 \
		satBenchmarks/pigeon-hole; do \
		echo "=== Бенчмарк: $$dir ==="; \
		./measure_all.sh "$$dir"; \
	done

clean:
	rm -rf build

.PHONY: all build format lint test bench clean
