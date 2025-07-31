#include <iostream>
#include <easyx.h>
using namespace std;
#ifndef main()
int main()
{
    initgraph(640, 480);
    setlinecolor(RGB(255, 0, 0));
    line(0, 0, 640, 480);
    system("pause");
    closegraph();
    return 0;
}
#endif