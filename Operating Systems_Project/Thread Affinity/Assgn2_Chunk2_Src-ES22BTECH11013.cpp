#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <sched.h>
#include <chrono>

using namespace std;

struct calcInfo // Declaring the structure
{
    int **matrix;
    int *row;
    int size;
    int p;
};

int **C; // Pointer for a 2D arrray to store the final squared matrix

void *runner(void *param);

int main(int argc, char *argv[])
{
    std::chrono::time_point<std::chrono::steady_clock> start, end; // Declaring the start and end variables to calculate the total time
    start = std::chrono::steady_clock::now();

    ifstream infile("inp.txt", ios::in); // Taking input from the inp.txt file
    int n, k, c, bt;

    if (infile.is_open())
    {
        infile >> n;
        infile >> k;
        infile >> c;
        infile >> bt;
    }

    // Dynamic allocation for A and C
    int **A = new int *[n];
    C = new int *[n];

    for (int i = 0; i < n; i++)
    {
        // Allocating memory for A and C
        A[i] = new int[n];
        C[i] = new int[n];
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            infile >> A[i][j]; // Storing the input matrix in A
        }
    }

    infile.close();

    pthread_t tid[k]; // Creating the array of threads
    pthread_attr_t attr;

    struct calcInfo *threads[k];

    // Starting to split work for the threads now

    for (int i = 0; i < k; i++)
    {

        // Initializing and dynamically allocating memory
        threads[i] = new struct calcInfo;
        threads[i]->matrix = A;
        threads[i]->size = n;

        // Partitioning into chunks
        threads[i]->p = 0;
        if (i == k - 1)
        {
            threads[i]->p = n - (k - 1) * (n / k);
        }
        else
        {
            threads[i]->p = n / k;
        }

        threads[i]->row = new int[threads[i]->p];

        int t = (i * (n / k));

        for (int j = 0; j < threads[i]->p; j++)
        {
            threads[i]->row[j] = t;
            t++;
        }
    }

    pthread_attr_init(&attr);

    // Creating and Binding the threads

    int b = k / c; // Calculating bas said in the question

    cpu_set_t cpuset; // Declaring cpuset

    CPU_ZERO(&cpuset); // Initializing cpuset to empty set

    int p;

    p = bt / b; // p is the number of cores that will be assigned bounded threads

    // p is the number of cores in the set

    int j = 0; // counter for p

    CPU_SET(j, &cpuset);

    int m = 0; // counter for threads

    for (int i = 0; i < k; i++)

    {

        pthread_create(&tid[i], &attr, runner, threads[i]); // First, create the thread

        if (j < p) // We will bind threads only to the first p=bt/b CPUs

        {

            if (m < b) // Each cpu will only get b threads. Hence, p*b = (bt/b)*b = bt = total number of bounded threads

            {

                // this thread should be bound to cpu j

                int ret = pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset); // Setting the affinity of the thread

                if (ret != 0)

                {

                    cout << "Error" << endl; // Handling the error
                }

                m++; // Incrementing thread counter
            }

            if (m == b)

            {

                j++; // If we have allotted b threads to the current cpu, we increment the cpu counter j

                if (j < p)

                {

                    CPU_ZERO(&cpuset); // Emptying the cpuset in order to use the next cpu

                    CPU_SET(j, &cpuset); // Setting cpuset to the new cpu

                    // this thread should be bound to cpu j

                    int ret = pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset); // Setting Affinity

                    if (ret != 0)

                    {

                        cout << "Error" << endl; // Error message
                    }

                    m = 1; // Resetting the thread counter
                }
            }
        }
    }

    // Waiting for the K Threads to complete
    for (int i = 0; i < k; i++)
    {
        pthread_join(tid[i], NULL);
    }

    end = std::chrono::steady_clock::now();
    // Calculate duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // Convert duration to seconds
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
                outfile << C[i][j] << " "; // Printing each number into the outfile
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

    // Computing the multiplication for the required rows
    for (int i = 0; i < thread->p; i++)
    {
        for (int j = 0; j < thread->size; j++)
        {
            C[thread->row[i]][j] = 0;
        }

        for (int j = 0; j < thread->size; j++)
        {
            for (int k = 0; k < thread->size; k++)
            {
                // Using the multiplication formula
                C[thread->row[i]][j] += thread->matrix[thread->row[i]][k] * thread->matrix[k][j];
            }
        }
    }

    pthread_exit(0);
}
