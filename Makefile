#Makefile par Pacôme Perrotin

LIB_VERSION = 1.0.0

CODE_FORMAT = cpp
HEADER_FORMAT = h
OBJECT_FORMAT = o
LIBRARY_FORMAT = a
#Dossier contenant les .CODE_FORMAT :
CREP = src
#Dossier contenant les .HEADER_FORMAT :
HREP = src
#Dossier contenant les .OBJECT_FORMAT :
OREP = obj
#Dossier contenant les .LIBRARY_FORMAT :
LREP = lib
#Dossier contenant les executables :
BREP = bin
#Nom de l'executable :
BIN = game-core
#Nom du compilateur :
CC = clang++

INCLUDE = -I/usr/local/include -I/usr/include
FLAGS = -L/usr/local/lib -L/usr/lib -Wall -std=c++11 -lpthread -g #-fPIC

#Pour chaque a.c dans CREP, on considère le module a
RAW_CODE = $(shell echo $(CREP)/*.$(CODE_FORMAT))
CODE = $(RAW_CODE:$(CREP)/%.$(CODE_FORMAT)=%)

CFILES = $(CODE:%=%.$(CODE_FORMAT))
HDR = $(CODE:%=%.$(HEADER_FORMAT))
HEADERS = $(HDR:%=$(HREP)/%)

SRC = $(CFILES) $(HDR)

OBJ = $(CODE:%=%.$(OBJECT_FORMAT))
OBJ_OUT = $(OBJ:%=$(OREP)/%)

BIN_OUT = $(BIN:%=$(BREP)/%)
LIB_OUT = $(BIN:%=$(LREP)/lib%.$(LIBRARY_FORMAT))

vpath %.$(CODE_FORMAT) $(CREP)
vpath %.$(HEADER_FORMAT) $(HREP)
vpath %.$(OBJECT_FORMAT) $(OREP)

all: $(SRC) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(FLAGS) $(OBJ_OUT) $(INCLUDE) -o $(BREP)/$@

%.$(OBJECT_FORMAT): %.$(CODE_FORMAT)
	$(CC) $(FLAGS) $< $(INCLUDE) -c -o $(OREP)/$@

lib: $(LIB_OUT)

$(LIB_OUT) : $(OBJ) 
	ar -cvq $(LIB_OUT) $(OBJ_OUT)
	cp $(HEADERS) $(LREP)

clear:
	-rm $(BIN_OUT) -f
	-rm $(OBJ_OUT) -f
	-rm $(LIB_OUT) -f
	-rm $(LREP)/* -f

open :
	gedit src/* &

