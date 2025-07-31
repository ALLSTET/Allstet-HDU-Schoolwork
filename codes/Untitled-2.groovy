#include <stdio.h>
#include <stdlib.h>

// 定义最大公约数函数
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 定义有理数结构体
typedef struct {
    long long numerator; // 分子
    long long denominator; // 分母
} Rational;

// 读取有理数
Rational readRational() {
    Rational r;
    scanf("%lld/%lld", &r.numerator, &r.denominator);
    return r;
}

// 化简有理数
Rational simplify(Rational r) {
    long long g = gcd(r.numerator, r.denominator);
    r.numerator /= g;
    r.denominator /= g;
    if (r.denominator < 0) {
        r.numerator = -r.numerator;
        r.denominator = -r.denominator;
    }
    return r;
}

// 有理数加法
Rational addRational(Rational r1, Rational r2) {
    Rational result;
    result.numerator = r1.numerator * r2.denominator + r2.numerator * r1.denominator;
    result.denominator = r1.denominator * r2.denominator;
    return simplify(result);
}

int main() {
    int N;
    scanf("%d", &N);

    Rational sum = {0, 1}; // 初始化和为0/1

    for (int i = 0; i < N; i++) {
        Rational r = readRational();
        sum = addRational(sum, r);
    }

    // 化简最终结果
    sum = simplify(sum);

    // 输出结果
    if (sum.numerator == 0) {
        printf("0\n");
    } else if (sum.denominator == 1) {
        printf("%lld\n", sum.numerator);
    } else {
        long long integerPart = sum.numerator / sum.denominator;
        long long newNumerator = sum.numerator % sum.denominator;
        if (integerPart != 0) {
            printf("%lld ", integerPart);
        }
        if (newNumerator != 0) {
            printf("%lld/%lld\n", newNumerator, sum.denominator);
        }
    }

    return 0;
}