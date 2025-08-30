#ifndef TOOL_H
#define TOOL_H

#include <stdbool.h>
#include <string.h>
typedef enum { INVALIDT = -1, AUD, BRI, MIC, CAP, NUM, SCR } Type;

static Type parseType(const char *type) {
    if (strcmp(type, "aud") == 0) return AUD;;
    if (strcmp(type, "cap") == 0) return CAP;
    if (strcmp(type, "num") == 0) return NUM;
    if (strcmp(type, "bri") == 0) return BRI;
    if (strcmp(type, "mic") == 0) return MIC;
    return INVALIDT;
}

float getVal(const Type mode);

void setVal(const Type mode, float val);

void step(const Type mode, bool direction, float stepVal);

int getMute(void);
int getMicMute(void);
#endif

