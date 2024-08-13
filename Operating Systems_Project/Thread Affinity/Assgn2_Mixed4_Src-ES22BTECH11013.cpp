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
    threadTimes = new double[k]; //Creating an array to store the times of each thread
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

 // Creating and Binding the threads

    int b = k / c; //Calculating bas said in the question

    cpu_set_t cpuset;	// Declaring cpuset

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

        if (j < p) //We will bind threads only to the first p=bt/b CPUs

        {

            if (m < b) //Each cpu will only get b threads. Hence, p*b = (bt/b)*b = bt = total number of bounded threads

            {

                // this thread should be bound to cpu j

                int ret = pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset);//Setting the affinity of the thread

                if (ret != 0)

                {

                    cout<<"Error"<<endl;//Handling the error
                }

                m++;//Incrementing thread counter
            }

            if (m == b)

            {

                j++;//If we have allotted b threads to the current cpu, we increment the cpu counter j

                if (j < p)

                {

                    CPU_ZERO(&cpuset);//Emptying the cpuset in order to use the next cpu

                    CPU_SET(j, &cpuset);//Setting cpuset to the new cpu

                    // this thread should be bound to cpu j

                    int ret = pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset);//Setting Affinity

                    if (ret != 0)

                    {

                        cout<<"Error"<<endl;//Error message
                    }

                    m = 1;//Resetting the thread counter
                }
            }
        }
    }

    // Waiting for the K Threads to complete

    for (int i = 0; i < k; i++)

    {

        pthread_join(tid[i], NULL);
    }

    double avg_t1 = 0, avg_t2 = 0; //Declaring variables to store the average time of bounded and normal threads

    for(int i = 0; i < totalThreads; i++)
    {
        if(i < totalThreads / 2)
        {
            avg_t1 += threadTimes[i]; //Adding the time taken by bounded threads
        }
        else
        {
            avg_t2 += threadTimes[i]; // Adding the time taken by normal threads
        }
    }

    avg_t1 /= (totalThreads / 2); //Dividing by the total number of bounded threads to obtain the average
    avg_t2 /= (totalThreads / 2); //Dividing by the total number of normal threads to obtain the average


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

        outfile << "Average Time execution for Core-Bounded threads is = " << (avg_t1) << " seconds" << endl;
        outfile << "Average Time execution for Normal threads is = " << (avg_t2) << " seconds" << endl;

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

    std::chrono::time_point<std::chrono::steady_clock> start, end;

    struct calcInfo *thread = (struct calcInfo *)param;

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
        
        //Finding the duration in milliseconds

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Convert duration to seconds

        threadTimes[thread->threadNumber]= duration.count() / 1000.0;
    

    pthread_exit(0);
}

