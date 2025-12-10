CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

.PHONY: all clean

all: process_memory parallel_sum zombie

# ===== Задание 3: process_memory =====
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o $@ process_memory.c

# ===== Библиотека суммы (Задание 5) =====
sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c

libsum.a: sum.o
	ar rcs libsum.a sum.o

# ===== Задание 5 и 6: parallel_sum =====
parallel_sum: parallel_sum.c libsum.a
	$(CC) $(CFLAGS) -o $@ parallel_sum.c libsum.a

# ===== Задание 2: zombie =====
zombie: zombie.c
	$(CC) $(CFLAGS) -o $@ zombie.c

clean:
	rm -f process_memory parallel_sum zombie libsum.a sum.o