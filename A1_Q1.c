#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {

        //Set the seed for rand() function
        srand(time(NULL));

        MPI_Init(NULL, NULL);

        int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        int world_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        char processor_name[MPI_MAX_PROCESSOR_NAME];
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);
        /*

        //create the graph for only one process
        if(world_rank==0)
        {
                int n;
                //Take input the number of nodes in the graph
                printf("Enter the nodes for the graph\n");
                scanf("%d", &n);

                //Create the graph.txt file to store the randomly generated graph
                FILE* fp;
                fp = fopen("graph.txt", "w");

                //First store the no of nodes
                fprintf(fp, "%d\n", n);

                //Then store the adjacency matrix
                int tAdjMat[n][n];
                for(int i=0; i<n; i++)
                {
                        for(int j=0; j<n; j++)
                        {
                                if(i>j) tAdjMat[i][j] = tAdjMat[j][i];
                                else tAdjMat[i][j] = rand()%2;
                                fprintf(fp, "%d ", tAdjMat[i][j]);
                        }
                        fprintf(fp, "\n");
                }
                fclose(fp);
        }
        MPI_Barrier(MPI_COMM_WORLD); //To make sure graph is constructed before all processes pass this point



        //Now read the adjacency matrix for every process
        int nodes;
        int adjMat[nodes][nodes];

        FILE* fptr;
        fptr = fopen("graph.txt", "r");
        fscanf(fptr, "%d", &nodes);
        for(int i=0; i<nodes; i++)
        {
                for(int j=0; j<nodes; j++)
                {
                        fscanf(fptr, "%d", &adjMat[i][j]);
                        //printf("%d %d %d %d\n", i, j, world_rank, adjMat[i][j]);
                }
        }
        fclose(fptr);

        MPI_Barrier(MPI_COMM_WORLD);

        */


        /////////////// The above code (written to create a random graph, store it in a file and then read it from there) was not behaving as desired and so, i am hard coding th node and graph.
        int nodes=4;
        int adjMat[4][4] = {{0,1,1,0},
                            {1,0,1,1},
                            {1,1,1,1},
                            {0,1,1,1}};

        int product[4][4] = {{0,1,1,0},
                             {1,0,1,1},
                             {1,1,1,1},
                             {0,1,1,1}};

        int src = 2, des =3;
        int length=1;
        int cnt=nodes/world_size; //No of rows each process will be responsible for in each matrix multiplication

        //Here if a process has rank r, then it will be calculating rows number r*cnt to r*(cnt+1)-1
        //
        //sRow - starting row, eRow - Ending row
        int sRow = world_rank*cnt, eRow = world_rank*(cnt+1);


        //CHECK FOR SHORTEST PATH CONDITION
        //Check if shortest path length is available at this point
        //Every process will send Product matrix value at (s,d) to every other process except itself
        //If no shortest path, then 0 will be sent to every process by every other process
        //So, loop will execute further
        //But if shortest path is present, then one process will send 1 to every other process
        //So, loop will break here for every one
        int check=0;
        MPI_Status status;
        for(int i=0; i<world_size; i++)
        {
                int t=0;
                if(i==world_rank) continue;
                MPI_Send(&product[src][des], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Recv(&t, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                check+=t;
        }
        int flag=0;
        if(check==1)
        {
                if(product[src][des]==1)
                        printf("Shortest path length between %d and %d is %d\n", src, des, length);
                flag=1;
        }



        while(flag==0)
        {
                //CHECK FOR NO SHORTEST PATH CONDITION
                if(length==nodes) break;


                //perform matrix multiplication here
                for(int i=sRow; i<eRow; i++)
                {
                        for(int j=0; j<nodes; j++)
                        {
                                int sum=0;
                                for(int k=0; k<nodes; k++)
                                        sum+=product[i][k] * adjMat[k][j];
                        }
                }

                //Make sure multiplication phase is over for all processes
                MPI_Barrier(MPI_COMM_WORLD);


                //CHECK FOR SHORTEST PATH CONDITION
                int check=0;
                MPI_Status status;
                for(int i=0; i<world_size; i++)
                {
                        int t=0;
                        if(i==world_rank) continue;
                        MPI_Send(&product[src][des], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                        MPI_Recv(&t, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                        check+=t;
                }
                if(product[src][des]==1)
                {
                        printf("Shortest path length between %d and %d is %d\n", src, des, length);
                        break;
                }
                if(check==1) break;




                //Now, the product calculated needs to be sent to every other process
                //Also, their calculations need to be received
                for(int i=0; i<world_size; i++)
                {
                        if(i==world_rank) continue;
                        for(int j=0; j<cnt; j++)
                        {
                                //Send from row number sRow to sRow+cnt-1
                                MPI_Send(&product[sRow+j], nodes, MPI_INT, i, 0, MPI_COMM_WORLD);

                                //Receive from row number i*cnt to i*(cnt+1)-1
                                MPI_Recv(&product[i*cnt+j], nodes, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
                        }
                }
                length = length+1;

                MPI_Barrier(MPI_COMM_WORLD); //Make sure every process has updates values before next iteration starts

        }
        if(world_rank==0 && length==nodes) //To avoid multiple printings
                printf("No path between source and destination\n");

        MPI_Finalize();
}