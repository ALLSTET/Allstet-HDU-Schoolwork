#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
struct Student
{
  int id;
  string name;
  float math;
  float english;
  float program;
  float average;
};

void Read_File(const string &Filename, vector<Student> *students);        /*负责读取文件的函数，第一个参数是文件名字，为防止意外更改添加了const锁定，第二个是结构体容器指针（有点类似数组指针）*/
void Manager(vector<Student> *students);                                  /*文件管理器函数*/
void Manager_Add(vector<Student> *students);                              /*添加学生数据*/
void Manager_Delete(vector<Student> *students);                           /*删除学生数据*/
void Manager_Check(vector<Student> *students);                            /*显示学生数据*/
void Manager_Sort(vector<Student> *students);                             /*对数据排序*/
void Write_File(const string &Filename, const vector<Student> *students); /*负责写回源文件的函数*/
void Clear_Screen();                                                      /*清屏用函数*/

int main()
{
  int Flag = 1;
  string Filename;
  do
  {
    cout << "Pls input Filename:";
    cin >> Filename;
    vector<Student> students;       /*定义结构体向量容器students*/
    students.clear();               /*清除可能残留于容器中的内容*/
    Read_File(Filename, &students); /*读取文件将内容存入students容器*/
    if (students.empty())
    {
      cerr << "The File can't be opened or the File do Not Exist!" << endl;
      continue; /*检查读取文件后的students容器是否为空，空则结束本轮循环*/
    }
    Manager(&students);
    Write_File(Filename, &students);
    cout << "Do u want to read another File?(1 to continue, other to exit)\nPls enter:"; /*询问是否继续读取文件，否则退出程序*/
    cin >> Flag;
    Clear_Screen();
  } while (Flag == 1);

  return 0;
}

void Read_File(const string &Filename, vector<Student> *students)
{
  ifstream file(Filename);
  if (!file.is_open())
  {
    cerr << "Failed to Open the File:" << Filename << " !" << endl;
    return; /*未能成功打开文件则返回函数不再执行后续部分*/
  }
  Student stu;
  while (file >> stu.id >> stu.name >> stu.math >> stu.english >> stu.program >> stu.average)
  {
    students->push_back(stu); /*将文件包含的每一个stu对象依次存入students容器*/
  }
  file.close(); /*关闭文件结束读取*/
}

void Manager(vector<Student> *students)
{
  int Flag = 1, num;
  do
  {
    cout << "[1]Add new info\n[2]Delete info\n[3]Check info\n[4]Sort info\n[5]Quit\nPls enter a num:"; /*根据输入的数字决定接下来进行的操作（具体看英文）*/
    cin >> num;
    Clear_Screen();
    switch (num)
    {
    case 1:
      Manager_Add(students);
      break;
    case 2:
      Manager_Check(students);
      cout << "\n";
      Manager_Delete(students);
      break;
    case 3:
      Manager_Check(students);
      break;
    case 4:
      Manager_Sort(students);
      break;
    case 5:
      Flag = 0;
      break;
    default:
      break;
    }
  } while (Flag == 1);
}

void Manager_Add(vector<Student> *students)
{
  Student stu;
  cout << "Pls enter the ID,Name,Math,English,Program,Average regularly:";
  cin >> stu.id >> stu.name >> stu.math >> stu.english >> stu.program >> stu.average;
  students->push_back(stu);
}

void Manager_Delete(vector<Student> *students)
{
  string Name;
  cout << "Pls enter the Name to delete info:";
  cin >> Name;
  for (auto it = students->begin(); it != students->end(); ++it)
  {
    if (Name == (it->name))
    {
      students->erase(it);
      cout << Name << "has been deleted." << endl;
      return;
    }
  } /*利用结构体向量自带的迭代器进行容器内容遍历查找特定名字的student对象并删除，删除后返回函数*/
  cout << Name << "not found." << endl; /*未查找到输入名字则输出“找不到该名字所属对象”*/
}

void Manager_Check(vector<Student> *students)
{
  for (auto it = students->begin(); it != students->end(); ++it)
  {
    cout << "Id:" << it->id << " Name:" << it->name
         << "\nMath:" << it->math << " English:" << it->english
         << "\nProgram:" << it->program << " Average:" << it->average << "\n"
         << endl;
  } /*遍历输出容器储存对象内容*/
}

void Manager_Sort(vector<Student> *students)
{
  int key;
  cout << "[1]ID\t[2]Math\n[3]English\t[4]Program\t[5]Average\nPls Choose the Key used to Sort:";
  cin >> key;
  switch (key)
  { /*以下利用lambda格式实现了关键词比较和排序*/
  case 1:
    sort(students->begin(), students->end(), [](const Student &a, const Student &b)
         { return a.id < b.id; });
    break;
  case 2:
    sort(students->begin(), students->end(), [](const Student &a, const Student &b)
         { return a.math < b.math; });
    break;
  case 3:
    sort(students->begin(), students->end(), [](const Student &a, const Student &b)
         { return a.english < b.english; });
    break;
  case 4:
    sort(students->begin(), students->end(), [](const Student &a, const Student &b)
         { return a.program < b.program; });
    break;
  case 5:
    sort(students->begin(), students->end(), [](const Student &a, const Student &b)
         { return a.average < b.average; });
    break;
  default:
    cout << "Invalid key. No sorting performed." << endl;
    return;
  }
  Manager_Check(students); /*排序完毕输出排序后容器存储的向量*/
}

void Write_File(const string &Filename, const vector<Student> *students)
{
  ofstream file(Filename);
  char Flag;
  if (!file.is_open())
  {
    cerr << "Failed to Open the File:" << Filename << "!" << endl;
    cerr << "Need to Try again?\tEnter Y or N:";
    cin >> Flag;
    if (Flag == 'Y')
    {
      file.close();
      Write_File(Filename, students);
    }
    return;
  }
  for (const auto &stu : *students)
  {
    file << stu.id << " " << stu.name << " " << stu.math << " " << stu.english << " " << stu.program << " " << stu.average << endl;
  }
  file.close();
  cout << "Data is written to " << Filename << " successfully." << endl;
}

void Clear_Screen()
{
  cout << "\033[2J\033[1;1H";
}