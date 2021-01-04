
SUBDIRS := src
	
CC ?= gcc
CFLAGS = $(CFLAG)
CFLAGS +=

export CFLAGS CC

.PHONY:all clean test $(SUBDIRS)
all clean:
	$(MAKE) $(SUBDIRS) TARGET=$@

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)


