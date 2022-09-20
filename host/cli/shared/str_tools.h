

#ifndef _STR_TOOLS_H
#define _STR_TOOLS_H

#ifdef __cplusplus 
extern "C" {
#endif

void toLower(char *ptr);
void toUpper(char *ptr);

uint32_t stripLeadingSpaces(char *array, uint32_t origLen);
uint32_t stripForbidden(char *array, uint32_t origLen);
char *truncateString(char *ptr);
char *findNext(char *ptr);
uint32_t formatString(char *array, uint32_t maxLen);

uint32_t countChars(const char *in);

char *isMatch(char *unknown, const char *temp);


char *isHexNumber(char *in);
char *isDecNumber(char *in);
uint32_t toHexNumber(char *in);
uint32_t toDecNumber(char *in);

#ifdef __cplusplus 
}
#endif
#endif