include config.mk

VERSION := 0.1
COPYRIGHT = ©2019 Joshua Scoggins 
CXXFLAGS += '-DVERSION="$(VERSION)"' \
			'-DCOPYRIGHT="$(COPYRIGHT)"'

LIBKZR_CORE_OBJS := Message.o

LIBKZR_ARCHIVE := libkzr.a

OBJS := $(LIBKZR_CORE_OBJS) $(KZRC_OBJS) 
PROGS := $(LIBKZR_ARCHIVE) 


all: options $(PROGS)

options:
	@echo Build Options
	@echo ------------------
	@echo CXXFLAGS = ${CXXFLAGS}
	@echo LDFLAGS = ${LDFLAGS}
	@echo ------------------

$(LIBKZR_ARCHIVE): $(LIBKZR_CORE_OBJS)
	@echo AR ${LIBKZR_ARCHIVE}
	@${AR} rcs ${LIBKZR_ARCHIVE} ${LIBKZR_CORE_OBJS}

.cc.o :
	@echo CXX $<
	@${CXX} -I. ${CXXFLAGS} -c $< -o $@

clean: 
	@echo Cleaning...
	@rm -f ${OBJS} ${PROGS}



.PHONY: options

# generated via g++ -MM -std=c++17 *.cc


Message.o: Message.cc Message.h
