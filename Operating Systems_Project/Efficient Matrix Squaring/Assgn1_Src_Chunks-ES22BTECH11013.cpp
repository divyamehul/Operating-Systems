#include <iostream>
#include <fstream>
#include <ctime>
#include <pthread.h>

using namespace std;

struct calcInfo // Declaring the structure
{
    int **matrix; 
    int *row;
    int size;
    int p;
};

int **C; 

void *runner(void *param);

int main(int argc, char *argv[])
{
    clock_t start, end;
    start = clock();

    ifstream infile("inp.txt", ios::in); // Taking input from the inp.txt file
    int n, k;

    if (infile.is_open())
    {
        infile >> n;
        infile >> k;
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
            infile >> A[i][j];
        }
    }

    infile.close();

    pthread_t tid[k];
    pthread_attr_t attr;

    struct calcInfo *threads[k];

    pthread_attr_init(&attr);

    //Starting to split work for the threads now

    for (int i = 0; i < k; i++)
    {

        //Initializing and dynamically allocating memory
        threads[i] = new struct calcInfo;
        threads[i]->matrix = A;
        threads[i]->size = n;

        //Partitioning into chunks
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

    //Creating the K threads
    for (int i = 0; i < k; i++)
    {
        pthread_create(&tid[i], &attr, runner, threads[i]);
    }

    //Waiting for the K Threads to complete
    for (int i = 0; i < k; i++)
    {
        pthread_join(tid[i], NULL);
    }

    end = clock();
    double t = end - start;
    double time = t / CLOCKS_PER_SEC;
    ofstream outfile("out.txt", std::ios::out);

    //Writing into the out.txt
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

    //Computing the multiplication for the required rows
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
                //Using the multiplication formula
                C[thread->row[i]][j] += thread->matrix[thread->row[i]][k] * thread->matrix[k][j];
            }
        }
    }

    pthread_exit(0);
}
