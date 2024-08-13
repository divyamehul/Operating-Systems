I have submitted 4 Src cpp files - Chunks, Mixed, Self, and Input. 
First execute the input file. Enter N and K - space separated. This will create a random N*N matrix in the inp.txt file. Now, run any one of the 3 files - Chunks, Mixed, or Self. These programs will read input from the inp.txt file - and print the output i.e. the squared matrix in out.txt file.
Thus - the order of working of the files is as follows
1. Run Src(Input) - Enter the input
2. Input is written into inp.txt
3. Run Src(Chunks/Mixed/Self) - It reads the input from inp.txt and executes to find out the square of the matrix
4. out.txt - The final squared matrix, and the time taken to compute it is printed into this file. 