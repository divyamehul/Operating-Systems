#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <atomic>
#include <chrono>

using namespace std;

int counter;         // Counter for the number of rows already allotted;
atomic<int> lock(0); // Declaring the lock atomically
bool *waiting;       // Pointer to an artray which stores the waiting status of all the threads - to implement bounded waiting
struct calcInfo      // Declaring the structure
{
    int **matrix;
    int totalNumberOfRows;
    int sizeOfChunks; // Size of chunks
    int threadNumber;
    int numberOfThreads;
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
            infile >> A[i][j]; // Reading the matrix A from the input file
        }
    }

    infile.close();

    pthread_t tid[k];
    pthread_attr_t attr;

    struct calcInfo *threads[k]; // Declaring the array of structure threads

    pthread_attr_init(&attr);

    // Starting to split work for the threads now

    counter = 0;           // Initializing the shared counter to be zero initially
    waiting = new bool[k]; // Allocating memory to the waiting array
    for (int i = 0; i < k; i++)
    {
        waiting[i] = false; // Initializing the waiting status of all threads to be false
    }

    for (int i = 0; i < k; i++)
    {

        // Initializing and dynamically allocating memory
        threads[i] = new struct calcInfo;
        threads[i]->matrix = A;
        threads[i]->totalNumberOfRows = n;
        threads[i]->threadNumber = i;
        threads[i]->numberOfThreads = k;

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

    ofstream outfile("out.txt", std::ios::out);

    // Writing into the out.txt
    if (outfile.is_open())
    {
        outfile << "The final calculation of matrix C is done now" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                outfile << C[i][j] << " "; // Writing the value of squared matrix C into the outfile
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
        int i = thread->threadNumber;
        int j;
        waiting[i] = true; // Making the initial waiting condition of this thread to be true
        int expected = 0;
        int key = 1;
        while (waiting[i] && key == 1)                        // Keeping it in busy waiting until key becomes 0(Exchange is done),
        {                                                     // or some other thread makes its waiting condition to be false
            key = !lock.compare_exchange_strong(expected, 1); // Making the value of the key to be the value returned by CAS
            expected = 0;                                     // reinitializing the expected value
        }
        waiting[i] = false; // Marking it to be non waiting now

        // critical section;
        if (counter == thread->totalNumberOfRows) // Checking whether we have already reached the total number of threads
        {                                         // In that case, we release the lock and exit
            waiting[i] = false;
            lock.store(0);
            pthread_exit(0);
        }

        // Now starts the caputring of the lock and picking up the chunk
        int rowStart = counter;          // Assigning the initial value of counter to rowStart
        counter += thread->sizeOfChunks; // Incrementing the value of the counter
        int rowEnd = counter;            // Allotting the update value of counter to rowEnd

        j = (i + 1) % thread->numberOfThreads;
        while (j != i && !waiting[j]) // We keep looking for a thread trhat is in waiting condition
        {
            j = (j + 1) % thread->numberOfThreads; // Keep incrementing until we find a thread
        }

        if (j == i) // If no threads are in waiting, we come back to i. In this case, we release the lock
        {
            lock.store(0);
        }
        else
        {
            waiting[j] = false; // We find a thread whose waiting is true. We make its waiting condition to be false
        }                       // - allowing it to now enter its CS

        // End of Critical Section
        //  Computing the multiplication for the required rows
        for (int m = rowStart; m < rowEnd; m++)
        {
            for (int n = 0; n < thread->totalNumberOfRows; n++)
            {
                C[m][n] = 0;
            }

            for (int n = 0; n < thread->totalNumberOfRows; n++)
            {
                for (int k = 0; k < thread->totalNumberOfRows; k++)
                {
                    // Using the multiplication formula
                    C[m][n] += thread->matrix[m][k] * thread->matrix[k][n];
                }
            }
        }
    }
    pthread_exit(0);
}
