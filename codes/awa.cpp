#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
using namespace std;

class Student {
private:
    string ID;
    string name;
    int age;
    float grades;
public:
    Student(const string a, const string b, const int c, const float d) 
        : ID(a), name(b), age(c), grades(d) {}
        
    void disPlayInfo() {
        cout << "ID:" << ID << '\n'
             << "Name:" << name << '\n'
             << "Age:" << age << '\n'
             << "Grades:" << grades << endl;
    }
    
    void upDate() {
        cout << "Which attribute need to UPDATE? Enter:";
        string temp;
        cin >> temp;
        
        cout << "Pls enter new " << temp << " :";
        if (temp == "id" || temp == "ID" || temp == "Id") {
            cin >> ID;
        }
        else if (temp == "Name" || temp == "name" || temp == "NAME") {
            cin >> name;
        }
        else if (temp == "Age" || temp == "age" || temp == "AGE") {
            cin >> age;
        }
        else if (temp == "Grades" || temp == "grades" || temp == "GRADES") {
            cin >> grades;                
        }
        else {
            cout << "Undefined Attribute!" << endl;
        }
    }
    
    float getGrades() const {
        return grades;
    }
};

int main() {
    Student a("023", "Adam", 16, 97.2f);
    Student b("012", "Henry", 17, 99.5f);
    Student c("085", "George", 15, 72.0f);
    Student d("024", "Bob", 16, 90.5f);
    Student e("075", "Kevin", 15, 86.1f);
    
    vector<Student> students = {a, b, c, d, e}; 
    
    cout << "Initial Data:" << endl;
    for (int i = 0; i < students.size(); i++) {
        students[i].disPlayInfo();
    }
    
    Sleep(5000);  
    system("cls");
    
    // 按成绩降序排序（分数从高到低）
    sort(students.begin(), students.end(), [](const Student &a, const Student &b) {
        return a.getGrades() > b.getGrades();
    });
    
    cout << "Now data(After sort):" << endl;        
    for (int i = 0; i < students.size(); i++) {
        students[i].disPlayInfo();
    }

    Sleep(5000);
    
    return 0;
}