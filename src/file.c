char* _Owner strdup(const char* s);
void* _Owner malloc(unsigned size);

void free(void* _Owner ptr);

struct X {
    char* _Owner name;
};

void x_destroy(struct X* _Obj_owner p) {
    free(p->name);
}

void x_print(struct X* p)
{
    //printf("%s", p->name);
}

int main() {
    struct X x = { 0 };
    x.name = strdup("a");
    x_destroy(&x);
    static_debug(x);
    x_print(&x);
    #pragma cake diagnostic check "-Wanalyzer-maybe-uninitialized"
}
#pragma cake diagnostic check "-Wmissing-destructor"

