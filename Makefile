MODULES := bazel/8.4.2 gcc/14.2.0 clang/20.1.1
LOAD_MODULES := module add $(MODULES) &&

GCC_GCOV := /app/vbuild/RHEL9-x86_64/gcc/14.2.0/bin/gcov
GENHTML := PERL5LIB=/app/vbuild/RHEL9-x86_64/lcov/2.3.2:/app/vbuild/RHEL9-x86_64/lcov/2.3.2/x86_64-linux-thread-multi /usr/bin/perl /app/vbuild/RHEL9-x86_64/lcov/2.3.2/bin/genhtml
COVERAGE_DIR := coverage

.PHONY: build run test tidy coverage clean server client compdb

build:
	$(LOAD_MODULES) bazel build //main:app //util/sum //util/factorial //server:server //client:client

run:
	$(LOAD_MODULES) bazel run //main:app

server:
	$(LOAD_MODULES) bazel run //server:server

client:
	$(LOAD_MODULES) bazel run //client:client

test:
ifdef TARGET
	$(LOAD_MODULES) bazel test $(TARGET) --test_output=all
else
	$(LOAD_MODULES) bazel test //... --test_output=all
endif

tidy:
	$(LOAD_MODULES) bazel build --config=tidy //main:app //util/sum //util/factorial
	@echo "\n=== clang-tidy results ==="
	@find -L bazel-out -name "*.clang-tidy.txt" -path "*/bin/*" ! -path "*runfiles*" -exec cat {} \;

coverage:
	$(LOAD_MODULES) bazel coverage //... \
		--combined_report=lcov \
		--test_output=all \
		--instrumentation_filter="//util[:/],//server[:/],//client[:/],//main[:/]" \
	&& mkdir -p $(COVERAGE_DIR) \
	&& $(GENHTML) bazel-out/_coverage/_coverage_report.dat \
		--output-directory $(COVERAGE_DIR) \
		--title "project coverage" \
	&& echo "" \
	&& echo "=== Coverage report generated ===" \
	&& echo "Open: $(COVERAGE_DIR)/index.html"

compdb:
	$(LOAD_MODULES) bazel run :refresh_compile_commands

clean:
	$(LOAD_MODULES) bazel clean --async
	rm -rf $(COVERAGE_DIR)
