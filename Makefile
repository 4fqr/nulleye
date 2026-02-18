TOP = .
BPF_DIR = src/ebpf/bpf
SRC_DIR = src
BUILD_DIR = build

CC = gcc
CLANG = clang
BPFOBJ := $(wildcard $(BPF_DIR)/*.bpf.o)
NCURSES_EXISTS := $(shell pkg-config --exists ncurses 2>/dev/null && echo yes || echo no)
ifeq ($(NCURSES_EXISTS),yes)
	CFLAGS += -DHAVE_NCURSES=1 $(shell pkg-config --cflags ncurses)
	LDFLAGS += -lncurses
else
	CFLAGS += -DHAVE_NCURSES=0
endif
CFLAGS += -std=gnu11 -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wconversion \
	-Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wredundant-decls \
	-Wcast-qual -Wcast-align -Wwrite-strings -Wstack-protector -fstack-protector-strong \
	-D_FORTIFY_SOURCE=2 -O2 -g -Iinclude -Isrc
LDFLAGS += -lpthread -ldl -lsqlite3 -lcrypto -lz -lm

# optional libraries (only link when available)
PKG_LIBYAML := $(shell pkg-config --exists libyaml && echo yes || echo no)
PKG_LIBBPF := $(shell pkg-config --exists libbpf && echo yes || echo no)
PKG_LIBELF := $(shell pkg-config --exists libelf && echo yes || echo no)
ifeq ($(PKG_LIBYAML),yes)
	LDFLAGS += -lyaml
else
	CFLAGS += -DNYE_NO_LIBYAML
endif
ifeq ($(PKG_LIBBPF),yes)
	LDFLAGS += -lbpf
endif
ifeq ($(PKG_LIBELF),yes)
	LDFLAGS += -lelf
endif

BINS = nulleye nulleyed

.PHONY: static deb docker
OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/tui/tui.o \
	$(BUILD_DIR)/tui/dashboard.o \
	$(BUILD_DIR)/tui/panels/event_log.o \
	$(BUILD_DIR)/tui/panels/graph.o \
	$(BUILD_DIR)/tui/panels/process_tree.o \
	$(BUILD_DIR)/tui/panels/network_map.o \
	$(BUILD_DIR)/tui/panels/alerts.o \
	$(BUILD_DIR)/tui/panels/file_browser.o \
	$(BUILD_DIR)/tui/themes.o \
	$(BUILD_DIR)/tui/keys.o \
	$(BUILD_DIR)/core/event_bus.o \
	$(BUILD_DIR)/core/module.o \
	$(BUILD_DIR)/core/config.o \
	$(BUILD_DIR)/core/logger.o \
	$(BUILD_DIR)/core/db.o \
	$(BUILD_DIR)/core/metrics.o \
	$(BUILD_DIR)/core/response_engine.o \
	$(BUILD_DIR)/modules/file_integrity.o \
	$(BUILD_DIR)/modules/process_monitor.o \
	$(BUILD_DIR)/modules/network_monitor.o \
	$(BUILD_DIR)/modules/user_monitor.o \
	$(BUILD_DIR)/modules/ai_engine.o \
	$(BUILD_DIR)/ebpf/loader.o \
	$(BUILD_DIR)/ai/isolation_forest.o \
	$(BUILD_DIR)/ai/features.o \
	$(BUILD_DIR)/ai/model.o \
	$(BUILD_DIR)/ai/lstm_predictor.o \
	$(BUILD_DIR)/utils/hash.o \
	$(BUILD_DIR)/utils/file_scan.o \
	$(BUILD_DIR)/utils/proc_reader.o \
	$(BUILD_DIR)/utils/network.o \
	$(BUILD_DIR)/utils/sandbox.o \
	$(BUILD_DIR)/core/rules.o \
	$(BUILD_DIR)/plugins/python_runner.o \
	$(BUILD_DIR)/modules/response_actions.o \
	$(BUILD_DIR)/plugins/example_plugin.o

.PHONY: all clean install check-deps format

all: check-deps $(BPF_DIR) $(BPFOBJ) $(BINS)

check-deps:
	@command -v $(CLANG) >/dev/null 2>&1 || { echo >&2 "clang is required. sudo apt install clang llvm -y"; exit 1; }
	@command -v bpftool >/dev/null 2>&1 || { echo >&2 "bpftool is required. sudo apt install bpftool -y"; exit 1; }
	@pkg-config --exists libbpf || echo "libbpf-dev not found - BPF support disabled; install libbpf-dev libelf-dev to enable eBPF features"
	@pkg-config --exists libyaml || echo "libyaml not found - install libyaml-dev for config support"
	@mkdir -p $(BUILD_DIR)

$(BPF_DIR)/%.bpf.o: $(BPF_DIR)/%.bpf.c
	@echo "[bpf] compiling $<"
	$(CLANG) -O2 -g -target bpf -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tui/%.o: $(SRC_DIR)/tui/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/tui/panels/%.o: $(SRC_DIR)/tui/panels/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/modules/%.o: $(SRC_DIR)/modules/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ebpf/%.o: $(SRC_DIR)/ebpf/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ai/%.o: $(SRC_DIR)/ai/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/utils/%.o: $(SRC_DIR)/utils/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/plugins/%.o: $(SRC_DIR)/plugins/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

nulleye: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

nulleyed: $(OBJS)
	$(CC) $(CFLAGS) -DDAEMON -o $@ $(OBJS) $(LDFLAGS)

install: all
	install -d /usr/bin /etc/nulleye /var/lib/nulleye /var/log/nulleye /usr/lib/nulleye/plugins
	install -m 0755 nulleye /usr/bin/nulleye
	install -m 0755 nulleyed /usr/bin/nulleyed
	install -m 0644 config.yaml.example /etc/nulleye/config.yaml
	install -m 0644 nulleye.service /etc/systemd/system/nulleye.service
	install -m 0644 README.md /usr/share/doc/nulleye/README.md
	systemctl daemon-reload

clean:
	rm -rf $(BUILD_DIR) nulleye nulleyed $(BPFOBJ)

format:
	clang-format -i $(shell find src -name "*.c" -o -name "*.h") || true

.PHONY: test
test: nulleye
	./src/tests/run_tests.sh
