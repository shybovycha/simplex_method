#include <stdio.h>

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <QDebug>

#include <QList>

#include "fraction.cpp"
#include "simplexsolver.cpp"

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

void printSimplexTable(SimplexSolver solver)
{
    QPoint s = solver.getMatrixSize();

    printf("\\\t");

    for (int i = 0; i < s.y() - 1; i++)
    {
        printf("X%d\t", i + 1);
    }

    printf("В.ч.\n");

    QMap<int, int> basis = solver.getBasisIndices();

    for (int i = 0; i < s.x(); i++)
    {
        if (i < s.x() - 1)
        {
            printf("X%d\t", basis[i]);
        } else
        {
            printf("f(x)\t");
        }

        for (int t = 0; t < s.y(); t++)
        {
            if (i != s.x() - 1 || t != s.y() - 1)
            {
                printf("%s\t", solver.getMatrixAt(i, t).toString().toStdString().c_str());
            } else
            {
                printf("%s*", solver.getMatrixAt(i, t).toString().toStdString().c_str());
            }
        }

        printf("\n");
    }
}

#define TEST_CASE_1

int main()
{
    QList<FractionMap> limitationCoefficients;
    QMap<int, Fraction> equationCoefficients;

#ifdef TEST_CASE_1
    equationCoefficients[0] = 4;
    equationCoefficients[1] = 6;
    equationCoefficients[2] = 0;
    equationCoefficients[3] = 0;
    equationCoefficients[4] = 0;
    equationCoefficients[5] = C_SIMPLEX_M;

    {
        QMap<int, Fraction> a;

        a[1] = Fraction(-2);
        a[2] = Fraction(5);
        a[3] = Fraction(1);
        a[5] = Fraction(-1);
        a[-1] = Fraction(3);

        limitationCoefficients.push_back(a);
    }

    {
        QMap<int, Fraction> a;

        a[2] = Fraction(4);
        a[4] = Fraction(1);
        a[5] = Fraction(-1);
        a[-1] = Fraction(4);

        limitationCoefficients.push_back(a);
    }

    {
        QMap<int, Fraction> a;

        a[1] = Fraction(1);
        a[2] = Fraction(6);
        a[5] = Fraction(-1);
        a[6] = Fraction(1);
        a[-1] = Fraction(12);

        limitationCoefficients.push_back(a);
    }
#else
    equationCoefficients[1] = -1;
    equationCoefficients[2] = -4;
    equationCoefficients[3] = 0;
    equationCoefficients[4] = 1;
    equationCoefficients[5] = 0;

    {
        QMap<int, Fraction> a;

        a[1] = Fraction(1);
        a[2] = Fraction(-1);
        a[3] = Fraction(1);
        a[-1] = Fraction(3);

        limitationCoefficients.push_back(a);
    }

    {
        QMap<int, Fraction> a;

        a[1] = Fraction(2);
        a[2] = Fraction(1);
        a[4] = Fraction(1);
        a[-1] = Fraction(2);

        limitationCoefficients.push_back(a);
    }

    {
        QMap<int, Fraction> a;

        a[1] = Fraction(-1);
        a[2] = Fraction(1);
        a[5] = Fraction(1);
        a[-1] = Fraction(1);

        limitationCoefficients.push_back(a);
    }
#endif

    SimplexSolver solver(limitationCoefficients, equationCoefficients);

    solver.fillLastMatrixRow();

    printf("Перша симплекс-таблиця:\n");

    printSimplexTable(solver);

    solver.recalculateSimplexTable();

    printf("Друга симплекс-таблиця:\n");

    printSimplexTable(solver);

    solver.recalculateSimplexTable();

    printf("Третя симплекс-таблиця:\n");

    printSimplexTable(solver);

    solver.recalculateSimplexTable();

    return 0;
}
