#ifndef HASHMAP_UTIL_H
#define HASHMAP_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENOMEM -12

typedef char *K;
typedef char *V;
typedef long(*HashFunction)(K, long);

typedef struct Element {
	K key;
	V value;
	struct Element *next;
} Element;

typedef struct HashTable {
	Element **bucket;
	long size;
	HashFunction hashFunction;
} HashTable;

int initHashTable(HashTable **hashTable, long size, HashFunction hf);

int findKeyInHashTable(HashTable *hashTable, K key);

V getFromHashTable(HashTable *hashTable, K key);

int createElem(Element **new_element, K key, V value);

int putInHashTable(HashTable *hashTable, K key, V value);

void deleteKeyFromHashTable(HashTable *hashTable, K key);

void printHashTable(HashTable *hashTable);

void freeHashTable(HashTable *hashTable);

long hashFunc(K key, long size);

#endif
