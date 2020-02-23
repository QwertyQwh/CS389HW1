#include <iostream>
#include <chrono>
#include <random>
#include "Read.h"

int main()
{
	const int repeat =99;
	// One way is to take the minimum of several runs to avoid an extremely large value. However, having too many repeats might lead to small extreme values.
	// If the repeat is sufficiently large, then taking average works as well.	
	double min_val; // This is to store the minimum read time for each array size
	const int steps = 1024;
	std::cout << "# Bytes    time"<<'\n';
	
	for (auto i = 256; i < pow(2, 25); i *= 2) // run the loop on all array sizes, multiplying it by 2 each time.
	{
		min_val = read(i, steps);// Assign an initial value to min_val
		for (auto k = 0; k < repeat; k++)
		{
			double new_val = read(i, steps); // Do a new timing 
			if(new_val<min_val)
			{
				min_val = new_val;// Compare the new timing to the best we currently get and replaced it if needed.
			}
		}
		std::cout << i* 4 << "      " <<min_val << '\n'; //Output the data one line at a time
	}
	return 0;
}

double read(int size, int steps)
{
    /*
    This function reads value from an integer array of size  = _size for steps times, then return the average time reading a single byte.
    Note the purpose is to read completely randomly, we designed a mechanism to do that. 
    We did not return the value of reading an integer or a single byte (which will be our value divided by 4) because we are always reading four consecutive bytes. 
    Consider if we are reading from L1, the time to read a single int is 4*time_of_L1
    Consider if we are reading from Memory, the time to read a single int, because of spatial locality, is time_of_Memory+3*time_of_L1
    In other words, it means the time of reading the 2nd,3rd,4th byte of an int is constant regardless of where the int is stored.
    If we are simply averaging the time we read a byte, there's always a compromise here. Either
    1. We return the time we read one integer and overestimate it in the case of reading from L1 
    2. We return the time we read a single byte in the integer and underestimate it in the case of reading L2 and Memory.
	We will solve this by subtracting an estimated reading time of three bytes from L1 from the average reading time of an int
	*/

	//First Part: create a random cycle of length = size.
	//Average cpu Frequency on my computer is 2.2-4 GHz, you need to figure out yours!!!!!!!!!!
	const double your_average_cpu_frequency = 3;
	const double read_from_L1 = 1 / your_average_cpu_frequency;
    // v1 is a temporary buffer we used to first generate a bijection
	int* v1 = (int*)malloc(size * sizeof(int));
	//v is the actual array we are gonna read from 
	int* v = (int*)malloc(size * sizeof(int));
    //initialize v1 as the identity map
	for (auto n = 0; n < size; n++)
	{
		v1[n] = n;
	}
	// now random shuffle v1 into a random permutation
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::shuffle(v1, v1 + size, gen);
	// the algorithm to generate v from v1, v is guaranteed to be a cycle of length n as we can verify.
	for (auto n = 0; n < size-1; n++)
	{
		v[v1[n]] = v1[n + 1];
	}
	v[v1[size - 1]] = v1[0];


    // Second part: the actual reading process and timing
	//initialize the value(index) to read from.
	int a1 = v[0];
    // start timing 
	auto const t1 = std::chrono::high_resolution_clock::now();
    // actual reading process, there are three things that take cpu cycles 
    // 1. increment i and comparing i to the value in another register
    // 2. read a value from the array (which is what we are trying to time)
    // 3. move the value to a register
    // However, as 1 and 3 take negligible time to compute when we are reading from anywhere except L1 cache, even in L1 it's not significant.
	for (auto i = 0; i < steps; i++)
	{
		a1 = v[a1]; //first we read out a random value from a random index, Then we use that random value as index to read another random value. Repeat. 
	}
    // Stop timing
	auto const t2 = std::chrono::high_resolution_clock::now();


	//Third Part: compute and output
	double const duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()*1000;
	// Don't forget to free the arrays!
	free(v1);
	free(v);
	// We are returning (the average time of reading an int from wherever it is ) minus 3 * the time it takes to read from L1.
	return duration / static_cast<double>(steps) - 3*read_from_L1;
}



