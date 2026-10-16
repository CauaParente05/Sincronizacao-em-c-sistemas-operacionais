#include "sync_car.h"

void station_init(station *station) {
    station->waiting_passengers = 0;
    station->available_seats = 0;
    station->boarding_passengers = 0;
    
    pthread_mutex_init(&station->lock, NULL);
    pthread_cond_init(&station->car_arrived, NULL);
    pthread_cond_init(&station->passenger_boarded, NULL);
}

void station_load_car(station *station, int count) {
    pthread_mutex_lock(&station->lock);
    
    station->available_seats = count;

    if (count > 0) {
        pthread_cond_broadcast(&station->car_arrived);
    }

    while ((station->waiting_passengers > 0 && station->available_seats > 0) || 
            station->boarding_passengers > 0) {
        pthread_cond_wait(&station->passenger_boarded, &station->lock);
    }
    
    station->available_seats = 0;
    
    pthread_mutex_unlock(&station->lock);
}

void station_wait_for_car(station *station) {
    pthread_mutex_lock(&station->lock);
    
    station->waiting_passengers++;

    while (station->available_seats == 0) {
        pthread_cond_wait(&station->car_arrived, &station->lock);
    }
    
    station->waiting_passengers--;
    station->available_seats--;
    station->boarding_passengers++;
    
    pthread_mutex_unlock(&station->lock);
}

void station_on_board(station *station) {
    pthread_mutex_lock(&station->lock);
    
    station->boarding_passengers--;
    
    if (station->boarding_passengers == 0) {
        pthread_cond_signal(&station->passenger_boarded);
    }
    
    pthread_mutex_unlock(&station->lock);
}