include ../common/Makefile.def

TARGET = transform$(EXE)

SOURCES = \
    src/main.cpp \
    ../scheduler/src/scheduler.cpp

INCLUDE= -I/usr/local/pion-5.0.6/include -I../driver/src -I../log-msg-macro/src -I../scheduler/src/
LIB= -L/usr/local/pion-5.0.6/lib -lpion -lboost_thread -lboost_filesystem -llog4cpp -lssl -lcrypto
CXX_FLAGS += 

include ../common/Makefile.in
