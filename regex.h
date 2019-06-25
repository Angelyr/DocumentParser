#ifndef REGEX_H
#define REGEX_H

int charSize(const char* regex);
int contains(const char* regex, char* input);
char* getRegex(char* filename);
int hasSymbol(const char* regex, char target);
int main(int argc, char* argv[]);
int matchChar(const char* regex, char* input);
int matches(const char* regex, char* input);
int regex_match(const char * filename, const char * regex, char *** matches );
void removeNL(char* str, int len);
char* removeSymbol(const char* regex, char target);

#endif