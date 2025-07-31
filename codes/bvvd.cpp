#include <iostream>
#include <cmath>
#include <windows.h>
using namespace std;

class Fraction {
private:
    int num;    // 分子
    int den;    // 分母

    // 计算最大公约数
    int gcd(int a, int b) const {
        return b == 0 ? a : gcd(b, a % b);
    }

    // 约分并标准化分数
    void reduce() {
        int common = gcd(abs(num), abs(den));
        num /= common;
        den /= common;
        
        // 确保分母始终为正
        if (den < 0) {
            num = -num;
            den = -den;
        }
    }

public:
    // 两参数构造函数
    Fraction(int numerator = 0, int denominator = 1) : num(numerator), den(denominator) {
        if (den == 0) den = 1;  // 简单处理分母为0的情况
        reduce();
    }

    // 加法
    Fraction add(const Fraction& other) const {
        return Fraction(
            num * other.den + den * other.num,
            den * other.den
        );
    }

    // 减法
    Fraction sub(const Fraction& other) const {
        return Fraction(
            num * other.den - den * other.num,
            den * other.den
        );
    }

    // 乘法
    Fraction mul(const Fraction& other) const {
        return Fraction(
            num * other.num,
            den * other.den
        );
    }

    // 除法
    Fraction div(const Fraction& other) const {
        return Fraction(
            num * other.den,
            den * other.num
        );
    }

    // 输入分数
    void input() {
        cout << "input num:";
        cin >> num;
        cout << "input den:";
        cin >> den;
        while (den == 0) {  // 处理分母为0的情况
            cout << "den mustn't be 0! Input a new number:";
            cin >> den;
        }
        reduce();
    }

    // 显示分数
    void display() const {
        if (den == 1)
            cout << num;
        else
            cout << num << "/" << den;
        cout << endl;
    }
};

int main() {
    // 测试用例
    Fraction a(3, 4);
    Fraction b(2, 5);

    cout << "Grades A:";
    a.display();
    cout << "Grades B:";
    b.display();

    cout << "\n+ result:";
    a.add(b).display();

    cout << "- result:";
    a.sub(b).display();

    cout << "* result:";
    a.mul(b).display();

    cout << "/ result:";
    a.div(b).display();

    Sleep(5000);

    return 0;
}