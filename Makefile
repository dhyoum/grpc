MODULES := bazel/8.4.2 gcc/14.2.0 clang/20.1.1 lcov/2.3.2
LOAD_MODULES := module add $(MODULES) &&

GCC_GCOV := /app/vbuild/RHEL9-x86_64/gcc/14.2.0/bin/gcov
COVERAGE_DIR := coverage

.PHONY: build run test tidy coverage clean

build:
	$(LOAD_MODULES) bazel build //main:app //util/sum //util/factorial

run:
	$(LOAD_MODULES) bazel run //main:app

test:
	$(LOAD_MODULES) bazel test //util/sum:sum_test //util/factorial:factorial_test --test_output=all

tidy:
	$(LOAD_MODULES) bazel build --config=tidy //main:app //util/sum //util/factorial
	@echo "\n=== clang-tidy results ==="
	@find -L bazel-out -name "*.clang-tidy.txt" -path "*/bin/*" ! -path "*runfiles*" -exec cat {} \;

coverage:
	$(LOAD_MODULES) bazel coverage //util/sum:sum_test //util/factorial:factorial_test \
		--combined_report=lcov \
		--test_output=all \
	&& mkdir -p $(COVERAGE_DIR) \
	&& genhtml bazel-out/_coverage/_coverage_report.dat \
		--output-directory $(COVERAGE_DIR) \
		--title "project coverage" \
	&& echo "" \
	&& echo "=== Coverage report generated ===" \
	&& echo "Open: $(COVERAGE_DIR)/index.html"

clean:
	$(LOAD_MODULES) bazel clean
	rm -rf $(COVERAGE_DIR)
