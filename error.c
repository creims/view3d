#include "emscripten.h"
#include "error.h"

char* error = NULL;

void setError(char* err) {
    error = err;
}

EMSCRIPTEN_KEEPALIVE
char* getError() {
    return error;
}

void clearError() {
    error = NULL;
}

EMSCRIPTEN_KEEPALIVE
bool isError() {
    return error != NULL;
}
