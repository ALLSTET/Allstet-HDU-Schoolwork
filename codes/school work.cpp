#include <iostream>
#include <string.h>
using namespace std;

char* Candle( float x,float y)
{
    if(x<y)
    {
        return "BW-Solid";
    }
    else if(x>y)
    {
        return "R-Hollow";
    }
    else
    {
        return "R-Cross";
    }
}

char* Shadow(float m,float n,float p,float q)
{
    if(m<p && m<q)
    {
        if(n>p && n>q)
        {
            return "with Lower Shadow and Upper Shadow";
        }
        else
        {
            return "with Lower Shadow";
        }
    }
    else
    {
        if(n>p && n>q)
        {
            return "with Upper Shadow";
        }
        else
        {
            return NULL;
        }
    }
}

int main()
{
    unsigned float Open,High,Low,Close;
    cin>>Open>>High>>Low>>Close;
    cout<<Candle(Close,Open)<<' '<<Shadow(Low,High,Open,Close)<<endl;
    return 0;
}