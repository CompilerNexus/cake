
void* _Owner malloc(unsigned size);
void free(void* _Owner ptr);


void f(int i)
{
  void * _Owner p1 = malloc(1);
  switch(i)
  {
      case 1:
      {
          void * _Owner p2 = malloc(1);
          free(p2);
      }
      break;

      case 2:
      {
          void * _Owner p3 = malloc(1);
            free(p3);
      }
      break;
  }

  free(p1);
  
}
