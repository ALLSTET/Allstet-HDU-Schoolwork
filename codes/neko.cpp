#include <iostream>
#include <windows.h>
using namespace std;

struct Vectors{
  float x,y,z;
}V1,V2;

int main(){
  cout<<"Pls input the value of a(x,y,z):";
  cin>>V1.x>>V1.y>>V1.z;
  system("cls");
  cout<<"Pls input the value of b(x,y,z):";
  cin>>V2.x>>V2.y>>V2.z;
  system("cls");
  cout<<"a:("<<V1.x<<","<<V1.y<<","<<V1.z<<")"<<endl;
  cout<<"b:("<<V2.x<<","<<V2.y<<","<<V2.z<<")"<<endl;
  cout<<"Wait for command..."<<endl;
  char a,b,c;
  cin>>a>>b>>c;
  if(b != '+' && b != '-' && b != '*'){
    cout<<"Invalid command!"<<endl;
  }
  else if((a == 'a' || a == 'b') && (c == 'a' || c == 'b')){
    switch(b){
      case '+':
        cout<<"Result is:("<<V1.x+V2.x<<","<<V1.y+V2.y<<","<<V1.z+V2.z<<")"<<endl;
        break;
      case '-':
        if(a=='a' && c=='b'){
          cout<<"Result is:("<<V1.x-V2.x<<","<<V1.y-V2.y<<","<<V1.z-V2.z<<")"<<endl;
        }
        else{
          cout<<"Result is:("<<V2.x-V1.x<<","<<V2.y-V1.y<<","<<V2.z-V1.z<<")"<<endl;
        }
        break;
      case '*':
        cout<<"Result is:("<<V1.x*V2.x<<","<<V1.y*V2.y<<","<<V1.z*V2.z<<")"<<endl;
        break;
      default:
        cout<<"How do u find this?"<<endl;
        break;
    }
  }

  Sleep(3000);
  return 0;
}
