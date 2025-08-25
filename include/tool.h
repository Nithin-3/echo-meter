#ifndef TOOL_H
#define TOOL_H

#include <stdbool.h>

typedef enum { INVALIDT = -1, AUD, BRI, MIC } Type;
float getVal(const char *mode);

void setVal(const char *mode, float val);

void step(const char *mode, bool direction);

#endif

