# Makefile for MinGW

CC=gcc
BIN=bin
OBJ=$(BIN)/obj

all: makedirs $(BIN)/WorldTransformExample.exe

$(BIN)/WorldTransformExample.exe: $(OBJ)/WinMain.o $(OBJ)/WndProc.o $(OBJ)/public.o
	$(CC) $(OBJ)/WinMain.o $(OBJ)/WndProc.o $(OBJ)/public.o	\
		-lgdi32 -ldwmapi	\
		-o $(BIN)/WorldTransformExample.exe

$(OBJ)/WinMain.o: WinMain.c public.h
	$(CC) -c WinMain.c -o $(OBJ)/WinMain.o
$(OBJ)/WndProc.o: WndProc.c public.h
	$(CC) -c WndProc.c -o $(OBJ)/WndProc.o
$(OBJ)/public.o: public.c public.h
	$(CC) -c public.c -o $(OBJ)/public.o

clean:
	rm -f $(BIN)/* $(OBJ)/*

makedirs:
	"mkdir" -p $(BIN) $(OBJ)

.PHONY: all clean makedirs