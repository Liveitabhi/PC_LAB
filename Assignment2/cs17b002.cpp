#include <random>
#include <iostream>
#include <chrono>
#include <omp.h>

using namespace std;
int main(int argc, char const *argv[])
{
    //Taking cmd line inputs
    int n = atoi(argv[1]);
    double t = atof(argv[2]);

    //Creating matrices
    double* matrix = new double[(n+2)*(n+2)];
    double* matrixNew = new double[(n+2)*(n+2)];
    
    //2D Representation to 1D : [i,j] => [i*(size)+j]

    //Initialize the matrix
    double lbound = 0;
    double ubound = 100;
    std::uniform_real_distribution<double> urd(lbound, ubound);
    std::default_random_engine re;

    for(int i=0; i<n+2; i++)
    {
        for(int j=0; j<n+2; j++)
        {
            if(i==0 || i==n+1 || j==0 || j==n+1)
            {
                matrix[i*(n+2)]=0;              //[i,0]
                matrix[i*(n+2)+(n+1)]=0;        //[i,n+1]
                matrix[i]=0;                    //[0,i]
                matrix[(n+1)*(n+2)+i]=0;        //[n+1,i]
            }
            else matrix[i*(n+2)+j] = urd(re);
        }
    }

    //Set OpenMP number of threads
    int totalThreads = 4;
    omp_set_num_threads(totalThreads);

    int iterCount = 0;
    bool flag = true;
    //Start computing time
    auto start = chrono::high_resolution_clock::now();

    while(flag)
    {
        flag = false;
        //Calculate next stage here
        #pragma omp parallel for reduction(||:flag)
        for(int i=0; i<(n+2)*(n+2); i++)
        {
            //1D Representation to 2D : [i] => [(i/size), (i%size)]
            int r = i/(n+2), c = i%(n+2);

            if(r==0 || r==n+1 || c==0 || c==n+1) continue;

            //2D Representation to 1D : [i,j] => [i*(size)+j]
            //matrixNew = matrix[i] + matrix[r-1][c] + matrix[r+1][c] + matrix[r][c-1] + matrix[r][c+1];
            matrixNew[i] = matrix[i] + matrix[(r-1)*(n+2)+c] + matrix[(r+1)*(n+2)+c] + matrix[r*(n+2)+(c-1)] + matrix[r*(n+2)+(c+1)];

            //If element lies at border, there then less than 5 neighbours
            int borderCnt = 0;
            if(r==1 || r==n) borderCnt++;
            if(c==1 || c==n) borderCnt++;
            matrixNew[i] /= (5-borderCnt) ;

            double diff = fabs(matrix[i]-matrixNew[i]);
            matrix[i] = matrixNew[i];
            flag = flag || (diff>=t);
        }

        iterCount++;
    }

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << "Time = " << duration.count() << endl;
    cout << "Number of iterations = " << iterCount << endl;
    cout << "Number of threads = " << totalThreads << endl;

}
