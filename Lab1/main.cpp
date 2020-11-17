//
//  main.cpp
//  Lab1
//
//  Created by Pavel Ronaldo on 10/22/20.
//

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <ctime>
#define MATRIX_SIZE 10
#define NUM_THREADS std::thread::hardware_concurrency()

std::mutex m;
std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;


int matrix[MATRIX_SIZE][MATRIX_SIZE];
std::vector<std::thread> threads;

void showMatrix(int matrix[][MATRIX_SIZE]) {
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            std::cout << matrix[i][j] << '\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

void fillMatrix(int matrix[][MATRIX_SIZE]) {
    srand(time(nullptr));
    for (int i = 0; i < MATRIX_SIZcE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = rand() % 10;
        }
    }
}

void flipSide(int matrix[][MATRIX_SIZE]) {
    int half = MATRIX_SIZE / 2;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[half + i][j] = matrix[i][j];
        }
    }
}

void fillMatrixThreaded(int id, int matrix[][MATRIX_SIZE]) {
    for(int i = id * (MATRIX_SIZE/ 2 / NUM_THREADS); i < (id + 1) * (MATRIX_SIZE / 2 / NUM_THREADS); i++) {
        for(int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = rand() % 10;
        }
    }
}

void flipSideThreaded(int id, int matrix[][MATRIX_SIZE]) {
    int half = MATRIX_SIZE / 2;
    int endPoint = half + (half / NUM_THREADS) * (id + 1);
    if(id == NUM_THREADS - 1) {
        endPoint++;
    }
    for(int i = half + (half / NUM_THREADS) * id; i < endPoint; i++) {
        
        for(int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = matrix[i - half][j];
        }
    }
}

void routine(int id, int matrix[][MATRIX_SIZE]) {
    fillMatrixThreaded(id, matrix);
    flipSideThreaded(id, matrix);
}
 
void worker_thread()
{
    // Wait until main() sends data
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{return ready;});
 
    // after the wait, we own the lock.
    std::cout << "Worker thread is processing data\n";
    data += " after processing";
 
    // Send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
 
    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv.notify_one();
}
 
int main()
{
    std::thread worker(worker_thread);
 
    data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
 
    // wait for the worker
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }
    std::cout << "Back in main(), data = " << data << '\n';
 
    worker.join();

    time_t start = clock();
    fillMatrix(matrix);
    showMatrix(matrix);
    // std::cout << '\n';
    flipSide(matrix);
    showMatrix(matrix);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = 0;
        }
    }
    std::cout << "Time taken " << ((clock() - start) / (double)CLOCKS_PER_SEC) << std::endl;
    for(int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(routine, i, matrix));
    }
    start = clock();
    for(int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }
    
    std::cout << "time taken(threaded) " << ((clock() - start) / (double)CLOCKS_PER_SEC) << std::endl;
    showMatrix(matrix);
}
