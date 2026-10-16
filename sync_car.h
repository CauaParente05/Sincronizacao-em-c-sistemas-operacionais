#ifndef SYNC_CAR_H
#define SYNC_CAR_H

#include <pthread.h>

typedef struct station {
    int waiting_passengers;
    int available_seats;
    int boarding_passengers;
    
    pthread_mutex_t lock;
    pthread_cond_t car_arrived;
    pthread_cond_t passenger_boarded;
}station;

void station_init(station* station);
void station_load_car(station* station, int count);
void station_wait_for_car(station* station);
void station_on_board(station* station);

#endif