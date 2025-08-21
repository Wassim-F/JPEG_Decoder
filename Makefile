CC = gcc
LD = gcc

# -O0 désactive les optimisations à la compilation
# C'est utile pour débugger, par contre en "production"
# on active au moins les optimisations de niveau 2 (-O2).
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O0 -g
LDFLAGS = -lm
EXEC = jpeg2ppm fct_tests

# Par défaut, on compile tous les fichiers source (.c) qui se trouvent dans le
# répertoire src/
SRC_FILES=src/fonctions.c src/jpeg2ppm.c

# Par défaut, la compilation de src/toto.c génère le fichier objet obj/toto.o
OBJ_FILES=$(patsubst src/%.c,obj/%.o,$(SRC_FILES))

SRC_TESTS=tests/fct_tests.c
OBJ_TESTS=$(patsubst tests/%.c,obj/%.o,$(SRC_TESTS)) obj/fonctions.o

all: $(EXEC)

jpeg2ppm: $(OBJ_FILES) 
	$(LD) $(OBJ_FILES) $(LDFLAGS) -o $@

fct_tests: $(OBJ_TESTS) 
	$(LD) $(OBJ_TESTS) $(LDFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

obj/%.o: tests/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -rf $(EXEC) obj/*.o
