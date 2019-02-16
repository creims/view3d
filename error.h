#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

void setError(const char* err);
extern "C" char* getError();
void clearError();
extern "C" bool isError();

#endif
