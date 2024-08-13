#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>

using namespace std;

// This program file is just to take input of nr, nw, kr, kw 
int main()
{
    int nw, nr, kw, kr;
    cout << "Enter nr, nw, kr, kw - space separated: ";
    cin >> nr >> nw >> kr >> kw;

    // Writing the values into the inp.txt file
    std::ofstream outfile("inp.txt", std::ios::out);

    if (outfile.is_open())
    {
        outfile << nr << " ";
        outfile << nw << " ";
        outfile << kr << " ";
        outfile << kw << "\n";

        outfile.close(); // Closing the file after writing
        std::cout << "File written successfully.\n";
    }
    else
    {
        std::cerr << "Unable to open the file.\n";
    }

    return 0;
}
