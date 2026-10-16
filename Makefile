CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

sync_car: sync_car-runner.o sync_car.o
	$(CC) $(CFLAGS) sync_car-runner.o sync_car.o -o sync_car

sync_car-runner.o: sync_car-runner.c sync_car.h
	$(CC) $(CFLAGS) -c sync_car-runner.c

sync_car.o: sync_car.c sync_car.h
	$(CC) $(CFLAGS) -c sync_car.c

clean:
	rm -f *.o sync_car

.PHONY: clean