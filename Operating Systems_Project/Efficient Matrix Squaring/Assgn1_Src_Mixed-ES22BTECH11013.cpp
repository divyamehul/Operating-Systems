#include <iostream>
#include <fstream>
#include <pthread.h>
#include <ctime>

using namespace std;

struct calcInfo //Declaring the structure
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

    //Reading from the input file

    ifstream infile("inp.txt", ios::in);
    int n, k;
    if (infile.is_open())
    {
        infile >> n;
        infile >> k;
    }

    int **A = new int *[n];
    for (int i = 0; i < n; i++)
    {
        A[i] = new int[n];
        for (int j = 0; j < n; j++)
        {
            infile >> A[i][j];
        }
    }
    infile.close();

    C = new int *[n];
    for (int i = 0; i < n; i++)
    {
        C[i] = new int[n];
    }

    pthread_t tid[k];
    pthread_attr_t attr;

    struct calcInfo *threads[k]; // Creating a pointer array of the structure Threads

    pthread_attr_init(&attr); //Initializing the thread attributes

    for (int i = 0; i < k; i++)
    {

        //Initilaiizng and dynamically allocatting memories
        threads[i] = new struct calcInfo;
        threads[i]->size = n;
        threads[i]->row = new int[n];
        threads[i]->matrix = new int *[n];

        //Doing the partitioning 
        for (int j = 0; j < n; j++)
        {
            threads[i]->matrix[j] = new int[n];
            for (int k = 0; k < n; k++)
            {
                threads[i]->matrix[j][k] = A[j][k];
            }
        }

        threads[i]->p = 0; //Allocating the thread row counter to zero
        int ct = i;
        while (ct < n)
        {
            threads[i]->row[threads[i]->p] = ct;
            threads[i]->p++;
            ct += k;
        }
    }

    //Creating the threads

    for (int i = 0; i < k; i++)
    {
        pthread_create(&tid[i], &attr, runner, threads[i]);
    }

    //Waiting for the threads to complete
    for (int i = 0; i < k; i++)
    {
        pthread_join(tid[i], NULL);
    }
    end = clock();
    double t = end - start;
    double time = t / CLOCKS_PER_SEC;

    //Writing into the output file
    
    ofstream outfile("out.txt", std::ios::out);

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

    for (int i = 0; i < n; i++)
    {
        delete[] A[i];
        delete[] C[i];
        for (int j = 0; j < n; j++)
        {
            delete[] threads[i]->matrix[j];
        }
        delete[] threads[i]->matrix;
        delete[] threads[i]->row;
        delete threads[i];
    }

    delete[] A;
    delete[] C;

    return 0;
}

void *runner(void *param)
{
    struct calcInfo *thread = (struct calcInfo *)param;

    //Computing the required rows

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
                //Using the matrix formula
                C[thread->row[i]][j] += thread->matrix[thread->row[i]][k] * thread->matrix[k][j];
            }
        }
    }

    pthread_exit(0);
}
