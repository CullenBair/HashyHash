// Cullen Bair
// cu326578

#include <stdio.h>
#include <stdlib.h>
#include "HashyHash.h"
#include <math.h>
// Prototype for the Prime helper function
int nextPrime(int n);

// Function that makes a new hash table with a given capacity
unsigned int hash(int key)
{
	return ((key / 10) % 10) * 37 + (key % 10);
}

HashTable *makeHashTable(int capacity)
{
    // declaring variables and allocating a hash struct
    int i;
    HashTable *hashy = malloc(sizeof(HashTable));
    if(hashy == NULL)
    {
        free(hashy);
        return NULL;
    }
    // assigning all the default values to the items in the hash struct
    hashy->size = 0;
    hashy->hashFunction = NULL;
    hashy->probing = LINEAR;
    hashy->stats.collisions = 0;
    hashy->stats.opCount = 0;
    // making sure that capacity is bigger than zero
    if(capacity <= 0)
        hashy->capacity = DEFAULT_CAPACITY;
    else
        hashy->capacity = capacity;
    // allocating the array and freeing everything if it fails
    hashy->array = malloc(sizeof(int) * hashy->capacity);
    if(hashy->array == NULL)
    {
        free(hashy->array);
        free(hashy);
        return NULL;
    }
    // initializing all the array cells to UNUSED
    for(i=0; i < hashy->capacity; i++) hashy->array[i] = UNUSED;

    return hashy;
}
// simple function to destroy the hash table
HashTable *destroyHashTable(HashTable *h)
{
    if(h == NULL)
        return NULL;
    // freeing everything
    free(h->array);
    free(h);
    return NULL;
}
// setting probing mechanism...
int setProbingMechanism(HashTable *h, ProbingType probing)
{
    // precautions
    if(h == NULL)
        return HASH_ERR;
    if(probing != LINEAR && probing != QUADRATIC)
        return HASH_ERR;
    // if its linear, we set it to linear, if not we set it to quadratic
    if(probing == LINEAR)
    {
        h->probing = LINEAR;
        return HASH_OK;
    }

    h->probing = QUADRATIC;
    return HASH_OK;
}
// takes the hash function and assigns it to the hash struct
int setHashFunction(HashTable *h, unsigned int (*hashFunction)(int))
{
    if(h == NULL)
        return HASH_ERR;

    h->hashFunction = hashFunction;
    return HASH_OK;
}
// used to see if we need to expand a hash table
int isAtLeastHalfEmpty(HashTable *h)
{
    // returning false if h is null, capacity is not positive, or if the table is at least half full
    if(h == NULL)
        return 0;
    if(h->capacity <= 0)
        return 0;
    if((h->capacity / 2) < h->size)
        return 0;

    return 1;
}
// Szumlanski's prime function that finds the next prime number greater than or equal to n
int nextPrime(int n)
{
	int i, root, keepGoing = 1;

	if (n % 2 == 0)
		++n;

	while (keepGoing)
	{
		keepGoing = 0;
		root = sqrt(n);

		for (i = 3; i <= root; i++)
		{
			if (n % i == 0)
			{
				n += 2;
				keepGoing = 1;

				break;
			}
		}
	}
	return n;
}
// if the table is filling up, we must expand it depending on the probing
int expandHashTable(HashTable *h)
{
    // initializations and some constants to keep for the end
    int i, n = 0;
    int o = h->stats.opCount;
    int s = h->size;
    // if the capacity isnt big enough its impossible to expand
    if(h->capacity <= 0)
        return HASH_ERR;
    // temp array because we must free the space and make a new array
    int *temp = malloc(sizeof(int)*h->capacity);
    for(i=0;i<h->capacity;i++)
    {
        // only taking the important values
        if(h->array[i] != UNUSED && h->array[i] != DIRTY)
            temp[n++] = h->array[i];
    }
    // making room for the new array without leaking memory
    free(h->array);
    // probing matters
    if(h->probing == LINEAR)
    {
        // linear way of creating the new array
        h->array = malloc(sizeof(int) * ((h->capacity * 2) + 1));
        if(h->array == NULL)
            return HASH_ERR;
        // new capacity for new array
        h->capacity = ((h->capacity * 2) + 1);
        // initializing the new array with unused cells
        for(i=0; i < h->capacity; i++)
            h->array[i] = UNUSED;
        // inserting the old values into the new array
        for(i=0; i < n; i++)
            insert(h, temp[i]);
    }
    else    // quadratic
    {
        // quadratic way of making new array
        h->array = malloc(sizeof(int) * nextPrime((h->capacity * 2) + 1));
        if(h->array == NULL)
            return HASH_ERR;
        // capacity for quadratic
        h->capacity = nextPrime((h->capacity * 2) + 1);
        // same as linear
        for(i=0; i < h->capacity; i++)
            h->array[i] = UNUSED;
        for(i=0; i < n; i++)
            insert(h, temp[i]);
    }
    // things we need to keep the same
    h->stats.opCount = o;
    h->size = s;

    free(temp);

    return HASH_OK;
}
// to insert values into a hash table
int insert(HashTable *h, int key)
{
    // initializations
    int index, oindex, going = 1, n = 0, ok, count = 0;
    // precautions
    if(h == NULL || h->hashFunction == NULL)
        return HASH_ERR;
    // if its in need of an update
    if(!isAtLeastHalfEmpty(h))
    {
        ok = expandHashTable(h);
        if(!ok)
            return HASH_ERR;
    }
    // finding the index from the hash function
    index = (h->hashFunction(key) % h->capacity);
    oindex = index;
    // only difference is from the collisions
    if(h->probing == LINEAR)
    {
        // we found a spot already!
        if(h->array[index] == UNUSED)
        {
            // put the key in the empty spot, increment size and operation count and return
            h->array[index] = key;
            h->size++;
            h->stats.opCount++;
            return HASH_OK;
        }
        // loop to go until we find a spot
        while(going)
        {
            // increment collisions and find new index
            h->stats.collisions++;
            index = (index + 1) % h->capacity;
            // if we find a spot, we put the key there and prepare to break out of the loop
            if(h->array[index] == UNUSED)
            {
                h->array[index] = key;
                going = 0;
            }
            // this is to prevent an infinite loop
            count++;
            if(count >= h->capacity)
                return HASH_ERR;
        }
    }
    else    // this is the same except the new index is different in quadratic
    {
        if(h->array[index] == UNUSED)
        {
            h->array[index] = key;
            h->size++;
            h->stats.opCount++;
            return HASH_OK;
        }
        while(going)
        {
            h->stats.collisions++;
            n++;
            index = (oindex + (n*n)) % h->capacity;

            if(h->array[index] == UNUSED)
            {
                h->array[index] = key;
                going = 0;
            }
            count++;
            if(count >= h->capacity)
                return HASH_ERR;
        }
    }
    // we want to increment size and operation count when we add our new number to the hash table
    h->size++;
    h->stats.opCount++;

    return HASH_OK;
}
// used to find a certain value and return the index in which is was found
int search(HashTable *h, int key)
{
    // initializations
    int index, oindex, going = 1, n = 0, count = 0;
    if(h == NULL || h->hashFunction == NULL)
        return -1;
    // finding the index the same way it would be in the insert function
    index = (h->hashFunction(key) % h->capacity);
    oindex = index;
    // this is the same process as insertion but we are finding the key instead of an open cell
    if(h->probing == LINEAR)
    {
        if(h->array[index] == key)
        {
            h->stats.opCount++;
            return index;
        }
        while(going)
        {
            h->stats.collisions++;
            index = (index + 1) % h->capacity;

            if(h->array[index] == UNUSED)
            {
                index = -1;
                break;
            }

            if(h->array[index] == key)
                going = 0;

            count++;
            if(count >= h->capacity)
            {
                index = -1;
                break;
            }
        }
    }
    else
    {
        if(h->array[index] == key)
        {
            h->stats.opCount++;
            return index;
        }
        while(going)
        {
            h->stats.collisions++;
            n++;
            index = (oindex + (n*n)) % h->capacity;

            if(h->array[index] == UNUSED)
            {
                index = -1;
                break;
            }

            if(h->array[index] == key)
                going = 0;

            count++;
            if(count >= h->capacity)
            {
                index = -1;
                break;
            }
        }
    }
    // yep searching is an operation
    h->stats.opCount++;
    // returning the index where we found the value or -1 if its not there
    return index;
}
// if we want to remove a value from a hash function
int delete(HashTable *h, int key)
{
    int index;
    if(h == NULL || h->hashFunction == NULL)
        return -1;
    // we use the search function to do the work for us
    index = search(h, key);
    // as long as we found the value, we overwrite the value with DIRTY to show that a number used to be there
    if(index != -1)
    {
        h->array[index] = DIRTY;
        h->size--;
    }

    return index;
}
