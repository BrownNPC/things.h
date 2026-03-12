/*MIT License

Copyright (c) 2026 Omer.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdbool.h>
#include <stdint.h>
/*
Things.h: Inspired by [this podcast
episode](https://www.youtube.com/watch?v=ShSGHb65f3M)

Things.h allows you to store Things in your C and C++ application with 100%
guaranteed NULL safety.

# Usage:

Just copy paste the `things.h` file into your include folder.

in one .c file (eg. main.c) do
```c
#define THINGS_IMPLEMENTATION
#include "things.h"
```

inside of things.h: Modify the `Thing` struct and `Kind` enum.

Example:
```c
typedef enum {
  Kind_Nil,
  Kind_Player,
  Kind_Enemy,
} Kind;

typedef struct {
  Kind kind;

  Vector2 Position;
  int Health;
} Thing;
```

inside of `things.h`: modify the `MAX_THINGS`to change the max number of
things that can be created.

```c
// Set max number of things that can exist at a time.
// (100+1) will allow you to spawn a maximum of 100 Things.
#define MAX_THINGS (100 + 1)
```


*/

// ----- CONFIGURATION -----
// Edit this struct to add fields to your Thing.

typedef enum {
    Kind_Nil,
    Kind_Player,
    Kind_Enemy,
} Kind;
typedef struct {
    Kind kind;
    // CHANGE THIS
    Vector2 Position;
    int Health;
} Thing;

// Set max number of things that can exist at a time.
// (100+1) will allow you to spawn a maximum of 100 Things.
#define MAX_THINGS (100 + 1)
// the +1 is added because 1 slot is always unused.
// ----- END CONFIGURATION -----

// ---- TYPE DECLARATIONS -----

// C++ initializes types differently :(
#if defined(__cplusplus)
#define T(type) type
#else
#define T(type) (type)
#endif

// ThingRef is used to index the storage of Thing's.
// They can only be created using CreateThing or NewThing{}
typedef struct {
    uint32_t idx, generation;
} ThingRef;

static const ThingRef nilRef = T(ThingRef) {};

typedef struct {
    uint32_t activeThings;

    // Index 0 is nil (ignored)
    Thing things[MAX_THINGS];
    bool used[MAX_THINGS];
    uint32_t generations[MAX_THINGS];

} _things_storage;
// ---- END TYPE DECLARATIONS -----

// ----- PUBLIC API ------
// NOTE: You should probably use NewThing()
// CreateThing stores a new Thing into the storage.
// It returns a NilRef if out of capacity.
ThingRef CreateThing(Thing thing);

// NewThing creates a NewThing.
// It is a convenience wrapper over CreateThing.
// It allows you to write
// NewThing (
//  .something = 1,
// );
// instead of
// CreateThing((Thing){
//  .something =1,
// });
//
#define NewThing(...) CreateThing(T(Thing) { __VA_ARGS__ })

// Get is guaranteed to never return NULL.
// It returns a zero value Thing when ref is invalid.
Thing* Get(ThingRef ref);
// IsNotNil checks if a ref is active.
bool IsNotNil(ThingRef ref);
// Delete marks Thing available for reuse. Doesn't do anything if ref isn't in
// use.
void Delete(ThingRef ref);

#ifndef THINGS_IMPLEMENTATION
extern _things_storage things;
#endif

// ForEachThing iterates over all the things.
#define ForEachThing(thing, ref, code)                              \
    do {                                                            \
        for (uint32_t __i = 1; __i <= things.activeThings; ++__i) { \
            if (!things.used[__i])                                  \
                continue;                                           \
            ThingRef ref = { __i, things.generations[__i] };        \
            Thing* thing = Get(ref);                                \
            code;                                                   \
        }                                                           \
    } while (0)

// #define ForEach()
// ----- END PUBLIC API ------

// ----- IMPLEMENTATIONS ------
#ifdef THINGS_IMPLEMENTATION
_things_storage things = (_things_storage) { 0 };

ThingRef __findEmpty()
{
    for (uint32_t i = 1; i < MAX_THINGS; i++) {
        if (!things.used[i]) {
            return (ThingRef) { i, things.generations[i] };
        }
    }
    return nilRef;
}

ThingRef CreateThing(Thing thing)
{
    ThingRef ref = __findEmpty();
    if (ref.idx > 0) {
        things.used[ref.idx] = true;
        things.things[ref.idx] = thing;
        things.activeThings++;
    }
    return ref;
}
// isNotNil checks if the ref is a NilRef, or out of bounds.
bool isInBounds(ThingRef ref)
{
    if (ref.idx > 0 && ref.idx < MAX_THINGS) {
        return true;
    }
    return false;
}
// isAlive checks if a ref is in use and the generation is not old.
bool isAlive(ThingRef ref)
{
    bool dead = things.used[ref.idx] == false || ref.generation != things.generations[ref.idx];
    return !dead;
}

bool IsNotNil(ThingRef ref) { return isInBounds(ref) && isAlive(ref); }

// Delete marks the Thing available for reuse.
void Delete(ThingRef ref)
{
    if (!IsNotNil(ref)) {
        // already inactive.
        return;
    }
    things.used[ref.idx] = false;
    things.generations[ref.idx] += 1;
    // zero it out  = things.things[0](set to nil)
    things.things[ref.idx] = things.things[0];
    things.activeThings--;
}
// passed to Get so user can safely corrupt it.
Thing zeroThing = T(Thing) {};

// Get is guaranteed to never return NULL.
// It returns a zero value (nil) instead.
Thing* Get(ThingRef ref)
{
    if (IsNotNil(ref)) {
        return &things.things[ref.idx];
    }
    zeroThing = T(Thing) {};
    return &zeroThing;
}

// ----- END OF IMPLEMENTATION ------
#endif
