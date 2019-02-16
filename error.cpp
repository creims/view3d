#include "emscripten.h"
#include "error.h"

char* error;

void setError(const char* err) {
    error = (char*)err;
}

EMSCRIPTEN_KEEPALIVE
extern "C" char* getError() {
    return error;
}

void clearError() {
    error = NULL;
}

EMSCRIPTEN_KEEPALIVE
extern "C" bool isError() {
    return error != NULL;
}
