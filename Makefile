SYSLOAD_SRC	= src/sysload.c
SYSLOAD_OBJ	= build/sysload.o
SYSLOAD_LIBNAME = libsysload

SYSLOAD_STATIC	= build/$(SYSLOAD_LIBNAME).a
SYSLOAD_SHARED	= build/$(SYSLOAD_LIBNAME).so

EXAMPLE_SRC	= example/main.c
EXAMPLE_OBJ	= build/main.o
EXAMPLE_EXEC	= build/main 

TEST_SRC	= tests/test_sysload.c
TEST_OBJ	= build/test_sysload.o
TEST_EXEC	= build/test_sysload

PREFIX 		?= /usr/local
INCLUDE_DIR 	?= include/sysload
LIBRARY_DIR 	?= lib

INSTALL_INCLUDE_DIR = $(DESTDIR)$(PREFIX)/$(INCLUDE_DIR)
INSTALL_LIB_DIR	    = $(DESTDIR)$(PREFIX)/$(LIBRARY_DIR)

INSTALL 	?= cp -a

CC 	?= gcc
CFLAGS	:= $(CFLAGS) -Wall -Wextra -std=gnu99 -O2 -fPIC
LDFLAGS := $(LDFLAGS)

$(shell mkdir -p build)


all: static shared example

static: $(SYSLOAD_STATIC)

shared: $(SYSLOAD_SHARED)

example: $(SYSLOAD_STATIC) $(EXAMPLE_EXEC)

run-example: example
	./$(EXAMPLE_EXEC)

$(SYSLOAD_OBJ): $(SYSLOAD_SRC) include/sysload.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

$(SYSLOAD_STATIC): $(SYSLOAD_OBJ)
	ar rcs $@ $^

$(SYSLOAD_SHARED): $(SYSLOAD_OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(EXAMPLE_OBJ): $(EXAMPLE_SRC) include/sysload.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

$(EXAMPLE_EXEC): $(EXAMPLE_OBJ) $(SYSLOAD_STATIC)
	$(CC) $(LDFLAGS) -o $@ $(EXAMPLE_OBJ) $(SYSLOAD_STATIC)

install: all
	install -d $(INSTALL_INCLUDE_DIR)
	install -d $(INSTALL_LIB_DIR)
	install -m 644 include/sysload.h $(INSTALL_INCLUDE_DIR)/
	install -m 644 $(SYSLOAD_STATIC) $(INSTALL_LIB_DIR)/
	install -m 644 $(SYSLOAD_SHARED) $(INSTALL_LIB_DIR)/
	ldconfig $(INSTALL_LIB_DIR) || true

install-static: static
	install -d $(INSTALL_INCLUDE_DIR)
	install -d $(INSTALL_LIB_DIR)
	install -m 644 include/sysload.h $(INSTALL_INCLUDE_DIR)/
	install -m 644 $(SYSLOAD_STATIC) $(INSTALL_LIB_DIR)/

install-shared: shared
	install -d $(INSTALL_INCLUDE_DIR)
	install -d $(INSTALL_LIB_DIR)
	install -m 644 include/sysload.h $(INSTALL_INCLUDE_DIR)/
	install -m 644 $(SYSLOAD_SHARED) $(INSTALL_LIB_DIR)/
	ldconfig $(INSTALL_LIB_DIR) || true

$(TEST_OBJ): $(TEST_SRC) include/sysload.h
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

$(TEST_EXEC): $(TEST_OBJ) $(SYSLOAD_STATIC) 
	$(CC) -o $@ $(TEST_OBJ) $(SYSLOAD_STATIC)

test: $(TEST_EXEC)
	@echo "Running sysload tests..."
	@./$(TEST_EXEC)

clean:
	rm -f $(SYSLOAD_OBJ) $(SYSLOAD_STATIC) $(SYSLOAD_SHARED) $(EXAMPLE_OBJ) $(EXAMPLE_EXEC) $(TEST_OBJ) ($TEST_EXEC)

uninstall:
	rm -f $(INSTALL_INCLUDE_DIR)/sysload.h
	rm -f $(INSTALL_LIB_DIR)/$(SYSLOAD_LIBNAME).a
	rm -f $(INSTALL_LIB_DIR)/$(SYSLOAD_LIBNAME).so
	rmdir -f $(INSTALL_INCLUDE_DIR)/sysload

.PHONY: all example run-example install install-static install-shared test uninstall clean
