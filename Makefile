include config.mk

VERSION := 0.1
COPYRIGHT = ©2019 Joshua Scoggins 
CXXFLAGS += '-DVERSION="$(VERSION)"' \
			'-DCOPYRIGHT="$(COPYRIGHT)"'

LIBKZR_CORE_OBJS := Message.o \
	Operations.o \
	Exception.o \
	Connection.o \
	FileHandleConnection.o \
	SocketConnection.o \
	UnixDomainSocketConnection.o

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


Connection.o: Connection.cc Connection.h Message.h Operations.h \
 Exception.h
Exception.o: Exception.cc Exception.h
FileHandleConnection.o: FileHandleConnection.cc FileHandleConnection.h \
 Connection.h Message.h Operations.h
Message.o: Message.cc Message.h Operations.h Exception.h
Operations.o: Operations.cc Operations.h Message.h
SocketConnection.o: SocketConnection.cc SocketConnection.h \
 FileHandleConnection.h Connection.h Message.h Operations.h Exception.h
UnixDomainSocketConnection.o: UnixDomainSocketConnection.cc \
 UnixDomainSocketConnection.h SocketConnection.h FileHandleConnection.h \
 Connection.h Message.h Operations.h
