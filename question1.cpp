#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <semaphore.h>

using namespace std;

const int N = 5;  // number of diners the restaurant can hold

mutex mtx;
sem_t frontDoor, backDoor, finished, started;
atomic<int> num_diners_waiting(0);
atomic<int> num_diners_in_restaurant(0);
atomic<int> num_diners_finished(0);

void diner_thread(int id) {
    while(true) {
        cout << "Diner " << id << " is waiting." << endl;
        num_diners_waiting++;
        
        // wait for the restaurant to open its front door
        sem_wait(&frontDoor);
        
        cout << "Diner " << id << " has entered the restaurant." << endl;
        
        num_diners_waiting--;
        num_diners_in_restaurant++;

        if (num_diners_in_restaurant == N) {
            // notify the restaurant to start service
            sem_post(&started);  
        }

        // wait for the restaurant to open its back door
        sem_wait(&backDoor);

        cout << "Diner " << id << " has left the restaurant." << endl;
        
        num_diners_in_restaurant--;
        num_diners_finished++;

        if (num_diners_finished == N) {
            num_diners_finished = 0;
            // notify the restaurant to prepare for the next batch
            sem_post(&finished);
        }
    }
}

void restaurant_thread() {
    // initialize all semaphores to 0.
    sem_init(&started, 0, 0);
    sem_init(&finished, 0, 0);
    sem_init(&backDoor, 0, 0);
    sem_init(&frontDoor, 0, 0);
    while (true) {
        // open front door ro let diners in
        for(int i = 0; i < N; i++) sem_post(&frontDoor);
        
        sem_wait(&started);

        cout << endl << "The restaurant is starting service." << endl << endl;
        
        // simulate service time
        this_thread::sleep_for(chrono::seconds(2));  

        // open the back door to let diners out
        for(int i = 0; i < N; i++) sem_post(&backDoor);
        
        // wait for all diners to leave before starting the next batch
        sem_wait(&finished);
        
        cout << endl << "The restaurant is finished serving." << endl << endl;
    }
}

int main() {
    int n = N*3;
    thread restaurant(restaurant_thread);
    thread diners[n];

    // creating n diner thread, n = N*3
    for (int i = 0; i < n; i++) {
        diners[i] = thread(diner_thread, i);
    }

    // wating for all threads to complete
    for (int i = 0; i < n; i++) {
        diners[i].join();
    }
    // wating dor restaurant thread to finish
    restaurant.join();
    return 0;
}
