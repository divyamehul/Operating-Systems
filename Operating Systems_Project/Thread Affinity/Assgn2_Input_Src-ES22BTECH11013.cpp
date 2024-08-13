#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>

using namespace std;

//This program file is just to take input of N and K  and generate the matrix A
int main()
{
    int n, k, c, bt;
    cout<<"Enter n,k,c,bt - space separated: ";
    cin>>n>>k>>c>>bt;
    
    //Writing into the inp.txt file
    std::ofstream outfile("inp.txt", std::ios::out);

    if (outfile.is_open())
    {
        outfile << n << " ";
        outfile << k << " ";
        outfile << c << " ";
        outfile << bt << "\n";

        srand(12); 

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                outfile << rand() % 100 << " "; // Generating random numbers between 0 and 99
            }
            outfile << "\n";
        }

        outfile.close(); // Closing the file after writing
        std::cout << "File written successfully.\n";
    }
    else
    {
        std::cerr << "Unable to open the file.\n";
    }

    return 0;
}

