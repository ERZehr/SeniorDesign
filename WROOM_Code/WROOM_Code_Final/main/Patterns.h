/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef Patterns_H
#define Patterns_H

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#include "PatternFlowField.h"
#include "PatternPlasma.h"
#include "PatternLife.h"
#include "PatternSpiral.h"

#include "Drawable.h"

class Patterns : public Drawable {
  private:
    PatternFlowField flowField;
    PatternPlasma plasma;
    PatternLife life;
    PatternSpiral spiral;

    int currentIndex = 0;
    Drawable* currentItem;

    int getCurrentIndex() {
      return currentIndex;
    }

    const static int PATTERN_COUNT = 4;

    Drawable* shuffledItems[PATTERN_COUNT];

    Drawable* items[PATTERN_COUNT] = {
      &life,
      &flowField,
      &plasma,
      &spiral,
    };

  public:
    Patterns() {
      this->currentItem = items[0];
      this->currentItem->start();
    }

    void stop() {
      if (currentItem) {
        currentItem->stop();
      }
    }

    void start() {
      if (currentItem) {
        currentItem->start();
      }
    }

    void move(int step) {
      currentIndex += step;

      if (currentIndex >= PATTERN_COUNT) {
        currentIndex = 0;
      }
      else if (currentIndex < 0) {
        currentIndex = PATTERN_COUNT - 1;
      }
      moveTo(currentIndex);
    }

    unsigned int drawFrame() {
      return currentItem->drawFrame();
    }

    char * getCurrentPatternName()
    {
      return currentItem->name;      
    }

    void moveTo(int index) {
      if (currentItem) {
        currentItem->stop();
      }
      currentIndex = index;
      currentItem = items[currentIndex];
      if (currentItem) {
        currentItem->start();
      }
    }    

    bool setPattern(String name) {
      for (int i = 0; i < PATTERN_COUNT; i++) {
        if (name.compareTo(items[i]->name) == 0) {
          moveTo(i);
          return true;
        }
      }
      return false;
    }


    bool setPattern(int index) {
      if (index >= PATTERN_COUNT || index < 0)
        return false;
      moveTo(index);
      return true;
    }

    bool isPlaylist() { return false; }

    virtual ~Patterns() { 
      currentItem->stop();
      currentItem = nullptr;
    }
};

#endif
