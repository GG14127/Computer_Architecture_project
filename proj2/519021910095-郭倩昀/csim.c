//Guo Qianyun 519021910095
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define BUFFERSIZE 50
static int hit = 0;		//hit count
static int miss = 0;	//miss count
static int eviction = 0;//eviction count
struct BLOCK {			//cache block
	int valid;			//valid-invalid bit
	long tag;			//tag infomation
	int LRU;			//LRU record
};

struct CACHE {
	int waynum;		//E associativity: num of ways per set
	int setnum;		//2^s num of sets
	int blocksize;	//2^b b: num of block bits
	struct BLOCK **c; 
};
int str_int(char *str);		//change decimal str to int
long hex_dec(char *str);	//change hex str to decimal
//analyze the memory trace and get the operation type and memory address
void parse(char *buffer, char *op, long *add);
//simulate accessing an address in the cache, using LRU
void execute(struct CACHE *cache, long address, int cnt);


int main(int argc, char *argv[])
{
	struct CACHE cache;
	int s, b, S, E, B = 0;
	FILE *fp = NULL;			//for tracefile
	char buffer[BUFFERSIZE];	//for instruction in tracefile
	
	//get S, E, B
	for(int i = 0; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(argv[i][1] == 's')
			{
				i++;
				s = str_int(argv[i]);
				S = 1 << s;//2^s
			}
			if(argv[i][1] == 'E')
			{
				i++;
				E = str_int(argv[i]);
			}
			if(argv[i][1] == 'b')
			{
				i++;
				b = str_int(argv[i]);
				B = 1 << b;//2^b
			}
			if(argv[i][1] == 't')
			{
				i++;
				if((fp = fopen(argv[i], "r")) == NULL)
				{
					printf("  ERROR: FILE %s OPEN FAILED", argv[i]);
					exit(1);
				}
			}
		}
	}
	if (s <= 0 || E <= 0 || b <= 0)
	{
		printf("  ERROR: INVALID PARAMETER");
		exit(1);
	}
	
	//initialize cache
	cache.waynum = E;
	cache.setnum = S;
	cache.blocksize = B;
	cache.c = (struct BLOCK **) malloc (sizeof(struct BLOCK *) * S);//S sets
	for (int i = 0; i < S; i++)
	{
		// E ways per set
		cache.c[i] = (struct BLOCK *) malloc (sizeof(struct BLOCK) * E);
		//initialize each block
		for(int j = 0; j < E; j++)
		{
			cache.c[i][j].valid = 0;
			cache.c[i][j].tag = 0;
			cache.c[i][j].LRU = 0;
		}
	}
	
	int cnt = 0;
	while(fgets(buffer,sizeof(buffer), fp))	//read in next memory trace
	{
		cnt++;
		char op;
		long address;
		parse(buffer, &op, &address);	//get operation and address
		if(op == 'I') continue;			//skip instruction cache accesses
		execute(&cache, address, cnt);
		if(op == 'M')					//M execute twice
			execute(&cache, address, cnt);
	}
    printSummary(hit, miss, eviction);
    return 0;
}

//change decimal str to int
int str_int(char *str)
{
	int len = strlen(str);
	int res = 0;
	for(int i = 0; i < len; i++)
	{
		res = res * 10 + str[i] - '0';
	}
	return res;
}

//change hex str to decimal
long hex_dec(char *str)
{
	int len = strlen(str);
	long res = 0;
	for(int i = 0; i < len; i++)
	{
		if(str[i] >='0' && str[i] <= '9')
			res = res * 16 + str[i] - '0';
		if(str[i] >='a' && str[i] <= 'f')
			res = res * 16 + str[i] - 'a' + 10;
		if(str[i] >='A' && str[i] <= 'F')
			res = res * 16 + str[i] - 'A' + 10;	
		
	}
	return res;
}

//simulate accessing an address in the cache, using LRU
void execute(struct CACHE *cache, long address, int cnt)
{
	int set_id = (address / cache->blocksize) % (cache->setnum);	//which set
	long tag_num = (address / cache->blocksize) / (cache->setnum);	//tag info
	//search in the set
	int pos = -1;
	for (int i = 0; i < cache -> waynum; i++)
	{
		//found
		if (cache->c[set_id][i].valid == 1 && cache->c[set_id][i].tag == tag_num)
		{
			hit++;
			cache->c[set_id][i].LRU = cnt;	//update LRU record
			return;
		}
		//not found search if empty
		if(cache->c[set_id][i].valid == 0)
			pos = i;
	}
	miss++;
	if(pos >= 0 && pos < cache->waynum)//miss but still have space
	{
		cache->c[set_id][pos].valid = 1;
		cache->c[set_id][pos].tag = tag_num;
		cache->c[set_id][pos].LRU = cnt;	//update LRU record
		return;
	}
	else//evict
	{
		eviction++;
		int min = cache->c[set_id][0].LRU;
		pos = 0;
		for(int i = 1; i < cache->waynum; i++)	//find eviction (minimum LRU)
		{
			if(cache->c[set_id][i].LRU < min)
			{
				min = cache->c[set_id][i].LRU;
				pos = i;
			}
		}
		cache->c[set_id][pos].tag = tag_num;
		cache->c[set_id][pos].LRU = cnt;	//update LRU record
		cache->c[set_id][pos].valid = 1;
	}
	return;
}

//analyze the memory trace and get the operation type and memory address
void parse(char *buffer, char *op, long *address)
{
	char addr[50];
	int i = 0;
	while (buffer[i] == ' ') i++;
	*op=buffer[i];
	i++;
	while (buffer[i] == ' ') i++;
	int j = 0;
	for(; buffer[i] != ',';i++,j++)
	{
		addr[j] = buffer[i];
	}
	addr[j] = 0;
	*address = hex_dec(addr);//change string to long type
}

