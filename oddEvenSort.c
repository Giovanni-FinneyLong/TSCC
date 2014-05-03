





#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

#include <mpi.h>

typedef int bool;
#define true 1
#define false 0
//can instead write a pass by reference NOT method (bool' = !bool)

long long int* generateMyRandomList(long long int length, long long int maxVal) {
	long long int* vec = malloc(length * sizeof(long long int));
    int i;
    for (i = 0; i < length; i++) {
		vec[i] = maxVal * rand();
    }
    return vec;
}


//totalListSize = length(vector) * totalprocs
long long int* generateMyNearRandomList( long long int length, int myRank, int totalprocs)
{
	//Start and end mark the range of the function.
	long long int start =  length * myRank;
	long long int end =  length * (myRank + 1);
	long long int* vec = malloc(length * sizeof(long long int));

	int i;
	for(i = start; i < end; i++){
		vec[i] = start + (end - start) * rand();
	}
	return vec;
}

/*
	data is the data to be sorted (localData of the process
	sortSize is the amount of elements in data
*/

void localSort(long long int* data, long long int sortSize)
{
	/*
	when done should send a boolean indicating whether it hasChanged
	0 keeps incrementing a counter as it receives from any source #cores-1 times
	if sum if hasChanged == 0 then done sorting
	*/

	//TODO remember to make sure that cap cores may not receive anything on a certain semi cycle

	//using bubble sort
	int i,j;
	long long int buf;

	for(i = 0; i < sortSize; i++)
	{
		for(j = 0; j < sortSize; j++)
		{

			if(data[j] > data[j + 1])
			{
				buf = data[j+1];
				data[j+1] = data[j];
				data[j] = buf;
			}
		}
	}
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
	if(oddUp)
	{
		if(odd)//want to send data to the above process UNLESS we are the cap
		{//Sending up, receiving from up
			if(!cap)//because 0 isnt odd
			{
				sendVector(myRank + 1, mySize, data);
			}
		}
		else
		{
			sendVector(myRank - 1, mySize, data);

			//Sending down, receiving from down
			//No bound issues
		}
	}
	else
	{
		if(odd){
		sendVector(myRank - 1, mySize, data);
			//Sending down, receiving from down
			//No bound issues
		}
		else
		{
			//Sending up, receiving from up
			if(!cap || myRank == 0)
			{
				//can send/receive
				//sending up, receiving from up
				sendVector(myRank + 1, mySize, data);
			}
		}
	}
	long long int* buf;


	receiveVector(mySize, buf);


	long long int* combinedV = malloc(mySize * 2);



	memcpy(combinedV, data, mySize);
	memcpy(combinedV + (mySize * sizeof(long long int)),buf, mySize);




	if((oddUp && odd) || (!oddUp && !odd))
	{
		//TODO put memcpy here and do isSorted test
		if(!(isSorted(data,mySize) && isSorted(buf,mySize) && data[0] < buf[0]))//else no need to sort or finish exchanging! :D

			{
				memcpy(combinedV, data, mySize);
				memcpy(combinedV + (mySize * sizeof(long long int)),buf, mySize);
				sortDivide(combinedV, data, buf);//not technically necessary to store into buf, but good to track data
			}

	}
	else
	{
		if(!(isSorted(data,mySize) && isSorted(buf,mySize) && buf[0] < data[0]))//else no need to sort or finish exchanging! :D
		{
			memcpy(combinedV, data, mySize);
			memcpy(combinedV + (mySize * sizeof(long long int)),buf, mySize);
			sortDivide(combinedV, buf, data);
		}
	}


	free(combinedV);






	//oddup = !oddup;//need to change oddup in outer loop
	free(buf);
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
void sortDivide(long long int* z, long long int* a, long long int* b,int length)
{
	localSort(z, length);
	a = malloc((length/2) * sizeof(long long int));
	b = malloc((length/2) * sizeof(long long int));
	int i;
	for (i = 0; i < length /2; i++)
	{
		a[i] = z[i];
	}
	for(i = length / 2; i < length; i++)
	{
		b[i] = z[i + (length / 2)];
	}
	free(z);
}



void recieveVector(int k, long long int* vector) {
   MPI_Status status;
   vector = malloc(k * sizeof(long long int));
   MPI_Recv(vector, k, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
}

void sendVector(int sendTo, int k, long long int* vector){
 MPI_Send(vector, k, MPI_LONG_LONG_INT, sendTo, 0, MPI_COMM_WORLD);//TODO double check 0 argument & above)
}



int main(int argc, char *argv[])
{

	//Seed random # gen
	srand(time((void *) 0);//null pointer

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
	long long int maxVal = LLONG_MAX; //The largest value that any n can obtain

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
		size += totalprocs - (size % totalprocs);
		//printf("Enter the max value of a data value(int): \n");
		//scanf("%lld", &maxVal);
    }


	MPI_Bcast(&type, 1, MPI_INT, 0, MPI_COMM_WORLD);//PSOE from MPI_INT being used to represent a bool
    MPI_Bcast(&size, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
	//MPI_Bcast(&maxVal, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);

	odd = myRank % 2;
	mySize = size / totalprocs;




    startwtime = MPI_Wtime();


	//Since scale doesnt deal with remainders, the next length of all localData's will only = size if size % totalprocs == 0

	if(type == true)//Fully random list
	{
		localData = generateMyRandomList(mySize, maxVal);
	}
	else
	{
		//TODO note that the below will make all localData the same size due the int value of scale

		localData = generateMyNearRandomList(mySize, myRank, totalprocs);
	}

	int cycles = 0;
	while(cycles < totalprocs) //set less than p cycles && unordered
	{
		oddUp = cycles % 2;
		transfer(myRank, mySize, oddUp, cap, odd, long long int* localData);


	}
	long long int* allBuf;
	long long int allSize = totalprocs * mySize * sizeof(long long int);

	if(myRank == 0)
	{
		allBuf = (long long int*)malloc(allSize);
	}

	MPI_Gather(localData, mySize,MPI_LONG_LONG_INT, allBuf, mySize, MPI_LONG_LONG_INT,0,MPI_COMM_WORLD);

	if(myRank == 0)
	{
		long long int i;
		printf("First 50 values: ");
		for(i = 0; i < 50; i++)
		{
			printf("%lld, ", allBuf[i]);
		}
		printf("\nLast 50 values: ")
		{
			for(i = allSize - 50; i < allSize; i++)
			{
				printf("lld, ", allBuf[i]);
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
