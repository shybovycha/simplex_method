#include <stdio.h>

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <QDebug>

#include <QList>

#include "fraction.cpp"

typedef QList<Fraction> FractionVector;
typedef QList<FractionVector> FractionMatrix;

struct SimplexTable
{
    QList<FractionVector> data;
    FractionVector marks;

    FractionVector freePartials;

    FractionVector cornerCoordinates;
    Fraction targetFunctionValue;

    QList<int> basis;
    int variableCount;
};

void printGreeting()
{
    printf("Ця програма кагбе рішає кагбе симплекс-метходом кагбе задачу оптимізації.\n\n");
    printf("Обмеження - система лінійних рівнянь.\n");
    printf("Функція цілі - лінійна.\n");
    printf("На всі змінні накладаєцця умова невід’ємності.\n\n");
}

FractionVector readTargetFunctionFromStdin()
{
    FractionVector res;

    printf("Ітак, вводимо коефіцієнти функції цілі (розділяйте числа комами, пробілами і взагалі чим завгодно - це повинні бути просто цілі числа):");
    char* s = new char[255];
    s = gets(s);

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

void printCompiledTargetFunction(FractionVector targetFunctionCoefficients)
{
    printf("Отаку функцію цілі ми побудували:\nf(x) = ");

    for (int i = 0; i < (int) targetFunctionCoefficients.size(); i++)
    {
        printf("(%s * X%d)", targetFunctionCoefficients[i].toString().toStdString().c_str(), i + 1);

        if (i < (int) targetFunctionCoefficients.size() - 1)
            printf(" + ");
    }

    printf(" -> min \n");
}

FractionVector readLimitationMatrixRow()
{
    FractionVector res;

    char* s = new char[255];
    s = gets(s);

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

FractionMatrix readLimitations()
{
    FractionMatrix res;

    printf("А тепер потрібно ввести систему обмежень. Просто вводьте рядки з чисел. Останній елемент рядка будемо вважати вільним членом. Рядок без жодних чисел або з лише одним числом (число буде ігноровано) вказує на кінець введення.\n");

    FractionVector row;

    do
    {
        row = readLimitationMatrixRow();

        if (row.size() > 1)
            res.push_back(row);
    } while (row.size() > 1);

    return res;
}

void printCompiledLimitations(FractionMatrix limitations)
{
    printf("Отаку систему обмежень ми отримали:\n");

    for (int i = 0; i < (int) limitations.size(); i++)
    {
        FractionVector limitationRow = limitations[i];

        for  (int t = 0; t < (int) limitationRow.size() - 1; t++)
        {
            printf("(%s X%d)", limitationRow[t].toString().toStdString().c_str(), t + 1);

            if (t < (int) limitationRow.size() - 2)
                printf(" + ");
        }

        printf(" = %s\n", limitationRow[limitationRow.size() - 1].toString().toStdString().c_str());
    }
}

QList<int> findBasis(FractionMatrix limitations)
{
    QList<int> res;

    for (int i = 0; i < (int) limitations.size(); i++)
    {
        for (int t = 0; t < (int) limitations.at(i).size() - 1; t++)
        {
            Fraction k = limitations.at(i).at(t).simplify();

            if (k.a == 1 && k.b == 1)
            {
                bool fl = false;

                for (int j = 0; j < (int) limitations.size(); j++)
                {
                    if (i == j)
                        continue;

                    if (limitations.at(j).at(t) == limitations.at(i).at(t))
                    {
                        fl = true;
                        break;
                    }
                }

                if (!fl)
                {
                    res.push_back(t);
                }
            }
        }
    }

    return res;
}

SimplexTable buildFirstSimplexTable(FractionVector targetFunctionCoefficients, FractionMatrix limitations)
{
    SimplexTable res;

    res.basis = findBasis(limitations);

    res.variableCount = limitations.at(0).size();

    // копіюємо внутрішню частину таблиці з системи обмежень
    for (int i = 0; i < (int) limitations.size(); i++)
    {
        FractionVector row;

        // одночасно ж шукаємо кількість змінних в задачі
        if (limitations.at(i).size() > res.variableCount)
            res.variableCount = limitations.at(i).size();

        for (int t = 0; t < (int) limitations.at(i).size() - 1; t++)
        {
            row.push_back(limitations.at(i).at(t));
        }

        res.data.push_back(row);
    }

    // знуляємо всі координати кутової точки
    for (int i = 0; i < res.variableCount; i++)
    {
        res.cornerCoordinates.push_back(Fraction(0));
    }


    // шукаємо початкові оцінки
    for (int i = 0; i < res.variableCount; i++)
    {
        if (res.basis.contains(i))
            continue;

        if (res.marks.size() < i)
            res.marks.push_back(0); else
                res.marks[i] = 0;

        for (int t = 0; t < (int) res.basis.size(); t++)
        {
            int basisIndex = res.basis.at(t);

            // here is 'out of range' error
            res.marks[i] += res.data.at(basisIndex).at(i) * targetFunctionCoefficients.at(basisIndex);

            // і одразу ж знаходимо координати кутової точки
            res.freePartials[basisIndex] = limitations.at(t).at(limitations.at(t).size() - 1);
        }

        res.marks[i] -= targetFunctionCoefficients.at(i);
    }

    return res;
}

int main()
{
    FractionVector targetFunctionCoefficients;

    printGreeting();

    targetFunctionCoefficients = readTargetFunctionFromStdin();

    printCompiledTargetFunction(targetFunctionCoefficients);

    FractionMatrix limitations;

    limitations = readLimitations();

    printCompiledLimitations(limitations);

    SimplexTable table = buildFirstSimplexTable(targetFunctionCoefficients, limitations);

    for (int i = 0; i < (int) table.marks.size(); i++)
    {
        printf("%s\n", table.marks.at(i).toString().toStdString().c_str());
    }

    return 0;
}
