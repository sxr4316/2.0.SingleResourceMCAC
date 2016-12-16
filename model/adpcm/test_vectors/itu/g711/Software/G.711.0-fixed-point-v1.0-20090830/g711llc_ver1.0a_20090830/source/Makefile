SUBDIRS = basicop g711llc utility

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	BINARY_LOCATION=../binaries/mac
else
	BINARY_LOCATION=../binaries/linux
endif

export CFLAGS=-ansi -Wall -pedantic -W -Wextra -Wshadow -Wno-long-long -O3 -fPIC -DWMOPS=1 -DHAVE_SYS_TIME_H -g

.PHONY: all clean subdirs $(SUBDIRS)

all: subdirs
	mkdir -p $(BINARY_LOCATION)
	$(CC) -o $(BINARY_LOCATION)/g711llc g711llc.c $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.o))
	@echo "**** Binary created in: $(BINARY_LOCATION)"

clean:
	$(MAKE) -C basicop clean
	$(MAKE) -C g711llc clean
	$(MAKE) -C utility clean
	$(RM) $(BINARY_LOCATION)/g711llc

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

