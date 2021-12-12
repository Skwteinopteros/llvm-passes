ifndef VERBOSE
.SILENT:
endif

LLVM_VERSION := 12

all: clean build

build_passes: ;@echo Building llvm-pass...
	cmake -S . -B out/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_C_COMPILER=clang-$(LLVM_VERSION) \
		-DCMAKE_CXX_COMPILER=clang++-$(LLVM_VERSION)
	cmake --build out/build
	echo Done!
	echo

build: clean build_passes

format: ;
	find . -type f \( -name "*.h" -o -name "*.cpp" -o -name "*.c" -o -name "*.hpp" -o -name "*.proto" \) \
		-exec clang-format-$(LLVM_VERSION) -i {} +

clean: ;@echo Cleaning...
	if [ -d out ]; then \
		rm -rf out; \
	fi
	rm -f *.log
	echo Done!
	echo
