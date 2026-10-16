#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "sync_car.h"

#define WATCHDOG_SECONDS 20
#define MAX_PASSENGERS 32

static station st;
static pthread_mutex_t test_lock = PTHREAD_MUTEX_INITIALIZER;
static int seated;          
static int in_transit;      
static int max_in_transit;  
static int board_delay_ms;  

static int failures;

static void sleep_ms(int ms) {
    struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

static void check(int cond, const char *msg) {
    printf("  [%s] %s\n", cond ? "OK" : "FALHA", msg);
    if (!cond) failures++;
}

static void watchdog(int sig) {
    (void)sig;
    const char msg[] = "\n[FALHA] Timeout: possivel deadlock na sincronizacao.\n";
    if (write(STDOUT_FILENO, msg, sizeof msg - 1) < 0)
    _exit(2);
}

static void *passenger_thread(void *arg) {
    (void)arg;

    station_wait_for_car(&st);

    pthread_mutex_lock(&test_lock);
    in_transit++;
    if (in_transit > max_in_transit) max_in_transit = in_transit;
    pthread_mutex_unlock(&test_lock);

    sleep_ms(board_delay_ms);

    pthread_mutex_lock(&test_lock);
    seated++;
    in_transit--;
    pthread_mutex_unlock(&test_lock);

    station_on_board(&st);
    return NULL;
}

static int get_seated(void) {
    pthread_mutex_lock(&test_lock);
    int n = seated;
    pthread_mutex_unlock(&test_lock);
    return n;
}

static void reset_scenario(int delay_ms) {
    station_init(&st);
    seated = 0;
    in_transit = 0;
    max_in_transit = 0;
    board_delay_ms = delay_ms;
}

static void start_passengers(pthread_t *threads, int n) {
    for (int i = 0; i < n; i++)
        pthread_create(&threads[i], NULL, passenger_thread, NULL);
}

static void join_passengers(pthread_t *threads, int n) {
    for (int i = 0; i < n; i++)
        pthread_join(threads[i], NULL);
}

static void wait_until_waiting(int n) {
    for (;;) {
        pthread_mutex_lock(&st.lock);
        int w = st.waiting_passengers;
        pthread_mutex_unlock(&st.lock);
        if (w == n) return;
        sleep_ms(5);
    }
}

static void test_passengers_before_car(void) {
    printf("\nCenario 1: passageiros chegam antes do vagao\n");
    enum { N = 5 };
    pthread_t th[N];

    reset_scenario(10);
    start_passengers(th, N);
    wait_until_waiting(N);
    sleep_ms(50);

    check(get_seated() == 0, "ninguem embarca antes de o vagao chegar");

    station_load_car(&st, N);

    check(get_seated() == N, "vagao so retorna depois de todos sentados");
    join_passengers(th, N);
}

static void test_more_passengers_than_seats(void) {
    printf("\nCenario 2: mais passageiros do que assentos\n");
    enum { N = 10, SEATS = 4 };
    pthread_t th[N];

    reset_scenario(10);
    start_passengers(th, N);
    wait_until_waiting(N);

    station_load_car(&st, SEATS);
    check(get_seated() == SEATS, "primeiro vagao parte cheio (4 sentados)");
    check(st.waiting_passengers == N - SEATS, "os 6 restantes continuam esperando");

    station_load_car(&st, SEATS);
    check(get_seated() == 2 * SEATS, "segundo vagao leva mais 4 passageiros");

    station_load_car(&st, SEATS);
    check(get_seated() == N, "terceiro vagao leva os 2 que sobraram");
    check(st.waiting_passengers == 0, "ninguem ficou esperando");

    join_passengers(th, N);
}

static void test_more_seats_than_passengers(void) {
    printf("\nCenario 3: mais assentos do que passageiros\n");
    enum { N = 3, SEATS = 10 };
    pthread_t th[N];

    reset_scenario(10);
    start_passengers(th, N);
    wait_until_waiting(N);

    station_load_car(&st, SEATS);

    check(get_seated() == N, "vagao parte com lugares vazios apos todos embarcarem");
    check(st.waiting_passengers == 0, "nenhum passageiro ficou na plataforma");
    join_passengers(th, N);
}

static void test_simultaneous_boarding(void) {
    printf("\nCenario 4: embarque simultaneo\n");
    enum { N = 6 };
    pthread_t th[N];

    reset_scenario(150);  
    start_passengers(th, N);
    wait_until_waiting(N);

    station_load_car(&st, N);

    check(max_in_transit == N,
          "todos retornam de wait_for_car antes de qualquer um chamar on_board");
    check(get_seated() == N, "vagao so parte depois de todos sentados");
    join_passengers(th, N);
}

static void test_empty_station(void) {
    printf("\nCenario 5: estacao vazia\n");

    reset_scenario(0);
    station_load_car(&st, 5);
    check(get_seated() == 0, "vagao chega sem ninguem e parte imediatamente");

    station_load_car(&st, 0);
    check(1, "vagao sem assentos livres tambem parte imediatamente");
}

int main(void) {
    signal(SIGALRM, watchdog);
    alarm(WATCHDOG_SECONDS);

    test_passengers_before_car();
    test_more_passengers_than_seats();
    test_more_seats_than_passengers();
    test_simultaneous_boarding();
    test_empty_station();

    printf("\n%s (%d falha(s))\n",
           failures == 0 ? "Todos os cenarios passaram." : "Alguns cenarios falharam.",
           failures);
    return failures == 0 ? 0 : 1;
}
