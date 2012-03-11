#ifndef __SIMPLEXSOLVER_CPP__
#define __SIMPLEXSOLVER_CPP__

#include <QList>
#include <QMap>
#include <QPoint>

#include "simplex.h"

class SimplexSolver
{
private:
    Fraction **M;

    int rowCount, columnCount;

    QMap<int, int> basisIndices;
    QMap<int, Fraction> equationCoefficients;

public:
    SimplexSolver(QMap<int, Fraction> _equationCoefficients, QList<FractionMap> _limitationCoefficients)
    {
        if (_limitationCoefficients.size() < 1)
        {
            throw E_NOT_ENOUGH_LIMITATIONS;
        }

        int limitationsCount = (int) _limitationCoefficients.size();

        this->rowCount = limitationsCount;
        this->columnCount = 0;

        for (int i = 0; i < limitationsCount; i++)
        {
            if (_limitationCoefficients.at(i).size() < 2)
            {
                throw E_LIMITATION_TOO_SMALL;
            }

            int currentLimitationSize = (int) _limitationCoefficients.at(i).keys().size();

            for (int t = 0; t < currentLimitationSize; t++)
            {
                int currentVariableIndex = _limitationCoefficients.at(i).keys().at(t);

                // визначаємо максимальний індекс змінної аби встановити, скільки змінних нам взагалі доступно
                if (currentVariableIndex > this->columnCount)
                {
                    this->columnCount = currentVariableIndex;
                }

                // не перевірятимемо, чи вільний член - базисна змінна =)
                if (currentVariableIndex > 0)
                {
                    Fraction currentVariable = _limitationCoefficients.at(i)[currentVariableIndex];

                    // і тут же визначаємо, чи базисною є поточна змінна, чи ні
                    bool fl = true;

                    for (int j = 0; j < limitationsCount; j++)
                    {
                        // якщо коефіцієнт їмінної не "+1" - вона НЕ базисна
                        if (currentVariable.a != currentVariable.b)
                        {
                            qDebug() << QString("X%1 (%2) does not have +1 coefficient").arg(currentVariableIndex).arg(currentVariable.toString().toStdString().c_str());

                            fl = false;
                            break;
                        }

                        // не перевірятимо поточне рівняння двічі
                        if (i == j)
                            continue;

                        // якщо змінна міститься хоча б в одному рівнянні, крім даного - вона також НЕ базисна
                        if (_limitationCoefficients.at(j).keys().contains(currentVariableIndex))
                        {
                            qDebug() << QString("eq[%1] && eq[%2] contains X%3").arg(i + 1).arg(j + 1).arg(currentVariableIndex);

                            fl = false;
                            break;
                        }
                    }

                    if (fl)
                    {
                       qDebug() << QString("X%1 is meant to be a basis-one, right?").arg(currentVariableIndex);

                        this->basisIndices[i] = currentVariableIndex;
                    }
                }
            }
        }

        // вільний член
        this->columnCount++;

        // рядок оцінок та значення функції у кутовій точці
        this->rowCount++;

        this->M = new Fraction*[this->rowCount];

        for (int i = 0; i < this->rowCount; i++)
        {
            this->M[i] = new Fraction[this->columnCount];

            // копіюватимемо лише те, що можемо
            if(i > _limitationCoefficients.size() - 1)
                continue;

            for (int t = 1; t < (int) _limitationCoefficients.at(i).keys().size(); t++)
            {
                int variableIndex = _limitationCoefficients.at(i).keys().at(t);
                Fraction coefficient = _limitationCoefficients.at(i)[variableIndex];

                // індексація масиву все ще в стилі C++
                this->M[i][variableIndex - 1] = coefficient;
            }

            this->M[i][this->columnCount - 1] = _limitationCoefficients.at(i)[-1];
        }

        // копіюємо коефіцієнти функції цілі
        for (int i = 0; i < (int) _equationCoefficients.keys().size(); i++)
        {
            this->equationCoefficients[_equationCoefficients.keys().at(i)] = _equationCoefficients[_equationCoefficients.keys().at(i)];
        }
    }

    QMap<int, Fraction> calculateMarks()
    {
        QMap<int, Fraction> result;

        for (int i = 0; i < (int) this->equationCoefficients.keys().size(); i++)
        {
            qDebug() << QString("c[%1] = %2").arg(this->equationCoefficients.keys().at(i)).arg(this->equationCoefficients[this->equationCoefficients.keys().at(i)].toString());
        }

        for (int i = 0; i < this->columnCount - 1; i++)
        {
            Fraction mark(0);

            // для всіх змінних, які не базисні рахуємо оцінку
            if (!this->basisIndices.values().contains(i + 1))
            {
                qDebug() << QString("X%1 is NOT within a basis").arg(i + 1);

                for (int t = 0; t < this->rowCount - 1; t++)
                {
                    int basisIndex = this->basisIndices[t];
                    Fraction k = this->M[t][i] * this->equationCoefficients[basisIndex - 1];

                    qDebug() << QString("M[%1][%2] * c[%3] == %4 * %5 == %6").arg(t).arg(i).arg(basisIndex).arg(this->M[t][i].toString()).arg(this->equationCoefficients[basisIndex - 1].toString()).arg(k.toString());

                    mark += k;
                }

                qDebug() << QString("c[%1] == %2").arg(i + 1).arg(this->equationCoefficients[i].toString());

                mark -= this->equationCoefficients[i];
            } else
            // для решти вона рівна нулю
            {
                qDebug() << QString("X%1 IS within a basis").arg(i + 1);
            }

            result[i] = mark;
        }

        return result;
    }

    Fraction getFunctionValue()
    {
        Fraction result(0);

        for (int i = 0; i < this->rowCount - 1; i++)
        {
            result += this->M[i][this->columnCount - 1] * this->equationCoefficients[this->basisIndices[i] - 1];
        }

        return result;
    }

    void fillLastMatrixRow()
    {
        Fraction f = this->getFunctionValue();
        QMap<int, Fraction> row = this->calculateMarks();

        for (int i = 0; i < (int) row.keys().size(); i++)
        {
            this->M[this->rowCount - 1][row.keys().at(i)] = row[row.keys().at(i)];
        }

        this->M[this->rowCount - 1][this->columnCount - 1] = f;
    }

    int findSolvingColumn()
    {
        int max = 0, fl = 0;

        for (int i = 1; i < this->columnCount - 1; i++)
        {
            if (this->M[this->rowCount - 1][i] > Fraction(0) && this->M[this->rowCount - 1][i] > this->M[this->rowCount - 1][max])
            {
                max = i;
                fl = 1;
            }
        }

        qDebug() << QString("maximum column's mark is %1 while no others found.").arg(this->M[this->rowCount - 1][max].toString());

        if (fl || this->M[this->rowCount - 1][max] > Fraction(0))
        {
            return max;
        } else
        {
            return -1;
        }
    }

    int findSolvingRow(int column)
    {
        if (column < 0)
            return -1;

        QMap<int, Fraction> deltas;

        for (int i = 0; i < this->rowCount - 1; i++)
        {
            if (this->M[i][this->columnCount - 1] > Fraction(0) &&
                this->M[i][column] > Fraction(0))
            {
                deltas[i] = this->M[i][this->columnCount - 1] / this->M[i][column];
            }
        }

        Fraction min = deltas[deltas.keys().at(0)];
        int index = deltas.keys().at(0);

        for (int i = 1; i < (int) deltas.keys().size(); i++)
        {
            if (deltas[deltas.keys().at(i)] < min)
            {
                index = deltas.keys().at(i);
            }
        }

        return index;
    }

    void recalculateSimplexTable()
    {
        int a = this->findSolvingColumn(), b = this->findSolvingRow(a);

        if (a < 0 || b < 0)
        {
            printf("Could not find valid solvind element (column == %d, row == %d). Terminating.\n", a, b);
            return;
        }

        Fraction k1 = this->M[b][a];

        this->basisIndices[b] = a + 1;

        for (int i = 0; i < this->columnCount; i++)
        {
            this->M[b][i] /= k1;
            this->M[this->rowCount - 1][i] = Fraction(0);
        }

        for (int i = 0; i < this->rowCount - 1; i++)
        {
            if (i != b)
            {
                Fraction k2 = this->M[i][a];

                for (int t = 0; t < this->columnCount; t++)
                {
                    this->M[i][t] -= this->M[b][t] * k2;
                }
            }
        }

        this->fillLastMatrixRow();
    }

    int isFurtherOptimizationsPossible()
    {
        // чи можливі подальші оптимізації?
        // 0 - можливі
        // 1 - не можливі бо поточне значення функції оптимальне
        // 2 - не можливі, бо функція цілі не обмежена на МПР знизу

        bool fl1 = true;

        for (int i = 0; i < this->columnCount - 1; i++)
        {
            // якщо хоча б над однією з додатніх оцінок нема жодного додатнього ел-та
            // ф-я цілі не обмежена знизу на МПР
            if (this->M[this->rowCount - 1][i] > Fraction(0))
            {
                fl1 = false;

                bool fl2 = true;

                for (int t = 0; t < this->rowCount - 1; t++)
                {
                    if (this->M[t][i] > Fraction(0))
                    {
                        fl2 = false;

                        break;
                    }
                }

                if (fl2)
                    return 2;
            }
        }

        if (fl1)
            return 1;

        return 0;
    }

    QPoint getMatrixSize()
    {
        return QPoint(this->rowCount, this->columnCount);
    }

    Fraction getMatrixAt(int x, int y)
    {
        if (x > -1 && x < this->rowCount && y > -1 && y < this->columnCount)
            return this->M[x][y]; else
                return Fraction(0, 0);
    }

    QMap<int, int> getBasisIndices()
    {
        return this->basisIndices;
    }
};

#endif
