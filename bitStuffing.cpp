/*
* Author: Sean Halnon
*  Provide functions to take a 4 byte data chunk and do bit stuffing and unstuffing. 
*	when stuffed, wherever a sequence of 5 ones is found, a 0 is inserted after it
*	unstuffing reverses that process. 
*/


#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <time.h> 

void get_random_bytes(unsigned char** data_bits)
{
	printf("int get_random_bytes\n");
	*data_bits = (unsigned char*)malloc(sizeof(unsigned char) * 4);
	unsigned int r = rand();
	printf("r = %d\n",r);
	for(int i=0; i<4; i++)
	{
		
		(*data_bits)[i] = ((unsigned char*)(&r))[i];

	}
}


char* get_binary_string(unsigned char byte)
{

	char* bit_string = (char*) malloc(sizeof(char)*9);
	bit_string[8] = '\0';

	int z = 128;
	for(int i=0; i<8; i++)
	{
		bit_string[i] = ((byte & z) ? '1' : '0');
		z >>=1;
	}

	return bit_string;
}


void print_binary_uint(unsigned int ui)
{
	unsigned char four_bytes[4];
	memcpy(four_bytes,&ui,4);

	for(int i=0; i<4; i++)
	{
		printf("%s ",get_binary_string(four_bytes[3-i]));

	}
	printf("\n");
}

void print_binary_ulli(unsigned long long int ulli)
{
	unsigned char eight_bytes[8];
	memcpy(eight_bytes,&ulli,8);

	printf("\nstuffed:");
	for(int i=0; i<8; i++)
	{
		printf(" %s ",get_binary_string(eight_bytes[7-i]));

	}	
}

void print_binary_stuffed_bytes(unsigned long long int ulli)
{
	unsigned char eight_bytes[8];
	memcpy(eight_bytes,&ulli,8);

	printf("\nstuffed:");
	for(int i=3; i<8; i++)
	{
		printf(" %s ",get_binary_string(eight_bytes[7-i]));

	}
}

void stuff_bits(unsigned int i_data_bits, unsigned char** stuffed_bytes)
{
	unsigned long long int  li_data_bits = i_data_bits;
	unsigned long long int all_ones = 0xffffffffffffffff; //a binary string of 64 1s that will be the basis for creating the bit masks

	int max = sizeof(unsigned int) * 8;//number of bits to iterate over. at this point the left most 32 bits are all 0, only the first 32 are of concern. 
	int num_bits_inserted = 0;
	for(int i=0; i<max; i++)
	{
		if( ((i_data_bits >> i) & (unsigned int)31) == (unsigned int)31)//31 in binary is 00011111
		{
			printf("got 5 consecutive bits!\n");
			unsigned long long int bit_mask;
			unsigned long long int left_bits;
			unsigned long long int right_bits;

			bit_mask = all_ones << (i + num_bits_inserted);//get a bit mask that will allow us to isolate the bits to the left of the insertion
			left_bits = li_data_bits & bit_mask;//isolate the bits left of the insertion
			bit_mask = ~bit_mask;//complement the mask so that it now can isolate the bits to the right of the insertion
			right_bits = li_data_bits & bit_mask;//isolate the right bits
			left_bits = left_bits << 1;//shifting once to the left effectively inserts a 0 at the right spot
			li_data_bits = left_bits | right_bits;//put back the right side bits
			num_bits_inserted++;//keep track of bits inserted
			i+=5;
		}
	}

	print_binary_stuffed_bytes(li_data_bits);

	unsigned char eight_bytes[8];
	memcpy(eight_bytes,&li_data_bits,8);
	memcpy(*stuffed_bytes,eight_bytes,5);


}


unsigned int unstuff_bits(unsigned char** stuffed_bytes)
{
	unsigned long long int  li_data_bits;
	memcpy(&li_data_bits,*stuffed_bytes,5);
	unsigned long long int all_ones = 0xffffffffffffffff; //a binary string of 64 1s

	for(int i=0; i<(sizeof(unsigned long long int) * 5); i++)
	{
		if( ((li_data_bits >> i) & (unsigned long long int)31) == (unsigned long long int)31)//31 in binary is 00011111
		{
			unsigned long long int bit_mask;
			unsigned long long int left_bits;
			unsigned long long int right_bits;

			bit_mask = all_ones << (i);//get a bit mask that will allow us to isolate the bits to the left of the insertion
			left_bits = (li_data_bits & bit_mask)>>1;//put those bits in their own variable and shift them right by one which when combined back with the other bits will effectively erase the stuffed bit
			bit_mask = (~bit_mask);//bitmask that allows us to isolate the stuffed bit and the bits to the right of it
			right_bits = li_data_bits & bit_mask;//isolate those bits
			li_data_bits = left_bits | right_bits;//combine the two sides together, overwriting the stuffed bit. 
			i+=4;//skip to the next relevent bits
		}
	}
	
	unsigned int unstuffed_data;
	memcpy(&unstuffed_data,&li_data_bits,4);
	return unstuffed_data;

}


int main() {
	srand(time(NULL));//seed random number generater
	unsigned char* data_bits;//will point to the chunk of data that wille eventually be stuffed
	get_random_bytes(&data_bits);//generating the bit sequence
	printf("bytes-%x,%x,%x,%x \n",data_bits[0],data_bits[1],data_bits[2],data_bits[3]);
   	
	
	unsigned int i_data_bits = *((unsigned int*)data_bits);//convert 4 bytes of unsigned char array to single unsigned int for easy use
	print_binary_uint(i_data_bits);//print the data as binary
	unsigned char* stuffed_bytes;//4 bytes plus one extra to make room for the extra bits being stuffed
	stuffed_bytes = (unsigned char*)malloc(sizeof(unsigned char) * 5);	//allocate memory for the stuffed data
	
	stuff_bits(i_data_bits, &stuffed_bytes);//stuff it
	

	unsigned int unstuffed_data = unstuff_bits(&stuffed_bytes);//unstuff it 

	if(unstuffed_data == i_data_bits)//if stuffing was successfull the return value of unstuff_bits() should be the same as what was passed into stuff_bits()
	{

		printf("\n\n unstuffed successfully!: ");
		print_binary_uint(unstuffed_data);
	}
	else
	{
		printf("Unstuffing not sucessfull");
	}


   return 0;
}