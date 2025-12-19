int pwr(int base, int n)
{
    int p;
    for(p = 1; n > 0; --n) {
        p = p * base;
    }
    return p;
}

int abs(int n)
{
    return ((n < 0) ? -n : n);
}

int sgn(int n)
{
    return (n > 0) - (n < 0);
}
