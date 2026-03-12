# things.h
Memory safe way to store Things in C (and C++)

Inspired by [this podcast
episode](https://www.youtube.com/watch?v=ShSGHb65f3M)

Watch it if you wanna learn how it works on the inside!

Things.h allows you to store Things in your C and C++ application with 100%
guaranteed NULL safety.

# Usage:

Just copy paste the `things.h` file into your include folder.

in one .c file (eg. main.c) do
```c
#define THINGS_IMPLEMENTATION
#include "things.h"
```
### Configuration:
inside of things.h: Modify the `Thing` struct, `MAX_THINGS` macro and `Kind` enum.

### Example:
```c

#include <stddef.h>
#include <stdio.h>
#include <raymath.h>
#include <raylib.h>
#define THINGS_IMPLEMENTATION
#include "nob.h"
#include "things.h"

/* Our Kind enum and Thing struct look like this:
#include <raylib.h>

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

*/
// dynamic array in C :O kwezyy
typedef struct {
    ThingRef* items;
    size_t count;
    size_t capacity;
} da_ThingRef;

int main(void)
{

    ThingRef Player = NewThing(.kind = Kind_Player, .Health = 100,
        .Position = (Vector2) { 20, 20 });

    // collect created enemies
    da_ThingRef enemies = { 0 };
    // spawn 10 enenmies on top of player
    for (int i = 0; i < 10; i++) {
        ThingRef enemy = NewThing(.kind = Kind_Enemy,
            .Health = 500,
            .Position = Vector2Add((Vector2) { 0, 50 }, Get(Player)->Position));
        // omg mom dynamic array in C :O
        da_append(&enemies, enemy);
    }

    // imagine this runs every frame in your game
    ForEachThing(thing, ref, {
        // Iterate over all things.
        switch (thing->kind) {
        case Kind_Nil:
            // Should never get triggered, unless you forgot to set .kind on a Thing.
            break;
        case Kind_Player:
            // check input or move the player around idk.
            break;
        case Kind_Enemy:
            // auto in C? Yessir. C23 added it!
            auto player = Get(Player);
            if (CheckCollisionCircles(player->Position, 50, thing->Position, 50)) {
                // enemy touching player.
                player->Health -= 10;
            };
            thing->Position = Vector2Subtract(thing->Position, (Vector2) { 0, 5 });
            printf("Enemy Position: (%f,%f)\n", thing->Position.x, thing->Position.y);
            break;
            return 0;
        }
    });

    // Loop over all the enemy refs.
    da_foreach(ThingRef, enemy, &enemies)
    {
        Delete(*enemy);
    };

    // free the dynamic array
    da_free(enemies);
    return 0;
}
```


## Public API




NewThing creates a NewThing. And returns the ThingRef for it.


It is a convenience wrapper over `CreateThing`.

```c
#define NewThing(...) CreateThing(T(Thing) { __VA_ARGS__ })
```
It allows you to write

```c
NewThing (
 .something = 1,
);
```
instead of
```c
CreateThing((Thing){
 .something =1,
});
```

NOTE: You should probably use `NewThing()`
`CreateThing` stores a new Thing into the storage.
```c
ThingRef CreateThing(Thing thing);
```
It returns a nilRef if out of capacity.

  
Get is guaranteed to never return NULL.
It returns a zero value Thing when ref is invalid.
```c
Thing* Get(ThingRef ref);
```

IsNotNil checks if a ref is active (not nil).
```c
bool IsNotNil(ThingRef ref);
```

Delete marks Thing available for reuse. Doesn't do anything if ref isn't in
use.
```c
void Delete(ThingRef ref);
```




