#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

class Employee{
protected:
  string name;
  int id;
  int salary;
public:
  virtual ~Employee(){}
  virtual void salaryShow(){}
};

class Manager : public Employee{
public:
  Manager(const string &a,const int &b){
    name = a;
    id = b;
    salary = 28000;
  }
  void salaryShow() override final{
    cout<<"Manager "<<name<<"(id="<<id<<")'s Salary:"<<salary<<endl;
  }
};

class Engineer : public Employee{
public:
  Engineer(const string &a,const int &b,const int& hour){
    name = a;
    id = b;
    salary = hour*200;
  }
  void salaryShow() override final{
    cout<<"Engineer "<<name<<"(id="<<id<<")'s Salary:"<<salary<<endl;
  }
}; 

class Salesman : public Employee{
public:
  Salesman(const string& a,const int& b,const float& sale){
    name = a;
    id = b;
    salary = int(sale*0.04)+3000;
  }
  void salaryShow() override final{
    cout<<"Salesman "<<name<<"(id="<<id<<")'s Salary:"<<salary<<endl;
  }
};

class SalesManager : public Employee{
public:
  SalesManager(const string& a,const int& b,const float& sale){
    name = a;
    id = b;
    salary = int(sale*0.05)+7000;
  }
  void salaryShow() override final{
    cout<<"SalesManager "<<name<<"(id="<<id<<")'s Salary:"<<salary<<endl;
  }
};
int main(){
  string a;
  int b;
  float c;
  vector<Employee*> staff;
  string Class[4]={"Manager","Engineer","Salesman","SalesManager"};
  string specialValue[4]={"\0"," and hour"," and sale"," and sale"};
  for(int i=0;i<4;i++){
    cout<<"Create "<<Class[i]<<" Objects,pls ENTER name,id"<<specialValue[i]<<":";
    switch(i){
      case 0:
        cin>>a>>b;
        staff.push_back(new Manager(a,b));
        break;
      case 1:
        cin>>a>>b>>c;
        staff.push_back(new Engineer(a,b,int(c)));
        break;
      case 2:
        cin>>a>>b>>c;
        staff.push_back(new Salesman(a,b,c));
        break;
      case 3:
        cin>>a>>b>>c;
        staff.push_back(new SalesManager(a,b,c));
        break;
    }
  }
  system("cls");
  cout<<"ABOUT SALARY:"<<endl;
  for (auto e : staff){
    e->salaryShow();
  }
  for (auto e : staff){
    delete e;
  }
  getchar();
  getchar();
  return 0;
}