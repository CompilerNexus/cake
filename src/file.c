#pragma safety enable

struct Z {
    int i;
};

struct Y {
    struct Z* _Opt pZ;
};

struct X {
    struct Y* _Opt pY;
};

void f(struct X* _Opt left, struct X* _Opt right)
{
    if (left && right)
    {
        
    }
}
