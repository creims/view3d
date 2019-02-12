#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

void setError(char* err);
char* getError();
void clearError();
bool isError();

#endif
