/*

FILE: queryengine.c
By: Delos Chang

Description: a command-line processing engine that asks users for input
and creates a ranking from the crawler and indexer to display to the user

INPUTS: ./queryengine [TARGET INDEXER FILENAME] [RESULTS FILE NAME]
AND / OR operators for command-line processing
- a space (" ") represents an 'AND' operator
- a capital OR represents an 'OR' operator

TO EXIT: '!exit'

Outputs: The query engine will output a ranking based on the queries that the 
user enters

Design Spec: 
The query engine reloads from index.dat into memory via an inverted index structure.
When the user enters a query, it is sanitized (removing extraneous characters) and then
split into a keyword list (queryList). Each keyword is checked against the inverted index:
hashing the word, then checking the hash slot and traversing until a WordNode is found.
If a WordNode is not found, then the word was not indexed. If it is found, all the URLNodes
of that WordNode are returned as a list. 

If the next keyword in the queryList is AND, the engine will find the intersection of word1's
list with word2's list. If the next keyword is OR, the engine will find the union of word1's
list with word2's list. This approach is iterated across the entire queryList.

A quicksort algorithm is used to rank the page word frequencies and sort the URLs. This is 
printed out for the user.

Implementation Spec Pseudocode: 
1. Validate user input arguments
2. Load the indexer's index file into memory
3. Query the user via the command line
4. Change the capital letters to lower case letters
5. Cross-reference the query with the index 
6. Rank results via an algorithm based on word frequency with AND / OR operators
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../utils/header.h"
#include "../utils/index.h"
#include "querylogic.h"
#include "queryengine.h"

INVERTED_INDEX* indexReload = NULL;

// this function prints generic usage information 
void printUsage(){
  printf("Usage: ./queryengine ../indexer_dir/index.dat ../crawler_dir/data \n"); 
}

void validateArgs(int argc, char* argv[]){
  // struct for checking whether directory exists
  struct stat s;

  // check for correct number of parameters first
  if ( (argc != 3) ){
    fprintf(stderr, "Error: insufficient arguments. 3 required. You provided %d \n", argc);
    printUsage();

    exit(1);
  }

  // Validate that file exists
  if ( stat(argv[1], &s) != 0){
    fprintf(stderr, "Error: The file argument %s was not found.  Please enter a readable and valid file. \n", argv[1]);
    printUsage();

    exit(1);
  }

  // Validate that file exists
  if ( stat(argv[2], &s) != 0){
    fprintf(stderr, "Error: The dir argument %s was not found.  Please enter a readable and valid dir. \n", argv[2]);
    printUsage();

    exit(1);
  }
}

int main(int argc, char* argv[]){
  // (1) Validate the parameters
  validateArgs(argc, argv);

  // (2) Initialize the inverted index
  if ( (indexReload = initStructure(indexReload)) == NULL){
    fprintf(stderr, "Could not initialize data structures. Exiting");
    exit(1);
  }

  // (2b) Load the index into memory
  char* loadFile = argv[1];
  char* urlDir = argv[2];
  int rankingResult = 0;

  INVERTED_INDEX* reloadResult = reloadIndexFromFile(loadFile, indexReload);
  if (reloadResult == NULL){
    exit(1);
  } else {
    LOG("Finished reloading index from file");
  }

  // (3) Query the user via the command line
  while (1) {
    char query[1000];
    printf(" \n KEY WORD:> ");
    fgets(query, 999, stdin);

    // (3a) Check for exit parameter
    if (!strncmp(query, "!exit\n", strlen("!exit\n") + 1) ){
      break;
    }

    sanitize(query);

    LOG("Querying..\n");

    // (4) Cross-reference the query with the index and retrieve results
    // (4a) Convert the actual query into a list of keywords, in queryList
    char* queryList[1000];
    BZERO(queryList, 1000);

    curateWords(queryList, query);

    // (4b) Convert keywords from uppercase to lowercase (except OR)
    sanitizeKeywords(queryList);

    // (4c) Lookup the keywords, apply operators, and return results
    DocumentNode* saved[1000];
    BZERO(saved, 1000);
    lookUp(saved, queryList, indexReload);

    // (5) Rank results via an algorithm based on word frequency with AND / OR operators
    rankingResult = rankAndPrint(saved, urlDir); 
    if ( rankingResult != 1){
      fprintf(stderr, "Couldn't rank results");
      exit(1);
    }

    LOG("Done");


    // (6) Clean up the word list with keywords
    cleanUpQueryList(queryList);
    BZERO(query, 1000);
  }


  // (7) Clean up the reloaded index
  LOG("Cleaning up");
  cleanUpIndex(indexReload);

  return 0;
}
