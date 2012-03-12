#include <stdio.h>

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <QDebug>

#include <QList>

#include "fraction.cpp"
#include "simplexsolver.cpp"
#include "simplexconverter.cpp"

QString readEquation()
{
    printf("Ця програма розв’язує лінійну задачу оптимізації за допомогою симплекс-методу.\n");
    printf("А тепер введіть лінійну функцію і вкажіть її зведення (x1 + x2 - 5x3 -> min або -7x3 - x4 -> max):");

    char *s = new char[255];
    gets(s);

    return QString(s);
}

QList<QString> readLimitations()
{
    printf("Тепер введіть систему обмежень. Приклад одного рядка: -7x1 + x2 >= -14. Пустий рядок визначає кінець введення цього блоку.\n");

    QList<QString> result;
    QString str("temp");
    char *s = new char[255];

    while (str.length() > 0)
    {
        s = gets(s);
        str = QString(s);

        if (str.length() > 0)
        {
            result.push_back(str);
        }
    }

    return result;
}

QList<QString> readPositives()
{
    printf("Тепер треба ввести список змінних, на які накладено умову невід’ємності. Це просто набір рядків виду x3, x7... Пустий рядок - кінець цього блоку.\n");

    QList<QString> result;
    QString str("temp");
    char *s = new char[255];

    while (str.length() > 0)
    {
        s = gets(s);
        str = QString(s);

        if (str.length() > 0)
        {
            result.push_back(str);
        }
    }

    return result;
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

    printf("\n");
}

int main()
{
    /*QString equation("-x1 -4x2 + x3 - x4 + 3x5 -> min");

    QList<QString> limitations;

    limitations << QString("x1 + x2 + 4x3 - x4 + 5x5 <= 9");
    limitations << QString("x1 + x2 + x3 - x4 + 3x5 <= 3");
    limitations << QString("4x1 + x2 + x3 + x4 + x5 = 3");

    QList<QString> positives;

    positives << QString("x1");
    positives << QString("x2");
    positives << QString("x3");
    positives << QString("x4");
    positives << QString("x5");*/

    /*QString equation("-x1 + x2 - 2x4 -> max");

    QList<QString> limitations;

    limitations << QString("x1 - 2x2 + x3 = 3");
    limitations << QString("4x1 + 3x2 + x4 = 4");
    limitations << QString("x1 - 2x2 + x5 = 1");
    limitations << QString("-4x1 - 3x2 + x6 = 2");

    QList<QString> positives;

    positives << QString("x1");
    positives << QString("x2");
    positives << QString("x3");
    positives << QString("x4");
    positives << QString("x5");
    positives << QString("x6");*/

#define _CONSOLE_IO_

#ifdef _CONSOLE_IO_
    QString equation = readEquation();
    QList<QString> limitations = readLimitations();
    QList<QString> positives = readPositives();
#else
    QString equation("-x1 -4x2 + x3 - x4 + 3x5 -> min");

    QList<QString> limitations;

    limitations << QString("x1 + x2 + 4x3 - x4 + 5x5 <= 9");
    limitations << QString("x1 + x2 + x3 - x4 + 3x5 <= 3");
    limitations << QString("4x1 + x2 + x3 + x4 + x5 = 3");

    QList<QString> positives;

    positives << QString("x1");
    positives << QString("x2");
    positives << QString("x3");
    positives << QString("x4");
    positives << QString("x5");
#endif

    SimplexConverter converter(equation, limitations, positives);

    converter.debugOriginalData();
    converter.convert();

    QList<FractionMap> limitationCoefficients = converter.getLimitationCoefficients();
    QMap<int, Fraction> equationCoefficients = converter.getEquationCoefficients();;

    SimplexSolver solver(equationCoefficients, limitationCoefficients);

    solver.fillLastMatrixRow();

    for (int i = 0; i < 10; i++)
    {
        printf("Симплекс-таблиця №%d:\n", i + 1);

        printSimplexTable(solver);

        solver.recalculateSimplexTable();

        int fl = solver.isFurtherOptimizationsPossible();

        if (fl)
        {
            qDebug() << QString("Optimizations are impossible: status %1").arg(fl);

            printf("Остання симплекс-таблиця:\n");

            printSimplexTable(solver);

            break;
        }
    }

    QString coords;

    FractionMap xc = solver.getCornerCoordinates();
    Fraction fc = solver.getFunctionValue();

    for (int i = 0; i < xc.keys().size(); i++)
    {
        coords += QString("%1, ").arg(xc[xc.keys().at(i)].toString());
    }

    QString res("Result: X*(%1), f* = %2");

    printf("Result: %s\n", res.arg(coords).arg(fc.toString()).toStdString().c_str());

    return 0;
}
