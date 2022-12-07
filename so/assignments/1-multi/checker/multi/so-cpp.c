#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hashtable.h"
#include "utils.h"

#define MAX_LEN 256
#define TABLE_SIZE 100

void freeMemory(HashTable *hashTable, char **directories,
				char *infile, char *outfile)
{
	freeHashTable(hashTable);

	free(directories);
}

/*Verifies each argument and saves it in the correspondent variable.*/
/*Each function returns 0 if it finishes correctly.*/
int parseArg(int argc, char **argv, HashTable *hashTable, int *count_dir,
	int size_dir, char ***directories, char **infile, char **outfile)
{
	int count = 1;
	char option;
	int index = 0;
	char *key = NULL, *value = NULL;
	int rc = 0;

	while (count < argc) {
		if (argv[count][0] == '-') {
			option = argv[count][1];
			if (strlen(argv[count])  <= 3) {
				count++;
				index = 0;
			} else {
				index = 2;
			}
			switch (option) {
			case 'D':
				key = (char *) calloc(strlen(argv[count]) + 1,
							sizeof(char));
				if (key == NULL)
					return -ENOMEM;

				value = strstr(&argv[count][index], "=");
				if (value == NULL) {
					value = "";
					strcpy(key, &argv[count][index]);
				} else {
					strncpy(key, &argv[count][index],
						value - &argv[count][index]);
					value = &value[1];
				}
				rc = putInHashTable(hashTable, key, value);
				if (rc != 0)
					return rc;
				free(key);
				break;

			case 'I':
				if (*count_dir == size_dir) {
					size_dir = size_dir * 2;
					*directories = (char **)
						realloc(*directories,
						size_dir * 2 * sizeof(char *));
					if (*directories == NULL)
						return -ENOMEM;
				}

				(*directories)[*count_dir] =
						&argv[count][index];
				*count_dir += 1;
				break;

			case 'o':
				if (*outfile == NULL)
					*outfile = argv[count];
				else
					return -1;
				break;

			default:
				return -1;
			}
		} else if (*infile == NULL)
			*infile = argv[count];
		else if (*outfile == NULL)
			*outfile = argv[count];
		else
			return -1;

		count++;
	}

	return 0;
}

/*Receives a line with a define and saves it in the hashtable.*/
int processDefines(char *line, HashTable *hashTable)
{
	char separators[10] = "\n\t \\";
	char *name = NULL;
	char *token = NULL;
	char *key = NULL, *value = NULL;
	int rc;
	char *copy = NULL;

	copy = (char *) calloc(strlen(line) + 1, sizeof(char));
	if (copy == NULL)
		return -ENOMEM;
	strcpy(copy, line);

	token = strtok(copy, separators);

	name = (char *) calloc(strlen(token) + 1, sizeof(char));
	if (name == NULL)
		return -ENOMEM;

	strcpy(name, token);

	token = strtok(NULL, separators);
	key = (char *) calloc(strlen(token) + 1, sizeof(char));
	if (key == NULL)
		return -ENOMEM;

	strcpy(key, token);

	token = strtok(NULL, separators);
	if (token != NULL) {
		value = (char *) calloc(strlen(line) + 1, sizeof(char));
		if (value == NULL)
			return -ENOMEM;
		strcpy(value, strstr(line, token));
		value[strlen(value) - 1] = '\0';
	} else {
		value = NULL;

	}

	if (strcmp(name, "#define") == 0) {
		rc = putInHashTable(hashTable, key, value);
		if (rc != 0)
			return rc;
	} else if (strcmp(name, "#undef") == 0)
		deleteKeyFromHashTable(hashTable, key);

	free(copy);
	free(name);
	free(key);
	if (value != NULL)
		free(value);
	return 0;
}

/*Receives a line that is not a conditional or a define/include and after it*/
/*recursively replaces defined values it writes it at the out filepointer*/
/*described file.*/
int processLine(char *line, HashTable *hashTable, FILE *fpout)
{
	char *token = NULL;
	char *copy = NULL;
	char separators[30] = "\n\t []{}()<>,.;:=+-*/%!|&^\\";
	int rc = 0;
	char *new_line = NULL;
	int index = 0;
	int old_index = 0;

	new_line = (char *) calloc(strlen(line) + 1, sizeof(char));
	if (new_line == NULL)
		return -ENOMEM;

	copy = (char *) calloc(strlen(line) + 1, sizeof(char));
	if (copy == NULL)
		return -ENOMEM;

	strcpy(copy, line);

	token = strtok(copy, separators);
	while (token != NULL) {
		index = token - copy;
		if (strcmp(token, "\"") == 0) {
			while (1) {
				token = strtok(NULL, separators);
				if (strchr(token, '\"'))
					break;
			}

		} else {
			if (findKeyInHashTable(hashTable, token)) {
				char *value;

				value = getFromHashTable(hashTable, token);

				strncat(new_line, &line[old_index],
					index - old_index);
				if (strlen(token) < strlen(value)) {
					new_line = (char *) realloc(new_line,
					strlen(line) + strlen(value) -
					strlen(token) + 1);
					if (new_line == NULL)
						return -ENOMEM;
				}
				strcat(new_line, value);
				old_index = token - copy + strlen(token);
			}
			token = strtok(NULL, separators);
		}
	}

	index = strlen(line) - 1;
	strncat(new_line, &line[old_index], index - old_index);
	strcat(new_line, "\n");

	if (strcmp(new_line, line) != 0) {
		rc = processLine(new_line, hashTable, fpout);
		if (rc != 0)
			return rc;
	} else
		fputs(new_line, fpout);

	free(copy);
	free(new_line);
	return 0;
}

/*Takes a conditional and checks it. In order to implement nested*/
/*conditionals it calls itself recursively. If the condition is valued as true*/
/*it parses the lines by calling the specific functions.*/
int processConditionals(char *line, HashTable *hashTable, int count_dir,
	char **directories, char *infile, FILE *fpin, FILE *fpout)
{
	char separators[10] = "\n\t \\";
	char *name = NULL;
	char *token = NULL;
	char *key = NULL;
	int cond = 0;
	char buf[MAX_LEN];
	char *copy = NULL;
	int rc = 0;

	copy = (char *) malloc((strlen(line) + 1) * sizeof(char));
	if (copy == NULL)
		return -ENOMEM;

	strcpy(copy, line);

	token = strtok(copy, separators);
	name = (char *) calloc(strlen(token) + 1, sizeof(char));
	if (name == NULL)
		return -ENOMEM;

	strcpy(name, token);

	if ((strcmp(name, "#define") == 0) ||
		(strcmp(name, "#undef") == 0)) {
		free(name);
		free(copy);
		rc = processDefines(line, hashTable);
		if (rc != 0)
			return rc;
		return 0;
	}

	if (strcmp(name, "#endif") == 0) {
		free(name);
		free(copy);
		return 0;
	}

	token = strtok(NULL, separators);
	if (strcmp(name, "#else") != 0) {
		key = (char *) calloc(strlen(token) + 2, sizeof(char));
		if (key == NULL)
			return -ENOMEM;

		strcpy(key, token);
	}

	if ((strcmp(name, "#if") == 0) ||
		(strcmp(name, "#elif") == 0) ||
		(strcmp(name, "#ifdef") == 0) ||
		(strcmp(name, "#ifndef") == 0)) {
		if ((strcmp(name, "#if") == 0) ||
			(strcmp(name, "#elif") == 0)) {
			if (findKeyInHashTable(hashTable, key))
				cond = atoi(getFromHashTable(hashTable, key));
			else
				cond = atoi(key);
		} else {
			if (strcmp(name, "#ifdef") == 0)
				cond = findKeyInHashTable(hashTable, key);
			else {
				if (findKeyInHashTable(hashTable, key))
					cond = 0;
				else
					cond = 1;
			}
		}

		if (cond != 0) {
			while (fgets(buf, MAX_LEN, fpin)) {
				if (buf[0] != '#') {
					rc = processLine(buf, hashTable,
							fpout);
					if (rc != 0)
						return rc;
				} else if (strstr(buf, "#define") ||
						strstr(buf, "#undef") ||
						strstr(buf, "#if")) {
					rc = processConditionals(buf,
						hashTable, count_dir,
						directories, infile,
						fpin, fpout);
					if (rc != 0)
						return rc;
				} else if (strstr(buf, "#include")) {
					rc = processInclude(buf, hashTable,
						count_dir, directories,
						infile, fpin, fpout);
					if (rc != 0)
						return rc;
				} else
					break;
			}
			if (strstr(buf, "#endif") == NULL)
				while (fgets(buf, MAX_LEN, fpin))
					if (strstr(buf, "#endif"))
						break;
		} else {
			while (fgets(buf, MAX_LEN, fpin)) {
				if (strstr(buf, "#elif") ||
					strstr(buf, "#else") ||
					strstr(buf, "endif")) {
					rc = processConditionals(buf,
						hashTable, count_dir,
						directories, infile,
						fpin, fpout);
					if (rc != 0)
						return rc;
					break;
				}
			}
		}
	}

	if (strcmp(name, "#else") == 0) {
		while (fgets(buf, MAX_LEN, fpin)) {
			if (strstr(buf, "#endif") == NULL) {
				rc = processLine(buf, hashTable, fpout);
				if (rc != 0)
					return rc;
			} else
				break;
		}
	}

	free(copy);
	if (key != NULL)
		free(key);
	free(name);
	return 0;
}

/*Tries to open the file from current folder if it is source file is passed*/
/*as stdin, from source folder if the source is a file or searches it in the*/
/*directories passed as arguments if they exist.*/
/*Return -1 if it doesn't succeed.*/
int processInclude(char *line, HashTable *hashTable, int count_dir,
	char **directories, char *infile, FILE *fpin, FILE *fpout)
{
	char separators[10] = "\n\t\" \\";
	char *name = NULL;
	char *token = NULL;
	char *filename = NULL;
	FILE *fpinclude;
	int i;
	char *path = NULL;
	int rc;
	char *ret = NULL;
	char *infile_dir = NULL;

	token = strtok(line, separators);
	name = (char *) calloc(strlen(token) + 1, sizeof(char));
	if (name == NULL)
		return -ENOMEM;

	strcpy(name, token);

	token = strtok(NULL, separators);
	filename = (char *) calloc(strlen(token) + 1, sizeof(char));
	if (filename == NULL)
		return -ENOMEM;

	strcpy(filename, token);

	if (fpin == stdin)
		fpinclude = fopen(filename, "r");
	else {
		ret = strrchr(infile, '/');
		infile_dir = (char *) calloc(strlen(infile) - strlen(ret) +
					strlen(filename) + 2, sizeof(char));
		if (infile_dir == NULL)
			return -ENOMEM;

		strncpy(infile_dir, infile, ret + 1 - infile);
		strcat(infile_dir, filename);

		fpinclude = fopen(infile_dir, "r");
		free(infile_dir);
	}

	if (fpinclude == NULL) {
		for (i = 0; i < count_dir; i++) {
			path = (char *) calloc((strlen(directories[i]) +
					strlen(filename) + 2), sizeof(char));
			if (path == NULL)
				return -ENOMEM;

			strcat(path, directories[i]);
			strcat(path, "/");
			strcat(path, filename);

			fpinclude = fopen(path, "r");
			free(path);
			if (fpinclude != NULL)
				break;
		}
	}
	free(name);
	free(filename);

	if (fpinclude == NULL)
		return -1;

	rc = parseFile(hashTable, count_dir, directories,
			infile, fpinclude, fpout);
	if (rc != 0)
		return rc;

	fclose(fpinclude);
	return 0;
}

/*Receives a file (included or source) and parses each line calling the right*/
/*function in order to process it.*/
int parseFile(HashTable *hashTable, int count_dir, char **directories,
		char *infile, FILE *fpin, FILE *fpout)
{
	char buf[MAX_LEN];
	char *line = NULL;
	int rc;

	while (fgets(buf, MAX_LEN, fpin)) {
		if (strcmp(buf, "\n") == 0)
			continue;
		if (line == NULL) {
			line = (char *) calloc(sizeof(buf), sizeof(char));
			if (line == NULL)
				return -ENOMEM;
			strcpy(line, buf);
		} else {
			line = (char *) realloc(line,
					sizeof(line) + sizeof(buf));
			if (line == NULL)
				return -ENOMEM;
			strcat(line, buf);
		}

		if (line[strlen(line) - 2] == '\\') {
			line[strlen(line) - 2] = '\0';
			continue;
		} else {
			if ((strncmp(line, "#define", 7) == 0) ||
				(strncmp(line, "#undef", 6) == 0)) {
				rc = processDefines(line, hashTable);
				if (rc != 0)
					return rc;
			} else if ((strncmp(line, "#if", 3) == 0) ||
					(strncmp(line, "#ifdef", 6) == 0) ||
					(strncmp(line, "#ifndef", 7) == 0)) {
				rc = processConditionals(buf, hashTable,
					count_dir, directories, infile,
					fpin, fpout);
				if (rc != 0)
					return rc;
			} else if (strncmp(line, "#include", 8) == 0) {
				rc = processInclude(line, hashTable, count_dir,
					directories, infile, fpin, fpout);
				if (rc != 0)
					return rc;
			} else {
				rc = processLine(line, hashTable, fpout);
				if (rc != 0)
					return rc;
			}

			free(line);
			line = NULL;

		}
	}
	return 0;
}

/*Opens the specified input and output files. If they don't exist it reads*/
/*or writes from/to stdin/stdout.*/
int processInputFile(HashTable *hashTable, int count_dir, char **directories,
		char *infile, char *outfile)
{
	FILE *fpin, *fpout;
	int rc;

	if (infile != NULL) {
		fpin = fopen(infile, "r");
		if (fpin == NULL)
			return -1;
	} else
		fpin = stdin;

	if (outfile != NULL)
		fpout = fopen(outfile, "w");
	else
		fpout = stdout;

	rc = parseFile(hashTable, count_dir, directories, infile, fpin, fpout);

	if (fpin != stdin)
		fclose(fpin);

	if (fpout != stdout)
		fclose(fpout);

	if (rc != 0)
		return rc;
	return 0;
}

/*Calls each function in correct order to resolve the problem.*/
/*The hashtable is implemented in hashtable.c.*/
int main(int argc, char **argv)
{
	char *infile = NULL, *outfile = NULL;
	int size_dir = 1, count_dir = 0;
	HashTable *hashTable;
	int rc;
	char **directories;

	directories = (char **) malloc(sizeof(char *) * size_dir);

	if (directories == NULL)
		return -ENOMEM;

	rc = initHashTable(&hashTable, TABLE_SIZE, &hashFunc);
	if (rc != 0)
		return rc;

	rc = parseArg(argc, argv, hashTable, &count_dir, size_dir, &directories,
			&infile, &outfile);
	if (rc != 0) {
		freeMemory(hashTable, directories, infile, outfile);
		return rc;
	}

	rc = processInputFile(hashTable, count_dir, directories,
			infile, outfile);
	if (rc != 0) {
		freeMemory(hashTable, directories, infile, outfile);
		return rc;
	}

	freeMemory(hashTable, directories, infile, outfile);
	return 0;
}
