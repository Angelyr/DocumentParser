#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "map.h"
#include "regex.h"

#define MAX_LENGTH (256)

//pair used to store unique words and their occurrences
typedef struct pair{
    char word[MAX_LENGTH];
    int count;
}pair;

//stores document data
typedef struct documents{
    int numDocs;

    char*** docs;
    int* numWords;

    map_int_t uniqueWords;
    int numUnique;

    map_int_t top50Words;

    map_int_t bigrams;
    int numBigrams;

    map_int_t trigrams;
    int numTrigrams;

    int totalWords;
    int totalBigrams;
    int totalTrigrams;
}documents;

//returns positive if b is greater than a
int cmpPair(const void* a, const void* b){
    const pair* x = a;
    const pair* y = b;
    int result = y->count - x->count;
    if (result < 0) return -1;
    if (result > 0) return 1;
    return strcmp(x->word, y->word);
}
 
//prints msg then prints sorted occurrences until size or numPrint is reached
void printTopWords(char* msg, map_int_t occurrences, int size, int numPrint){
    const char* word;
    map_iter_t iter = map_iter(&words);
    int i=0;
    printf("%s",msg);
    pair* sort = calloc(size, sizeof(pair));
    //inserts all top50 words into sort
    while((word = map_next(&occurrences, &iter))){
        pair occurrence;
        strcpy(occurrence.word, word);
        occurrence.count = *map_get(&occurrences,word);
        sort[i] = occurrence;
        i++;
    }
    //sorts occurrences and prints it
    qsort(sort, size, sizeof(pair), cmpPair);
    qsort(sort, size, sizeof(pair), cmpPair);
    for(i=0; i<numPrint && i<size; i++){
        printf("%d %s\n",sort[i].count, sort[i].word);
    }
}

//prints all the statistics of doc
void printDocument(documents* doc){
    printf("Total number of documents: %d\n", doc->numDocs);
    printf("Total number of words: %d\n", doc->totalWords);
    printf("Total number of unique words: %d\n", doc->numUnique);
    printf("Total number of interesting bigrams: %d\n", doc->totalBigrams);
    printf("Total number of unique interesting bigrams: %d\n", doc->numBigrams);
    printf("Total number of interesting trigrams: %d\n", doc->totalTrigrams);
    printf("Total number of unique interesting trigrams: %d\n", doc->numTrigrams);
    printf("\n");
    printTopWords("Top 50 words:\n", doc->uniqueWords, doc->numUnique, 50); printf("\n");
    printTopWords("Top 20 interesting bigrams:\n", doc->bigrams, doc->numBigrams, 20); printf("\n");
    printTopWords("Top 10 interesting trigrams:\n", doc->trigrams, doc->numTrigrams, 10);
}

//removes nonAlphnum characters from word except quotes in the middle of the word
void removeNonAlpha(char* word){
    int found = 0;
    //removes nonalnum characters from beginning of word
    while(*word && !isalpha(*word)){
        *word = ' ';
        word++;
    }
    //removes nonalnum characters in the middle ignore quotes
    while(*word){
        if(!isalpha(*word) && (*word != '\'' || found || (*(word-1) && *(word-1) == ' '))){
            *word = ' ';
        }
        if(*word == '\'') found=1;
        if(*word == '\'' && *(word-1) && *(word-1) == ' ')
            *word = ' ';
        word++;
    }
    //removes nonalnum characters from the end of word
    word--;
    while(*word && !isalpha(*word)){
        *word = ' ';
        word--;
    }
}

//converts word to lower case
void lower(char* word){
    while(*word){
        *word = tolower(*word);
        word++;
    }
}

//trims extra white space from words
void trim(char* word){

    //trims leading white space
    while(isspace(*word)) word++;
    if(*word == 0) return;

    // Trim trailing space
    char* end = word + strlen(word) - 1;
    while(end > word && isspace(*end)) end--;
    end[1] = '\0';

    return;
}

//Adds word to doc
void addWord(documents* doc, char* word){
    //adds the word
    doc->docs[doc->numDocs][doc->numWords[doc->numDocs]] = (char*)calloc(256, sizeof(char));
    strcpy(doc->docs[doc->numDocs][doc->numWords[doc->numDocs]], word);
    doc->numWords[doc->numDocs]++;
    if(strlen(word) > 1) doc->totalWords++;
}

//Reads file word by word calls addWord for each word
void readWords(documents* doc, FILE* file){
    doc->docs[doc->numDocs] = calloc(500000, sizeof(char*));
    doc->numWords[doc->numDocs] = 0;
    char word[MAX_LENGTH];
	while(fscanf(file, "%s", word) == 1){
        //processes word to remove punctuation and uppercase
        lower(word);
        int yes = 0;
        if(strstr(word,"to") && strcmp(word,"to") != 0) yes++;
        char oldWord[512];
        strcpy(oldWord, word);
        removeNonAlpha(word);
        char newWord[512];
        strcpy(newWord, word);
        char* token = strtok(word, " ");
        while(token){
            removeNonAlpha(token);
            trim(token);
            if(yes && strcmp(token,"to") == 0) printf("%s ? %s\n", oldWord, newWord);
            addWord(doc, token);
            token = strtok(NULL, " ");
        }
	}
}

//Initializes arrays for a documents struct
void initializeDocuments(documents* doc){
    doc->numDocs = 0;
    doc->docs = calloc(5, sizeof(char**));
    doc->numWords = calloc(5, sizeof(int));
    
    map_init(&doc->uniqueWords);
    doc->numUnique = 0;

    map_init(&doc->top50Words);

    map_init(&doc->bigrams);
    doc->numBigrams = 0;

    map_init(&doc->trigrams);
    doc->numTrigrams = 0;

    doc->totalWords = 0;
    doc->totalBigrams = 0;
    doc->totalTrigrams = 0;
}

//populates uniqueWords with all the words
void setUniqueWords(documents* doc){
    for(int i=0; i<doc->numWords[doc->numDocs]; i++){
        int* count = map_get(&doc->uniqueWords, doc->docs[doc->numDocs][i]);
        if(count){//if word in map it increments count
            map_set(&doc->uniqueWords, doc->docs[doc->numDocs][i], (*count)+1);
        }
        else if(strlen(doc->docs[doc->numDocs][i]) > 1){//if word is not in map it adds it
            map_set(&doc->uniqueWords, doc->docs[doc->numDocs][i], 1);
            //printf("%s ",doc->words[i]);
            doc->numUnique++;
        }
    }
}

//finds word occurrences then sorts it and places it in top50words
void setTop50Words(documents* doc){
    const char* word;
    map_iter_t iter = map_iter(&doc->uniqueWords);
    pair* wordOccurrences = calloc(doc->numUnique, sizeof(pair));
    int i=0;
    //read each unique word into word Occurrences
    while((word = map_next(&doc->uniqueWords, &iter))){
        pair curr;
        strcpy(curr.word, word);
        curr.count = *map_get(&doc->uniqueWords, word);
        wordOccurrences[i] = curr;
        i++;
    }
    //sort word occurrences and use to create top50Words
    qsort(wordOccurrences, doc->numUnique, sizeof(pair), cmpPair);
    for(int i=0; i<doc->numUnique && i<50; i++){
        map_set(&doc->top50Words, wordOccurrences[i].word, wordOccurrences[i].count);
    }
}

//returns next interesting word in document used to create bigram and trigrams
char* getInteresting(documents* doc, int location){
    char** words = doc->docs[doc->numDocs];
    int* stopword = map_get(&doc->top50Words, words[location]);
    if(!stopword && strlen(words[location]) > 1){
        return words[location];
    }
    else return NULL;
}

//creates bigrams for document
void setBigrams(documents* doc){
    char ** words = doc->docs[doc->numDocs];
    for(int i=0; i<doc->numWords[doc->numDocs]-1; i++){//for each word
        //if bigram is interesting
        char* word1 = getInteresting(doc, i);
        while(strlen(words[i+1]) < 2) i++;
        char* word2 = getInteresting(doc, i+1);

        if(word1 && word2){
            //create bigram
            doc->totalBigrams++;
            char bigram[MAX_LENGTH*2] = "";
            strcat(bigram, word1);
            strcat(bigram, " ");
            strcat(bigram, word2);
            //add bigram to map
            int* count = map_get(&doc->bigrams, bigram);
            if(count){//if word in map it increments count
                map_set(&doc->bigrams, bigram, (*count)+1);
            }
            else{//if word is not in map it adds it
                map_set(&doc->bigrams, bigram, 1);
                doc->numBigrams++;
            }
        }
    }
}

//creates trigrams and stores them
void setTrigrams(documents* doc){
    char** words = doc->docs[doc->numDocs];
    for(int i=0; i<doc->numWords[doc->numDocs]-2; i++){//for each word
        //if bigram is interesting
        int skip = 0;
        char* word1 = getInteresting(doc, i);
        while(strlen(words[i+1+skip]) < 2) skip++;
        char* word2 = getInteresting(doc, i+1+skip);
        while(strlen(words[i+2+skip]) < 2) skip++;
        char* word3 = getInteresting(doc, i+2+skip);
        if(word1 && word2 && word3){
            //create bigram
            doc->totalTrigrams++;
            char trigram[MAX_LENGTH*3] = "";
            strcat(trigram, word1);
            strcat(trigram, " ");
            strcat(trigram, word2);
            strcat(trigram, " ");
            strcat(trigram, word3);
            //add bigram to map
            int* count = map_get(&doc->trigrams, trigram);
            if(count){//if word in map it increments count
                map_set(&doc->trigrams, trigram, (*count)+1);
            }
            else{//if word is not in map it adds it
                map_set(&doc->trigrams, trigram, 1);
                doc->numTrigrams++;
            }
        }
    }
}

//Receives arguments from user and calls other methods
#ifndef USE_SUBMITTY_MAIN
int main(int argc, char* argv[]){
    documents doc;
    initializeDocuments(&doc);
    for(int i=1; i<argc; i++){
        FILE* inputfile = fopen(argv[i], "r");
        doc.numDocs++;
        readWords(&doc, inputfile);
        setUniqueWords(&doc);
    }
    setTop50Words(&doc);
    int numDocs = doc.numDocs+1;
    for(int i=1; i < numDocs; i++){
        doc.numDocs = i;
        setBigrams(&doc);
        setTrigrams(&doc);
    }
    printDocument(&doc);
    return 1;
}
#endif

//Handles inputs and returns of functions
int main2(int argc, char* argv[]){
	char* regex = getRegex(argv[1]);
    char** matches;
	int lines = regex_match(argv[2], regex, &matches);
    for (int i = 0 ; i < lines ; i++ ){
        printf("%d: %s\n",i, matches[i]);
        free( matches[i] );
    } 
	free(regex);
    free(matches);
    return 1;
}