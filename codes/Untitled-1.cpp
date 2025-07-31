#include <iostream>
#include <string>
#include <conio.h>
using namespace std;

class Hajiren
{
private:
  string name;
  string sex;
  int age;

public:
  Hajiren(string n, string s, int a)
  {
    name = n;
    sex = s;
    age = a;
  }

  void setName(string n)
  {
    name = n;
  }

  string getName() const
  {
    return name;
  }

  void setSex(string s)
  {
    sex = s;
  }

  string getSex() const
  {
    return sex;
  }

  void setAge(int a)
  {
    age = a;
  }

  int getAge() const
  {
    return age;
  }

  void displayInfo() const
  {
    cout << "Name:" << name << "\n"
         << "Sex:" << sex << "\n"
         << "Age:" << age << endl;
  }
};
int main(){
  Hajiren YG("Yu Shuyang", "male", 19);
  YG.displayInfo();
  _getch();
  YG.setAge(20);
  cout<<"Age:"<<YG.getAge()<<endl;
  _getch();
  return 0;
}