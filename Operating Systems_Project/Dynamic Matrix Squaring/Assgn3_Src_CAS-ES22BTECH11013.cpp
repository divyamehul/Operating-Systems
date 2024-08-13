#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <atomic>
#include <chrono>

using namespace std;

int counter;         // Counter for the number of rows already allotted;
atomic<int> lock(0); // Declaring the lock atomically
struct calcInfo      // Declaring the structure
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

    struct calcInfo *threads[k]; // Declaring an array of the structure for each thread

    pthread_attr_init(&attr);

    // Starting to split work for the threads now

    counter = 0; // Initializing the share counter to be zero initially

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

    // Preparing to write stuff into the outfile
    ofstream outfile("out.txt", std::ios::out);

    // Writing into the out.txt
    if (outfile.is_open())
    {
        outfile << "The final calculation of matrix C is done now" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                outfile << C[i][j] << " "; // Priting the numbers in the Square Matrix into the outfile
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
        int expected = 0;
        while (!lock.compare_exchange_strong(expected, 1)) // If the exchange is succesfull, it means that the value
        {                                                  // Of lock was same as the expected value - i.e. 0, and
            expected = 0;                                  // Reinitializing to 0            //hence, this thread can acquire the lock. If the exchange
        }                                                  // Was not successful, It would return a 0, and we would be
        ;                                                  // Stuck in busy waiting

        // critical section - entering it once the exchange is made
        if (counter == thread->totalNumberOfRows) // If the counter has already reached the total number
        {                                         // of threads, we exit, as all the work is over
            lock.store(0);
            pthread_exit(0);
        }

        // Now starts the caputring of the lock and picking up the chunk
        int rowStart = counter;          // Storing the initial counter value into rowStart
        counter += thread->sizeOfChunks; // Incrementing the counter
        int rowEnd = counter;            // Giving rowEnd the new value of counter

        lock.store(0); // Making the lock 0 so that other threads can aquire it
        // Critical Section ends

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
