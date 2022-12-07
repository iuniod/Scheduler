#ifndef LIN_UTILS_H_
#define LIN_UTILS_H_

int parseFile(HashTable *hashTable, int count_dir, char **directories,
		char *infile, FILE *fpin, FILE *fpout);

int processInclude(char *line, HashTable *hashTable, int count_dir,
		char **directories, char *infile, FILE *fpin, FILE *fpout);

#endif
