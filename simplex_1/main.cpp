#include <stdio.h>

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <vector>

using std::vector;

class Fraction
{
private:
    int a, b;

public:
    Fraction(int _a, int _b = 1)
    {
        this->a = _a;
        this->b = _b;
    }

    static int gcd(int a, int b)
    {
        return (!b) ? a : gcd(b, a % b);
    }

    Fraction simplify()
    {
        int x = 2;

        while (x > 1)
        {
            x = Fraction::gcd(this->a, this->b);
            this->a /= x;
            this->b /= x;
        }

        return *this;
    }

    Fraction(const Fraction &v)
    {
        this->a = v.a;
        this->b = v.b;
    }

    Fraction operator+(Fraction v)
    {
        return Fraction((this->a * v.b) + (this->b * v.a), this->b + v.b);
    }

    Fraction operator-(Fraction v)
    {
        return Fraction((this->a * v.b) - (this->b * v.a), this->b + v.b);
    }

    Fraction operator*(Fraction v)
    {
        return Fraction(this->a * v.a, this->b * v.b);
    }

    Fraction operator/(Fraction v)
    {
        return Fraction(this->a * v.b, this->b * v.a);
    }

    bool operator>(Fraction v)
    {
        return ((*this - v).a > 0);
    }

    bool operator<(Fraction v)
    {
        return ((*this - v).a < 0);
    }

    QString toString()
    {
        return QString("%1/%2").arg(this->a).arg(this->b);
    }
};

typedef vector<Fraction> FractionVector;

void printGreeting()
{
    printf("Ця програма кагбе рішає кагбе симплекс-метходом кагбе задачу оптимізації.\n\n");
    printf("Обмеження - система лінійних рівнянь.\n");
    printf("Функція цілі - лінійна.\n");
    printf("На всі змінні накладаєцця умова невід’ємності.\n\n");
}

FractionVector readTargetFunctionFromStdin()
{
    vector<Fraction> res;

    printf("Ітак, вводимо коефіцієнти функції цілі (розділяйте числа комами, пробілами і взагалі чим завгодно - це повинні бути просто цілі числа):");
    char* s = new char[255];
    gets(s);

    QString str(s);
    QStringList list = str.split(QRegExp("[^\\d-+]+"));

    bool ok = false;

    for (int i = 0; i < list.size(); i++)
    {
        Fraction f(list[i].toInt(&ok, 10));

        if (!ok)
        {
            printf("Отакої! Сталась помилка, тому я не зможу перетворити рядок `%s` в ціле число чи дріб...\n", list[i].toStdString().c_str());
            continue;
        }

        res.push_back(f);
    }

    return res;
}

void printCompiledTargetFunction(vector<Fraction> targetFunctionCoefficients)
{
    printf("Отаку функцію цілі ми побудували:\nf(x) = ");

    for (int i = 0; i < targetFunctionCoefficients.size(); i++)
    {
        printf("(%s * X%d)", targetFunctionCoefficients[i].toString().toStdString().c_str(), i + 1);

        if (i < targetFunctionCoefficients.size() - 1)
            printf(" + ");
    }

    printf(" -> min \n");
}

vector<Fraction> readLimitationMatrixRow()
{
    vector<Fraction> res;

    char* s = new char[255];
    gets(s);

    QString str(s);

    if (str.length() <= 0)
        return res;

    QStringList list = str.split(QRegExp("[^\\d-+]+"));

    bool ok = false;

    for (int i = 0; i < list.size(); i++)
    {
        Fraction f(list[i].toInt(&ok, 10));

        if (!ok)
        {
            printf("Отакої! Сталась помилка, тому я не зможу перетворити рядок `%s` в ціле число чи дріб...\n", list[i].toStdString().c_str());
            continue;
        }

        res.push_back(f);
    }

    return res;
}

vector<FractionVector> readLimitations()
{
    vector<FractionVector> res;

    printf("А тепер потрібно ввести систему обмежень. Просто вводьте рядки з чисел. Останній елемент рядка будемо вважати вільним членом. Рядок без жодних чисел вказує на кінець введення.");

    vector<Fraction> row;

    do
    {
        row = readLimitationMatrixRow();

        if (row.size() > 1)
            res.push_back(row);
    } while (row.size() > 1);

    return res;
}

void printCompiledLimitations(vector<FractionVector> limitations)
{
    printf("Отаку систему обмежень ми отримали:\n");

    for (int i = 0; i < limitations.size(); i++)
    {
        vector<Fraction> limitationRow = limitations[i];

        for  (int t = 0; t < limitationRow.size() - 1; t++)
        {
            printf("(%s X%d)", limitationRow[t].toString().toStdString().c_str(), i + 1);

            if (t < limitationRow.size() - 2)
                printf(" + ");
        }

        printf(" = %s\n", limitationRow[limitationRow.size() - 1].toString().toStdString().c_str());
    }
}

int main()
{
    vector<Fraction> targetFunctionCoefficients;

    printGreeting();

    targetFunctionCoefficients = readTargetFunctionFromStdin();

    printCompiledTargetFunction(targetFunctionCoefficients);

    vector<FractionVector> limitations;

    limitations = readLimitations();

    printCompiledLimitations(limitations);

    return 0;
}
