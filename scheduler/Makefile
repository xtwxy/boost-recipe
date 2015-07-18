include ../common/Makefile.def

TARGET = scheduler$(EXE)

SOURCES = \
    src/main.cpp \
    src/scheduler.cpp

INCLUDE= -I/usr/local/pion-5.0.6/include
LIB= -L/usr/local/pion-5.0.6/lib -lpion -lboost_thread -lboost_filesystem -llog4cpp -lssl -lcrypto
CXX_FLAGS += 

include ../common/Makefile.in
