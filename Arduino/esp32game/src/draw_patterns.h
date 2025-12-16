#pragma once
#include <Arduino.h>

struct LineSeg { float x0; float y0; float x1; float y1; };

typedef struct {
  const LineSeg* segs;
  uint8_t count;
} GlyphPattern;

const GlyphPattern* getGlyphPattern(char c);
