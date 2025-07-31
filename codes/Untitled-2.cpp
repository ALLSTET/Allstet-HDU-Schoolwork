#include <iostream>
#include <string>
using namespace std;

int main()
{
  int times;    
  string str;
  cin>>times;
  cin.ignore;
  for (int i=1;i<=times;i++)
  {
    getline (cin,str);
  }
  cout<<str[7]<<endl;

  return 0;
}