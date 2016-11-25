# Makefile for ush
#
# Vincent W. Freeh
# 

CC=gcc
CFLAGS=-g
SRC=shell.c parse.c parse.h
OBJ=shell.o parse.o

ush:	$(OBJ)
	$(CC) -o $@ $(OBJ)

tar:
	tar czvf ush.tar.gz $(SRC) Makefile README

clean:
	\rm $(OBJ) ush

ush.1.ps:	ush.1
	groff -man -T ps ush.1 > ush.1.ps
