#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <atomic>
#include <chrono>
using namespace std;

atomic<int> counter(0); // Counter for the number of rows already allotted, have made the counter itself atomic here
struct calcInfo         // Declaring the structure
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
            infile >> A[i][j]; // Takign the matrix A from the infile
        }
    }

    infile.close();

    pthread_t tid[k];
    pthread_attr_t attr;

    struct calcInfo *threads[k]; // Declaring an array of structures for the k threads

    pthread_attr_init(&attr);

    // Starting to split work for the threads now

    counter = 0; // Initializing the shared counter variable to zero

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

    // Preparing to write stuff into the out.txt
    ofstream outfile("out.txt", std::ios::out);

    // Writing into the out.txt
    if (outfile.is_open())
    {
        outfile << "The final calculation of matrix C is done now" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                outfile << C[i][j] << " "; // Writing the calculated square matrix
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
    while (true)
    {

        struct calcInfo *thread = (struct calcInfo *)param;

        // critical section;

        // Now starts the picking up of the chunk
        int rowStart = counter, load();          // Loading the initial value of the counter
        counter.fetch_add(thread->sizeOfChunks); // Atomically incrementing the value of counter
        int rowEnd = counter.load();             // Loading the updated value of counter

        if (rowStart >= thread->totalNumberOfRows) // Checking if we have already crossed the
        {                                          // total number of threads. In this case,
            pthread_exit(0);                       // we can exit this thread
        }
        // End of critical section

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
