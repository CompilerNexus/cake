#pragma nullable enable


struct Y {
  char * _Owner p0;
  int * _Owner p2;
  double i2;
};

struct X {
  char * _Owner text;
  int * _Owner p1;
  int i;
  struct Y  *pY;
};

void init(_Out struct X * p);
void destroy(_Out struct X * _Obj_owner p);

int main() {
   struct X x;
   init(&x);

   static_state(x.p1, "not-null ");
   static_state(x.i, "any");
   static_state(x.pY, "not-null");
   static_state(x.pY->p0, "not-null ");
   static_state(x.pY->p2, "not-null ");
   static_state(x.pY->i2, "any");
   destroy(&x);
}

