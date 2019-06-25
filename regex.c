#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

//Return the regular expression
//Extracts the regex from the file
char* getRegex(char* filename){
	FILE* myfile = fopen(filename, "r");
	if(myfile == NULL){
		return NULL;
	}
	char* regex = (char*)calloc(256,sizeof(char));
	fscanf( myfile, "%s", regex);
	return regex;
}

//returns a string that is a copy of regex without target
char* removeSymbol(const char* regex, char target){
	char* result = (char*)calloc(strlen(regex),sizeof(char));
	int i = 0;
	while(*regex){
		if(target && *regex == target){
			target = '\0';
			regex++;
			continue;
		}
		*result++ = *regex++;
		i++;
	}
	*result = *regex;
	return result - i;
}

//returns how large the match is
//returns 0 if first char doesn't match
//used to match sections without ?,+,*
int matchChar(const char* regex, char* input){
    int total=0;
    int size=0;
    if(*regex == '['){
        regex++;size++;
        while(*regex && *regex != ']'){
            total += matchChar(regex,input);
            if(*regex == '\\') {regex+=2; size+=2;}
            else {regex+=1;size+=1;}
        }
        if(total) return size+1;
        else return total;
    }
	if(*regex == *input) {return 1;}
	else if(*regex == '.') {return 1;}
	else if(*regex == '\\' && *(regex+1)){
		if(*(regex+1) == 'd' && isdigit(*input)) {return 2;}
		else if(*(regex+1) == 'D' && !isdigit(*input)) {return 2;}
		else if(*(regex+1) == 'w' && isalpha(*input)) {return 2;}
		else if(*(regex+1) == 'W' && !isalpha(*input)) {return 2;}
		else if(*(regex+1) == 's' && isspace(*input)) {return 2;}
		else if(*(regex+1) == '\\') {return 2;}
	}
	return 0;
}

//checks if first char is affected by ?/*/+
int hasSymbol(const char* regex, char target){
    if(*regex == '['){
        while(*regex && *regex != ']'){
            regex++;
        }
    }
	return 	(*(regex+1) && *(regex+1) == target) ||
			(*(regex+2) && *(regex+2) == target && *regex == '\\'); 
}

//returns size of char including special characters
int charSize(const char* regex){
    int size = 0;
    if(*regex == '['){
        while(*regex && *regex != ']'){
            regex++; size++;
        }
        return size+1;
    }
    if(hasSymbol(regex,'?') || hasSymbol(regex,'*') ){return 2;}
    return 1;
}

//returns true/false if regex matches input
int matches(const char* regex, char* input){
	if(*input && *regex){
		int size = 0;
        int total = 0;
		//printf("%s,%s\n",regex,input);
        //matches complex characters
		if(hasSymbol(regex,'?')){
			char* test = removeSymbol(regex, '?');
			total = matches(regex+charSize(regex),input) || matches(test,input);
			free(test); 
			return total;
		}
		else if(hasSymbol(regex,'+')){
			char* test = removeSymbol(regex, '+');
			total = matches(test,input) || (matchChar(regex,input) && matches(regex,input+1));
			free(test); 
			return total;
		}
		else if(hasSymbol(regex,'*')){
			char* test = removeSymbol(regex, '*');
			total = matches(regex+charSize(regex),input) || matches(test,input) || (matchChar(regex, input) && matches(regex,input+1));
			free(test); 
			return total;
		}
        //matches characters that aren't complex
		else if((size = matchChar(regex,input))){
			return matches(regex+size,input+1);
		}
        //ignores special symbols that have been processed
		else if(*regex == '?') return matches(regex+1,input);
		else if(*regex == '+') return matches(regex+1,input);
		else if(*regex == '*') return matches(regex+1,input);
	}
    //increments regex
	if(*(regex+1) && *(regex+1) == '?') regex+=2;
	else if(*(regex+1) && *(regex+1) == '*') regex+=2;
	else if(*regex && *regex == '\\' && *(regex+2) && *(regex+2) == '?') regex+=3;
	else if(*regex && *regex == '\\' && *(regex+2) && *(regex+2) == '*') regex+=3;
    //returns if regex matched
	if(!*regex) return 1;
	return 0;
}

//searches input for start of regex
//returns true/false if regex is found
int contains(const char* regex, char* input){
	if(*input){
		if(matches(regex, input)){
			return 1;
		}
		else{
			return contains(regex,input+1);
		}
	}
	return matches(regex, input);
}

//removes new line characters
void removeNL(char* str, int len){
	int i=0;
	while(i < len){
		char c = str[i];
		if (isprint(c)){}
	   	else if(c=='\n'||c=='\r')
	        str[i] = '\0';
	    i++;
	}
}

//Prints the lines that match the regex
int regex_match(const char * filename, const char * regex, char *** matches ){
    FILE* inputfile = fopen(filename, "r");
    char*  input = (char*)calloc(1024,sizeof(char));
    *matches = (char**)calloc(500000,sizeof(char*));
    int result = 0;
	while(fgets(input, 1024, inputfile)){
		removeNL(input,strlen(input));
		if(contains(regex,input)){
            (*matches)[result] = (char*)calloc(1024,sizeof(char));
            strcpy((*matches)[result],input);
            result++;
        }
	}
	free(input);
    return result;
}