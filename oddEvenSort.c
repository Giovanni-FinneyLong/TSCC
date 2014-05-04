




#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <mpi.h>

typedef int bool;
#define true 1
#define false 0

void localSort(long long int* data, long long int sortSize)
{
	/*
	when done should send a boolean indicating whether it hasChanged
	0 keeps incrementing a counter as it receives from any source #cores-1 times
	if sum if hasChanged == 0 then done sorting
	*/

	//TODO remember to make sure that cap cores may not receive anything on a certain semi cycle

	//using bubble sort
	printf("Before localSort:");
	pv(data,sortSize);
	long long int i,j;
	long long int buf;

	for(i = 0; i < sortSize; i++)
	{
		for(j = 0; j < sortSize - 1; j++)
		{

			if(data[j] > data[j + 1])
			{
				buf = data[j+1];
				data[j+1] = data[j];
				data[j] = buf;
			}
		}
}
	printf("After localSort:");
	pv(data, sortSize);
}
bool isSorted(long long int* data, long long int size)
{
	long long int i, last;
	last = data[0];
	for(i = 1; i < size; i++)
	{
		if(data[i] < last)
		{
			return false;
		}
	}
	return true;
}

void transfer(int myRank, int mySize, bool oddUp, bool cap, bool odd, long long int* data)
{
	printf("Starting transfer for process %d\n", myRank);
	long long int* buf;
        //buf = (long long int*)malloc(mySize * sizeof(long long int));		
	printf("Done allocatiing buf for p%d\n", myRank);
	printf("Before transfer:");
	pv(data,mySize);
	
	bool doSend = false;

	if(oddUp)
	{
		if(odd)//want to send data to the above process UNLESS we are the cap
		{//Sending up, receiving from up
			if(!cap)//because 0 isnt odd
			{
			        buf = (long long int*)malloc(mySize * sizeof(long long int));
				doSend = true;
				printf("SendStatement1 by %d\n",myRank);
				sendVector(myRank + 1, mySize, data);
				printf("Process %d is about to receive\n", myRank);
				receiveVector(mySize, buf);
				
			}
		}
		else
		{
			if(myRank != 0)
			{
			        buf = (long long int*)malloc(mySize * sizeof(long long int));
				doSend = true;
				printf("SendStatement2 by %d\n",myRank);
				sendVector(myRank - 1, mySize, data);
				printf("Process %d is about to receive\n", myRank);
				receiveVector(mySize, buf);
			}
			//Sending down, receiving from down
			//No bound issues
		}
	}
	else
	{
		if(odd){
		        buf = (long long int*)malloc(mySize * sizeof(long long int));	
			doSend = true;
			printf("SendStatement3 by %d\n",myRank);
			sendVector(myRank - 1, mySize, data);
			//Sending down, receiving from down
			//No bound issues
			printf("Process %d is about to receive\n", myRank);
			receiveVector(mySize, buf);
		}
		else
		{
			//Sending up, receiving from up
			if(!cap || myRank == 0)
			{
        			buf = (long long int*)malloc(mySize * sizeof(long long int));
				//can send/receive
				doSend = true;
				//sending up, receiving from up
				printf("SendStatement4 by %d\n",myRank);
				sendVector(myRank + 1, mySize, data);
				printf("Process %d is about to receive\n", myRank);
			
				receiveVector(mySize, buf);
			}
		}
	}
	//long long int* buf;
	





	if(doSend)
	{

	



		//receiveVector(mySize, buf);
		printf("P%d done receiving, received vector:", myRank);
		pv(buf, mySize);

		long long int* combinedV = (long long int*)malloc(mySize * 2 * sizeof(long long int));
	

		dbg("Starting memcpy", myRank);
		//memcpy(combinedV, data, mySize);
		//memcpy(combinedV + (mySize * sizeof(long long int)),buf, mySize);
		//dbg("DOne with memcpy", myRank);
	


		if((oddUp && odd) || (!oddUp && !odd))
		{
			//TODO put memcpy here and do isSorted test
			if(!(isSorted(data,mySize) && isSorted(buf,mySize) && data[0] < buf[0]))//else no need to sort or finish exchanging! :D

			{
				memcpy(combinedV, data, mySize * sizeof(long long int));
				printf("P%d is past first memcpy\n", myRank);
				memcpy(&combinedV[mySize],buf, mySize * sizeof(long long int));
				printf("P%d is past second memcpy, will S&D\n", myRank);
				printf("CombinedV after both copies:");
				pv(combinedV,mySize * 2);
				sortDivide(combinedV, data, buf, mySize * 2);//not technically necessary to store into buf, but good to track data
			}

		}
		else
		{
			if(!(isSorted(data,mySize) && isSorted(buf,mySize) && buf[0] < data[0]))//else no need to sort or finish exchanging! :D
			{
				memcpy(combinedV, buf, mySize * sizeof(long long int));
				printf("P%d is past first memcpy\n", myRank);
				memcpy(&combinedV[mySize], data, mySize * sizeof(long long int));
				printf("P%d is past second memcpy, will S&D\n", myRank);
				printf("CombinedV after both copies:");
				pv(combinedV,mySize * 2);
				sortDivide(combinedV, buf, data, mySize * 2);
			}
		}


	
		dbg("Done with memcpys",myRank);

		free(combinedV);

		dbg("Done with first free", myRank);
	

		//oddup = !oddup;//need to change oddup in outer loop
		free(buf);
		dbg("P%d exiting from transfer", myRank);
	}
	else
	{
		printf("P%d did not need to do anything!!!!!\n");
	} 
	
}

//returns true if they are identical
bool compareArrays(long long int* a, long long int* b, int length)
{
	int i;
	for(i = 0; i < length; i++)
	{
		if(a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}
/*
Length is the length of z, the combined unsorted array
Z vector is prefilled, a and b vectors defined but not allocated
*/
void sortDivide(long long int* z, long long int* a, long long int* b,long long int length)
{
	printf("\nBefore S&D:(Length=%lld):",length);
	pv(z, length);
	localSort(z, length);
	//a = malloc((length/2) * sizeof(long long int));
	//b = malloc((length/2) * sizeof(long long int));
	long long int i;
	
	/*
	for (i = 0; i < length / 2; i++)
	{
		a[i] = z[i];
	}
	for(i = length / 2; i < length; i++)
	{
		b[i] = z[i + (length / 2)];
	}
	*/

	memcpy(a, z, (length / 2) * sizeof(long long int));

	memcpy(b,&z[length / 2], (length / 2) * sizeof(long long int));
	


	printf("After S&D(BOTH):");
	pv(z,length);
	printf("After S&D(1st 1/2):");
	pv(a, length/2);
	printf("After S&D(2nd 1/2):");
	pv(b, length/2);
	printf("\n");
}



void receiveVector(int k, long long int* vector) {
   MPI_Status status;
   //vector = (long long int*)malloc(k * sizeof(long long int));
   MPI_Recv(vector, k, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
}

void sendVector(int sendTo, int k, long long int* vector){
 MPI_Send(vector, k, MPI_LONG_LONG_INT, sendTo, 0, MPI_COMM_WORLD);//TODO double check 0 argument & above)
}
void dbg(char* in, int rank)
{
	if(rank == 0)
	{
		printf("DBG:%s\n", in);
	}
	else
	{
		printf("DBGRNK:%d\n", rank);
	}
}
void p(char* in)
{
	dbg(in, 0);
}
void pv(long long int* v, long long int l)
{
	long long int i;
	printf("[ ");
	for(i = 0; i < l; i++)
	{
		printf("%lld ", v[i]);
	}
	printf("]\n");
}
int main(int argc, char *argv[])
{


	

    double startwtime, endwtime;
    int  namelen, myRank, totalprocs;

	char processor_name[MPI_MAX_PROCESSOR_NAME];


	bool odd, cap, sorted, type, oddUp, done;

	//odd is true if myrank is odd
	//cap is true if myrank = (0 || comm_sz -1)
	//sorted is broadcast to 1 when done sorting, all sorted when the sum = comm_sz - 1 (0 doesn't send)
	//type controls whether we generate a fully random list or a near sorted one; TRUE = fully random
	//sendOddUp is true if the odd values are sending up (and evens send down) or vice versa.
	//done is true when no list changes from sorting (check both up and down!!!!!)

	long long int* localData;//Array containing the data to be sorted
	long long int size;// = n, the total number of numbers to be sorted, also the size of all localData concatenated
//	long long int maxVal = LLONG_MAX; //The largest value that any n can obtain

	long long int mySize;
	long long int scale;// n/p = n's per p





    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &totalprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Get_processor_name(processor_name, &namelen);




	if(myRank == 0 || myRank == totalprocs - 1)
	{
		cap = true;
	}
	else
	{
		cap = false;
	}

	sorted = false;

	//Receive input for testing:
    if (myRank == 0) {
		printf("Enter value 0 for random list, 1 for semiRandom: \n");
        scanf("%d", &type);
		printf("Enter the desired size of the vector: \n");
		scanf("%lld", &size);//lld to scan long long int
		size += (totalprocs - (size % totalprocs)) * (size % totalprocs != 0);
		//printf("Enter the max value of a data value(int): \n");
		//scanf("%lld", &maxVal);
    }

	printf("P%d start 1st Bcast\n", myRank);
	MPI_Bcast(&type, 1, MPI_INT, 0, MPI_COMM_WORLD);//PSOE from MPI_INT being used to represent a bool
	printf("P%d start 2nd Bcast\n", myRank);
    MPI_Bcast(&size, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
	//MPI_Bcast(&maxVal, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

	odd = myRank % 2;
	mySize = size / totalprocs;


	sleep(myRank % 3);//So as to get different seeds
	srand(time(NULL));
    	startwtime = MPI_Wtime();
	printf("Started clock,P%d,mySize:%lld\n", myRank,mySize);

	//Since scale doesnt deal with remainders, the next length of all localData's will only = size if size % totalprocs == 0

	if(type == false)//Fully random list
	{
		//Tnis is the method that fails
		//generateMyRandomList(localData,mySize);
		localData = (long long int*)malloc(mySize * sizeof(long long int));
    		int i;
		long long int buff;
    		for (i = 0; i < mySize; i++) {
			//buff = LLONG_MAX * rand();
			buff = rand() % 100;
			printf("%d/%d:%lld\n",i,mySize,buff);
			localData[i] = buff;
    		}


	}
	else
	{
		//TODO note that the below will make all localData the same size due the int value of scale
		
	
		//Start and end mark the range of the function.
		long long int start =  mySize * myRank;
		long long int end =  mySize * (myRank + 1.0);
		localData = (long long int*)malloc(mySize * sizeof(long long int));
		long long int buff;
		int i;
		for(i = start; i < end; i++){
			buff = start + ((end - start) * rand());
			printf("P%d:%d/%lld:%lld\n",myRank,i,end,buff);
			localData[i] = buff;
		}
	}
	dbg("Done gen lists",0);
	printf("List right after gen:\n");
	pv(localData, mySize);
	int cycles = 0;
	while(cycles < totalprocs) 
	{
		printf("Rank:%d Cycle:%d/%d\n", myRank, cycles, totalprocs);
		oddUp = cycles % 2;
		transfer(myRank, mySize, oddUp, cap, odd, localData);
		cycles++;

	}
	long long int* allBuf;
	long long int allSize = totalprocs * mySize;


	if(myRank == 0)
	{
		printf("\nSize of allSize:%lld\n", allSize);

		allBuf = (long long int*)malloc(allSize * sizeof(long long int));
	}

	MPI_Gather(localData, mySize,MPI_LONG_LONG_INT, allBuf, mySize, MPI_LONG_LONG_INT,0,MPI_COMM_WORLD);

	if(myRank == 0)
	{
		long long int i;
		printf("First 50 values: ");
		for(i = 0; i < 50 && i < allSize; i++)
		{
			printf("%lld, ",allBuf[i]);
		}
		printf("\nLast 50 values: \n");
		{
			for(i = allSize - 50; i < allSize && i > 0; i++)
			{
				printf("%lld, ", allBuf[i]);
			}
		}
	}







	//instead send a didnt change state variable on odds xor evens
	//When all are done and time for the next cycle, broadcast a done signal.


    endwtime = MPI_Wtime();
    MPI_Finalize();

    printf("%f seconds to complete.\n", endwtime - startwtime);


    return 0;
}
