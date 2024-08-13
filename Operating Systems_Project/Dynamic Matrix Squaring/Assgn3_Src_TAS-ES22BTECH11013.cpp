#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <atomic>
#include <chrono>

using namespace std;

int counter;                         // Shared Global Counter for the number of rows already allotted;
atomic_flag lock = ATOMIC_FLAG_INIT; // Initializing the Lock

struct calcInfo // Declaring the structure
{
    int **matrix;
    int totalNumberOfRows;
    int sizeOfChunks; // Size of chunks
};

int **C;

void *runner(void *param);

int main(int argc, char *argv[])
{
    std::chrono::time_point<std::chrono::steady_clock> start, end; // Declaring the start and end variables to calculate the total time

    start = std::chrono::steady_clock::now(); // Starting the time caculator

    ifstream infile("inp.txt", ios::in); // Taking input from the inp.txt file
    int n, k;
    int rowInc;

    if (infile.is_open())
    {
        infile >> n;
        infile >> k;
        infile >> rowInc;
    }

    // Dynamic allocation for A and C
    int **A = new int *[n];
    C = new int *[n];

    for (int i = 0; i < n; i++)
    {
        A[i] = new int[n];
        C[i] = new int[n];
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            infile >> A[i][j]; // Reading the matrix A from the infile
        }
    }

    infile.close();

    pthread_t tid[k];
    pthread_attr_t attr;

    struct calcInfo *threads[k]; // Declaring an array of structures for the threads

    pthread_attr_init(&attr); // Initializing the structures

    // Starting to split work for the threads now

    counter = 0; // Initializing the shared counter to zero

    for (int i = 0; i < k; i++)
    {

        // Initializing and dynamically allocating memory
        threads[i] = new struct calcInfo;
        threads[i]->matrix = A;
        threads[i]->totalNumberOfRows = n;

        threads[i]->sizeOfChunks = rowInc; // We have to assume that roInc perfectly divides totalNumberOfRows
    }
    // Creating the K threads
    for (int i = 0; i < k; i++)
    {
        pthread_create(&tid[i], &attr, runner, threads[i]); // At this point, we start creating these k threads. Each one of these threads have the Matrix, Size of Chunks, Size of matrix(total number of rows), And also the couunter - which is global
    }

    // Waiting for the K Threads to complete
    for (int i = 0; i < k; i++)
    {
        pthread_join(tid[i], NULL);
    }

    end = std::chrono::steady_clock::now(); // Ending the time calculator

    // Calculating duration in milliseconds

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Converting duration to seconds

    double time = duration.count() / 1000.0;

    // Preparing to write stuff into the out.txt file
    ofstream outfile("out.txt", std::ios::out);

    // Writing into the out.txt
    if (outfile.is_open())
    {
        outfile << "The final calculation of matrix C is done now" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                outfile << C[i][j] << " ";
            }
            outfile << endl;
        }
        outfile << "Time taken to calculate square = " << time << " seconds" << endl;

        outfile.close();
    }
    else
    {
        cerr << "Unable to open the file.\n";
    }

    // Deallocating memory
    for (int i = 0; i < n; i++)
    {
        delete[] A[i];
        delete[] C[i];
    }

    delete[] A;
    delete[] C;

    return 0;
}

void *runner(void *param)
{

    struct calcInfo *thread = (struct calcInfo *)param;
    while (true)
    {
        while (lock.test_and_set(memory_order_acquire)) // Test and set returns the value of the lock
            ;                                           // If the lock was initially 0, this indicates that it
              // is free and can be aquired. In this case, we break out
              // of the while loop, and set the lock to 1.
              // If the lock was initially 1, it means it has been taken
              // By some other thread already, and this thread
              // Stays in busy waiting condition

        // critical section
        if (counter == thread->totalNumberOfRows) // Checking if we have already reached the value -
        {                                         // totalNumberOfRows, and exiting if it is so
            lock.clear(memory_order_release);
            pthread_exit(0);
        }

        // Now starts the caputring of the lock and picking up the chunk
        int rowStart = counter;          // Allotting the current value of counter to rowStart
        counter += thread->sizeOfChunks; // Incrementing counter by rowInc
        int rowEnd = counter;            // The rows from rowStart to rowEnd will be computed by this thread

        lock.clear(memory_order_release);
        // Critical Section ends here

        // Remainder Section

        // Computing the multiplication for the required rows
        for (int i = rowStart; i < rowEnd; i++)
        {
            for (int j = 0; j < thread->totalNumberOfRows; j++)
            {
                C[i][j] = 0;
            }

            for (int j = 0; j < thread->totalNumberOfRows; j++)
            {
                for (int k = 0; k < thread->totalNumberOfRows; k++)
                {
                    // Using the multiplication formula
                    C[i][j] += thread->matrix[i][k] * thread->matrix[k][j];
                }
            }
        }
    }
    pthread_exit(0);
}
