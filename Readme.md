# HW1


## Choice of algorithms

To start with the conclusion, in the final version I created an array storing a random cycle of length = #test_size. (e.g. a = {1,2,3,4,0}) We then read an integer from the the array, use that integer as an index to read the next integer. Repeat this process for #step number of times.

I'll mention a few alternatives to this solution, why they are less accurate, and a few issues with my final algorithm. (Detailed comments in code)

### 1

The first thing we might notice is that we are reading one integer at a time instead of a char. The reason we can't read a char array is that the size of the array would have to be limited to 256, which is unpractical for our purposes. 

That said, reading an integer does create a few problems. For example, the naive method of dividing total time by 4 would not work.  

Consider if we are reading from L1, the time to read a single int is 4*time_of_L. 
  
Consider if we are reading from Memory, the time to read a single int, because of spatial locality, is time_of_Memory+3*time_of_L1.

In other words, it means the time of reading the 2nd,3rd,4th byte of an int is constant regardless of where the int is stored.

If we are simply averaging the time we read a byte, there's always a compromise here. Either:

1. We return the time we read one integer and overestimate it in the case of reading from L1 
2. We return the time we read a single byte in the integer and underestimate it in the case of reading L2 and Memory.

We will solve this by subtracting an estimated reading time of three bytes from L1 from the average reading time of an int. This, however, requires that we know the reading time of a byte from L1. My code will ask you to define your own cpu frequency in order to accomplish that.
	
### 2

Another intuitive way would be to read a char at a time and combine them into a larger integer. i.e.

	for (auto i = 0; i < 1024; i++){
		char b1 = v2[16777216 * a1 + 65536 * a2 + 256 * a3 + a4];
		char b2 = v2[16777216 * a2 + 65536 * a3 + 256 * a4 + a1];
		char b3 = v2[16777216 * a3 + 65536 * a4 + 256 * a1 + a2];
		char b4 = v2[16777216 * a4 + 65536 * a1 + 256 * a2 + a3];
		char a1 = v2[16777216 * b1 + 65536 * b2 + 256 * b3 + b4];
		char a2 = v2[16777216 * b2 + 65536 * b3 + 256 * b4 + a4];
		char a1 = v2[16777216 * b3 + 65536 * b4 + 256 * b1 + a4];
		char a1 = v2[16777216 * b4 + 65536 * b1 + 256 * b2 + a4];}
This doesn't work either since multiplcation could take more than 5 cycles, which is enough to affect our measurement.

A similar idea would be to forge four bytes into an integer. 

	int a = (int)(buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]);
	
After some test, this is unrealistic as well.

### 3

The reason we emphasized a certain property of our array (cycle of length n) is that our algorithm would loop at a single byte if v[i] = i for any i. The probability of that is 1/size for a single iteration. Similarly, if v[0] = 1 and v[1] = 0, we will have a two-cycle. Then the OS will simply keep those two values in L1 and loop through them. In general, the chance of a loop to happen before we reach our iteration steps is (iteration steps)/(array size)

Hence the loop is likely to happen when our size is small compared to the iteration steps. 

A permutation would not suffice and we need a stronger condition.

To illustrate how I generate the cycle:

Step 1: generate an identity map a = {0,1,2,3,4}

Step 2 : randomly shuffle it into an arbitrary permutation a = {4,0,3,1,2}

Step 3: From this permutation we generate a cycle that maps 4 to 0, 0 to 3, 3 to 1, 1 to 2, 2 to 4. i.e. b = {3,2,4,1,0}.


One can verify that no matter where we start, we are not going to loop unless the iteration steps exceed array size.


### 4
We now have our algorithm to test reading time. The result can be fluctuating a lot for many reasons: randomness of array, external conditions, step size, etc. There are two methods to compensate for that. First is to run a few times and take the min value. The other is to run a lot more times and take the avg value. I tried both and it turned out not to be a big difference. 

### 5

Some parameters to consider: 

repeat: the number of times to repeat the experiment. You might want to change it based on your statistic method.(min or avg)

steps: the number of iterations in a single experiment to read an integer from memory.

your_average_cpu_frequency: used to minimize the bias introduced by reading integers.


## Graph
![Step is the number of iterations to read a byte from memory](https://github.com/QwertyQwh/CS389HW1/blob/master/compareHW1.png?raw=true)

Step is the number of iterations to read a byte from memory.

The graph will look steeper and steeper because our x-Axis does not grow linearly but exponentially.

There are two places where the reading time more than doubled: from 262144 bytes to  524288 bytes, and from 8388608 bytes to 16777216 bytes. The difference is not as dramatic considering the efficiency difference between L1 and RAM because our miss rate does not raise from 0% to 100% instantaneously. 

From what I got online, the two gaps correspond exactly to L1 cache size (384 Kb) and L3 cache size (9 Mb). The reason L2 cache is not manifested as explicitly as the other two is not clear. It might be that L2 cache does not sit that far from L1 or L3 in terms of efficiency.

