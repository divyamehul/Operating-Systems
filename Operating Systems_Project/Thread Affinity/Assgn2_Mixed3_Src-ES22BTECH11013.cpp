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

    threadTimes = new double[k]; //Creating an array to store the time taken by each thread


    // Dynamic allocation for A and C

    int **A = new int *[n];

    C = new int *[n];

    for (int i = 0; i < n; i++)

    {
    //Allocatting memory for A and C

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

    pthread_t tid[k];//Creating the array of threads

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

        threads[i]->row = new int[n];

        // Partitioning

        threads[i]->p = 0; // p is the number of rows this particular thread is gonna get

        int ct = i;

        while (ct < n)

        {

            threads[i]->row[threads[i]->p] = ct;

            threads[i]->p = threads[i]->p + 1;

            ct += k;
        }
    }

    pthread_attr_init(&attr);

    // Creating K threads now

    for (int i = 0; i < k; i++)

    {

        pthread_create(&tid[i], &attr, runner, threads[i]);
    }

    // Waiting for the K Threads to complete

    for (int i = 0; i < k; i++)

    {

        pthread_join(tid[i], NULL);
    }

   
    double averageTime  = 0; // Declaring averageTime to calculate the average time taken

    for(int i = 0; i < totalThreads; i++)
    {
        averageTime += threadTimes[i]; //Adding the time taken by all th threads into the averageTime
    }

    averageTime /= totalThreads; //Dividing by the total number of threads to obtain the average

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
    
    //Obtaining the duration in milliseconds

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Convert duration to seconds

    threadTimes[thread->threadNumber] = duration.count() / 1000.0;

    pthread_exit(0);
}

