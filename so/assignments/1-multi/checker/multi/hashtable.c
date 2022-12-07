#include "hashtable.h"

/*Each function return 0 if it succeeds but find return 1 if success.*/

int initHashTable(HashTable **hashTable, long size, HashFunction hf)
{
	int i;

	*hashTable = (HashTable *) malloc(sizeof(HashTable));
	if (*hashTable == NULL)
		return -ENOMEM;

	(*hashTable)->bucket = (Element **) malloc(size * sizeof(Element *));
	if ((*hashTable)->bucket == NULL)
		return -ENOMEM;

	for (i = 0; i < size; i++)
		(*hashTable)->bucket[i] = NULL;

	(*hashTable)->size = size;
	(*hashTable)->hashFunction = hf;
	return 0;
}

int findKeyInHashTable(HashTable *hashTable, K key)
{
	int index;
	Element *aux;

	index = hashTable->hashFunction(key, hashTable->size);

	if (hashTable->bucket[index] != NULL) {
		aux = hashTable->bucket[index];
		while (aux != NULL) {
			if (strcmp(aux->key, key) == 0)
				return 1;
			aux = aux->next;
		}
	}
	return 0;
}

V getFromHashTable(HashTable *hashTable, K key)
{
	int index;
	Element *aux;

	index = hashTable->hashFunction(key, hashTable->size);
	aux = hashTable->bucket[index];

	while (aux != NULL) {
		if (strcmp(aux->key, key) == 0)
			return aux->value;
		aux = aux->next;
	}
	return NULL;
}

int createElem(Element **new_element, K key, V value)
{
	*new_element = (Element *) malloc(sizeof(Element));
	if ((*new_element) == NULL)
		return -ENOMEM;

	(*new_element)->key = (char *) calloc(strlen(key) + 1, sizeof(char));
	if ((*new_element)->key == NULL)
		return -ENOMEM;

	strcpy((*new_element)->key, key);

	if (value != NULL) {
		(*new_element)->value = (char *) calloc(strlen(value) + 1, sizeof(char));
		if ((*new_element)->value == NULL)
			return -ENOMEM;

		strcpy((*new_element)->value, value);
	} else {
		(*new_element)->value = (char *) calloc(1, sizeof(char));
		if ((*new_element)->value == NULL)
			return -ENOMEM;
		strcpy((*new_element)->value, "");
	}
	(*new_element)->next = NULL;

	return 0;
}

int putInHashTable(HashTable *hashTable, K key, V value)
{
	int index;
	int rc;
	Element *aux;

	index = hashTable->hashFunction(key, hashTable->size);
	if (findKeyInHashTable(hashTable, key)) {
		aux = hashTable->bucket[index];
		while (aux != NULL) {
			if (strcmp(aux->key, key) == 0) {
				aux->value = value;
				strcpy(aux->value, value);
				return 0;
			}
			aux = aux->next;
		}
	} else {
		Element *new_element;

		rc = createElem(&new_element, key, value);
		if (rc != 0)
			return rc;

		new_element->next = hashTable->bucket[index];
		hashTable->bucket[index] = new_element;
	}
	return 0;
}

void deleteKeyFromHashTable(HashTable *hashTable, K key)
{
	int index;
	Element *aux;

	index = hashTable->hashFunction(key, hashTable->size);
	aux = hashTable->bucket[index];

	if (aux == NULL)
		return;

	if (strcmp(aux->key, key) == 0) {
		free(aux->key);
		free(aux->value);
		free(aux);
		hashTable->bucket[index] = NULL;
		return;
	}

	while (aux->next != NULL) {
		if (strcmp(aux->next->key, key) == 0) {
			aux->next = aux->next->next;
			free(aux->next->key);
			free(aux->next->value);
			free(aux->next);
			return;
		}
		aux = aux->next;
	}
}

void printHashTable(HashTable *hashTable)
{
	int i;
	Element *aux;

	for (i = 0; i < hashTable->size; i++) {
		if (hashTable->bucket[i] == NULL)
			continue;
		else {
			printf("%d:\n", i);
			aux = hashTable->bucket[i];
			while (aux != NULL) {
				printf("\t'%s' : '%s'\n", aux->key, aux->value);
				aux = aux->next;
			}
		}
	}
}

void freeHashTable(HashTable *hashTable)
{
	int i;
	Element *aux;

	for (i = 0; i < hashTable->size; i++) {
		aux = hashTable->bucket[i];
		while (aux != NULL) {
			aux = hashTable->bucket[i]->next;
			free(hashTable->bucket[i]->key);
			free(hashTable->bucket[i]->value);
			free(hashTable->bucket[i]);
			hashTable->bucket[i] = aux;
		}
	}

	free(hashTable->bucket);
	free(hashTable);
}

long hashFunc(K key, long size)
{
	int i;
	long long h = 0;

	for (i = 0; i < strlen(key) - 1; i++)
		h = h * 17 + key[i];

	return h % size;
}
