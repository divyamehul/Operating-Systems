#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>
#include <sched.h>
#include <chrono>
using namespace std;

double* threadTimes; 
int totalThreads;

struct calcInfo // Declaring the structure

{
    int **matrix;
    int *row;
    int size;
    int p;
    int threadNumber;
};

int **C;//Pointer for a 2D arrray to store the final squared matrix

void *runner(void *param);

int main(int argc, char *argv[])

{

    
    ifstream infile("inp.txt", ios::in); // Taking input from the inp.txt file

    int n, k, c, bt;

    if (infile.is_open())

    {

        infile >> n;

        infile >> k;

        infile >> c;

        infile >> bt;
    }

    totalThreads = k; 

    threadTimes = new double[k]; // array to store the times of all the threads

    // Dynamic allocation for A and C

    int **A = new int *[n];

    C = new int *[n];

    for (int i = 0; i < n; i++)

    {
    //Allocating memory for A and C

        A[i] = new int[n];

        C[i] = new int[n];
    }

    for (int i = 0; i < n; i++)

    {

        for (int j = 0; j < n; j++)

        {

            infile >> A[i][j];//Storing the input matrix in A
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

        threads[i]->threadNumber = i;

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

    // Creating the K threads

    for (int i = 0; i < k; i++)

    {

        pthread_create(&tid[i], &attr, runner, threads[i]);
    }

    // Waiting for the K Threads to complete

    for (int i = 0; i < k; i++)

    {

        pthread_join(tid[i], NULL);
    }

    
    double averageTime  = 0; // Declaring variable to calculate the Average time

    for(int i = 0; i < totalThreads; i++)
    {
        averageTime += threadTimes[i]; //Adding the time taken by all the threads and storing it in averageTime
    }

    averageTime /= totalThreads; //Dividing averageTime by the total number of threads to obtain the averageTime 

    ofstream outfile("out.txt", std::ios::out);

    // Writing into the out.txt

    if (outfile.is_open())

    {

        outfile << "The final calculation of matrix C is done now" << endl;

        for (int i = 0; i < n; i++)

        {

            for (int j = 0; j < n; j++)

            {

                outfile << C[i][j] << " ";//Printing each number into the outfile
            }

            outfile << endl;
        }

        outfile << "Average Time taken to calculate square = " << averageTime << " seconds" << endl;

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

    std::chrono::time_point<std::chrono::steady_clock> start, end;

    start = std::chrono::steady_clock::now();

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

    end = std::chrono::steady_clock::now();
    
    //calculating the duration in milliseconds

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Convert duration to seconds

    threadTimes[thread->threadNumber] = duration.count() / 1000.0;

    pthread_exit(0);
}

