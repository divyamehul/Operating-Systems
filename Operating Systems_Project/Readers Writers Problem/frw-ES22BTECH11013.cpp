#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <random>
#include <time.h>
#include <mutex>

using namespace std;

double uCS = 0.01;//Declaring the sleep times globally
double uRem = 0.005;

typedef struct _rwlock_t
{
    sem_t writelock;
    sem_t rmutex;
    sem_t serviceQueue; // Service queue of readers and writers
    int readers;        // To maintain the count of readers

} rwlock_t;

sem_t filemutex;

double *timeReaders; // Pointer to the array to store the times to get into CS
double *timeWriters;

struct threadInfo // Structure to pass on the thread info to the reader and writer threads functions
{
    int n;
    int threadId;
} threadInfo;

rwlock_t *rw; // Declaring the global lock strcture

double exponentialRandom(double mean) // Exponential sleep function
{
    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<> dist(1.0 / mean);
    return dist(gen);
}

void rwlock_init(rwlock_t *rw) // Initializing the semaphores
{
    rw->readers = 0;
    sem_init(&rw->rmutex, 0, 1);
    sem_init(&rw->writelock, 0, 1);
    sem_init(&rw->serviceQueue, 0, 1);
}

void *reader(void *arg)
{
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    ofstream outfile; // Declare outfile outside the loop
    for (int i = 0; i < thread.n; i++)
    {
        auto now1 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time1 = chrono::system_clock::to_time_t(now1);

        // Acquire the file mutex
        sem_wait(&filemutex);
        // Open the file
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file
        outfile << (i + 1) << "th CS request by reader thread " << thread.threadId << " at " << ctime(&current_time1) << endl;
        // Close the file
        outfile.close();
        // Release the file mutex
        sem_post(&filemutex);

        // Acquiring the lock
        sem_wait(&rw->serviceQueue);
        sem_wait(&rw->rmutex);
        rw->readers++;
        if (rw->readers == 1)
            sem_wait(&rw->writelock);
        sem_post(&rw->serviceQueue);
        sem_post(&rw->rmutex);

        // We are inside the CS now

        auto now2 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time2 = chrono::system_clock::to_time_t(now2);
        // Re-acquire the file mutex
        sem_wait(&filemutex);
        // Open the file again
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file again
        outfile << (i + 1) << "th CS entry by reader thread " << thread.threadId << " at " << ctime(&current_time2) << endl;
        // Close the file again
        outfile.close();
        // Release the file mutex
        sem_post(&filemutex);

        // Sleeping to simulate the execution of CS
        double randCSTime = exponentialRandom(uCS);
        sleep(randCSTime);

        // CS exection is over, we are exiting it now
        sem_wait(&rw->rmutex);
        rw->readers--;
        if (rw->readers == 0)
            sem_post(&rw->writelock);
        sem_post(&rw->rmutex);

        auto now3 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time3 = chrono::system_clock::to_time_t(now3);
        // Re-acquire the file mutex
        sem_wait(&filemutex);
        // Open the file again
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file again
        outfile << (i + 1) << "th CS exit by reader thread " << thread.threadId << " at " << ctime(&current_time3) << endl;
        // Close the file again
        outfile.close();

        // Release the file mutex
        sem_post(&filemutex);

        // Mainting count of readers, and calculating the time taken to acquire the lock
        double duration = chrono::duration<double>(now2 - now1).count();
        timeReaders[(thread.threadId - 1) * thread.n + i] = duration;

        // Sleeping to simulate the execution of the exit section
        double randRemTime = exponentialRandom(uRem);
        sleep(randRemTime);
    }
    return NULL;
}

void *writer(void *arg)
{
    struct threadInfo thread;
    thread = *((struct threadInfo *)arg);
    ofstream outfile; // Declare outfile outside the loop
    for (int i = 0; i < thread.n; i++)
    {
        auto now1 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time1 = chrono::system_clock::to_time_t(now1);
        // Acquire the file mutex
        sem_wait(&filemutex);
        // Open the file
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file
        outfile << (i + 1) << "th CS request by writer thread " << thread.threadId << " at " << ctime(&current_time1) << endl;
        // Close the file
        outfile.close();
        // Release the file mutex
        sem_post(&filemutex);

        // Code to acquire the Lock
        sem_wait(&rw->serviceQueue);
        sem_wait(&rw->writelock);

        sem_post(&rw->serviceQueue);

        // We are now into the CS

        auto now2 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time2 = chrono::system_clock::to_time_t(now2);
        // Re-acquire the file mutex
        sem_wait(&filemutex);
        // Open the file again
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file again
        outfile << (i + 1) << "th CS entry by writer thread " << thread.threadId << " at " << ctime(&current_time2) << endl;
        // Close the file again
        outfile.close();
        // Release the file mutex
        sem_post(&filemutex);

        // Sleeping to simulate the execution of the CS
        double randCSTime = exponentialRandom(uCS);
        sleep(randCSTime);

        // Releasing the Lock
        sem_post(&rw->writelock);

        auto now3 = chrono::system_clock::now();
        // Convert to a time_point representing the current time
        time_t current_time3 = chrono::system_clock::to_time_t(now3);
        // Re-acquire the file mutex
        sem_wait(&filemutex);
        // Open the file again
        outfile.open("FairRW-log.txt", std::ios::out | std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open the file.\n";
            return NULL; // Return from the function if file opening fails
        }
        // Write to the file again
        outfile << (i + 1) << "th CS exit by writer thread " << thread.threadId << " at " << ctime(&current_time3) << endl;
        // close the file again
        outfile.close();

        // Releasing the file mutex
        sem_post(&filemutex);

        // Maintaining the count of Writers, and the time taken to acquire the Lock
        double duration = chrono::duration<double>(now2 - now1).count();
        timeWriters[((thread.threadId - 1) * thread.n) + i] = duration;

        // Sleeping to simulate the execution of the exit section
        double randRemTime = exponentialRandom(uRem);
        sleep(randRemTime);
    }
    return NULL;
}

int main()
{
    int numberReaders; // To count the total number of critical sections requests by readers
    int numberWriters; // To count the total number of critical sections requuests by writers

    remove("FairRW-log.txt");

    ifstream infile("inp.txt", ios::in); // Taking input from the inp.txt file
    int nw, nr, kw, kr;
    if (infile.is_open())
    {
        infile >> nr;
        infile >> nw;
        infile >> kr;
        infile >> kw;
    }

    numberReaders = nr * kr;
    numberWriters = nw * kw;
    timeReaders = new double[numberReaders]; // Array to store the time of each reader call to the CS
    timeWriters = new double[numberWriters]; // Array to store the time of each reader call to the CS

    pthread_t readers[nr];
    pthread_t writers[nw];
    struct threadInfo readersInfo[nr];
    struct threadInfo writersInfo[nw];
    rw = new rwlock_t;
    rwlock_init(rw);
    sem_init(&filemutex, 0, 1); // Initialixing the file mutex - to Write into the outfile without race conditions
    // Create reader threads
    for (int i = 0; i < nr; i++)
    {
        readersInfo[i].n = kr;           // Passing kr to the thread
        readersInfo[i].threadId = i + 1; // Passing the thread ID
        pthread_create(&readers[i], NULL, reader, &readersInfo[i]);
    }
    // Create writer threads
    for (int i = 0; i < nw; i++)
    {
        writersInfo[i].n = kw;           // Passing kw to the thread
        writersInfo[i].threadId = i + 1; // Passing the thread ID
        pthread_create(&writers[i], NULL, writer, &writersInfo[i]);
    }
    // Join reader threads
    for (int i = 0; i < nr; i++)
    {
        pthread_join(readers[i], NULL);
    }
    // Join writer threads
    for (int i = 0; i < nw; i++)
    {
        pthread_join(writers[i], NULL);
    }

    double avgReaders = 0; // Initializing variables to store calculate the avg time
    double avgWriters = 0;

    for (int i = 0; i < nw * kw; i++)
    {
        avgWriters += timeWriters[i]; // First adding all the times
    }

    for (int i = 0; i < nr * kr; i++)
    {
        avgReaders += timeReaders[i];
    }

    double worstReaders = 0; // Declaring variables to calculate the worst time taken by a thread
    double worstWriters = 0;
    double sum = 0;
    for (int i = 0; i < nr; i++)
    {
        sum = 0;
        for (int j = 0; j < kr; j++)
        {
            sum += timeReaders[(i * kr) + j]; // First summing over all the kr requests by a particular thread
        }
        worstReaders = max(worstReaders, sum); // Finding the max/worst time taken by a thread
    }
    for (int i = 0; i < nw; i++)
    {
        sum = 0;
        for (int j = 0; j < kw; j++)
        {
            sum += timeWriters[(i * kw) + j]; // First summing over all the kw requests by a particular thread
        }
        worstWriters = max(worstWriters, sum); // Finding the max/worst time taken by a thread
    }

    avgWriters /= (nw * kw); // Finding the average by dividing the total sum with the total number of CS calls
    avgReaders /= (nr * kr);

    worstWriters /= kw; // Finding the average by dividing sum of times for a particular thread by the number of requests made by thaht thread
    worstReaders /= kr;

    ofstream outfile;
    outfile.open("Average_time.txt", std::ios::out | std::ios::app);
    if (!outfile.is_open())
    {
        cerr << "Unable to open the file.\n";
        return 0; // Return from the function if file opening fails
    }
    // Write to the file again
    outfile << "Printing the data for Fair Solution:" << endl;
    outfile << "Average time in ms taken by each reader thread to get CS = " << avgReaders * 1000 << endl; // Multiplying with 1000 to convert to milliseconds
    outfile << "Average time in ms taken by each writer thread to get CS = " << avgWriters * 1000 << endl;
    outfile << "Worst case time in ms taken by reader thread to get CS = " << worstReaders * 1000 << endl;
    outfile << "Worst case time in ms taken by writer thread to get CS = " << worstWriters * 1000 << endl
            << endl;

    // close the file again
    outfile.close();

    return 0;
}
