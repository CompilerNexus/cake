#pragma safety enable


#pragma safety enable

#include <stdlib.h>
#include <string.h>

struct X {
  char *_Owner _Opt name;
};

struct X * _Owner make();


int main() {
   struct X * _Owner p = make();
   
   static_debug(p);
}


