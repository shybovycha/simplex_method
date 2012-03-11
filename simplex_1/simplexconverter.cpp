#ifndef __SIMPLEXCONVERTER_CPP__
#define __SIMPLEXCONVERTER_CPP__

#include <QDebug>
#include <QList>
#include <QMap>
#include <QString>
#include <QRegExp>

#include "simplex.h"

class SimplexConverter
{
private:
    FractionMap orig_coefficients;
    QList<FractionMap> orig_matrix;
    QList<QString> orig_signs;
    QList<int> orig_positive_variables;
    QString orig_mode;

    FractionMap coefficients;
    QList<FractionMap> matrix;
    QList<QString> signs;
    QList<int> positive_variables;
    QString mode;

public:
    SimplexConverter(QString equation, QList<QString> limitations = QList<QString>(), QList<QString> positive_variable_list = QList<QString>())
    {
        QRegExp equation_re("([^>]{1,})\\s*->\\s*(min|max)", Qt::CaseInsensitive);
        QRegExp equation_partial_re("((\\+|\\-)?\\s*(\\d+)?)\\s*x(\\d+)\\s*", Qt::CaseInsensitive);

        /*
         * RegExp capture groups:
         *
         *    cap3      cap4                                cap6
         *   |---|     |----|                             |-------|
         * (((+|-)?\\s*(\\d+))?\\s*x(\\d+)\\s*)+\\s*->\\s*(min|max)
         *  |----------------|      |----|
         *         cap2              cap5
         * |----------------------------------|
         *                cap1
         *
         * But that's too complicated, so i simplify that as follows:
         *   1) split the entire string into cap1 and cap2
         *   2) match cap1 with a simple regexp
         *
         * The "equation_partial_re" "simple regexp" i call.
         *
         *   cap2      cap3         cap4
         *  |---|     |----|       |----|
         * ((+|-)?\\s*(\\d+))?\\s*x(\\d+)\\s*
         * |----------------|
         *        cap1
         *
         * Actually, we do not need groups #3 and #4. They represent
         * a number. So, we can simply treat them as one group.
         *
         * However, cap1 could contain spaces, as you can see.
         * So, before converting it to a number, it should be altered
         * to remove those (possible) spaces.
         *
         */

        if (equation_re.indexIn(equation) > -1)
        {
            QString equation_left_part = equation_re.cap(1);

            int pos = 0;

            while ((pos = equation_partial_re.indexIn(equation_left_part, pos)) != -1)
            {
                QString s1 = equation_partial_re.cap(1).replace(QRegExp("\\s+"), QString(""));
                QString s2 = equation_partial_re.cap(4);
                QString s3 = equation_partial_re.cap(3).replace(QRegExp("\\s+"), QString(""));

                s1 = s1.replace(QRegExp("\\s+"), QString(""));

                if (s3.length() <= 0)
                    s1 += QString("1");

                int index = s2.toInt();

                this->orig_coefficients[index] = Fraction(s1.toInt());

                pos += equation_partial_re.matchedLength();
            }

            this->orig_mode = equation_re.cap(2);
        }

        QRegExp limitation_re("([^<>=]{1,})\\s*(.?=)\\s*(-?\\s*\\d+)", Qt::CaseInsensitive);

        /*
         * RegExp capture groups:
         *
         * ([^<>=]{1,})\\s*(.?=)\\s*(\\d+)
         *
         * Well, here's nothing to comment...
         *
         */

        for (int i = 0; i < limitations.size(); i++)
        {
            QString limitation = limitations.at(i);
            FractionMap limitation_row;

            if (limitation_re.indexIn(limitation) > -1)
            {
                QString limitation_left_part = limitation_re.cap(1);

                int pos = 0;

                while ((pos = equation_partial_re.indexIn(limitation_left_part, pos)) != -1)
                {
                    QString s1 = equation_partial_re.cap(1).replace(QRegExp("\\s+"), QString(""));
                    QString s2 = equation_partial_re.cap(4);
                    QString s3 = equation_partial_re.cap(3).replace(QRegExp("\\s+"), QString(""));

                    s1 = s1.replace(QRegExp("\\s+"), QString(""));

                    if (s3.length() <= 0)
                        s1 += QString("1");

                    int index = s2.toInt();

                    limitation_row[index] = Fraction(s1.toInt());

                    pos += equation_partial_re.matchedLength();
                }
            }

            limitation_row[-1] = Fraction(limitation_re.cap(3).replace(QRegExp("\\s+"), QString("")).toInt());

            this->orig_matrix.push_back(limitation_row);
            this->orig_signs.push_back(limitation_re.cap(2));
        }

        for (int i = 0; i < positive_variable_list.size(); i++)
        {
            QString v = positive_variable_list[i].replace(QRegExp("\\s+"), QString(""));
            QRegExp positive_re("x(\\d+)", Qt::CaseInsensitive);

            if (positive_re.indexIn(v) > -1)
            {
                this->orig_positive_variables.push_back(positive_re.cap(1).toInt());
            }
        }
    }

    void debugOriginalData()
    {
        printf("<ORIGINAL>\n");

        QString function_line("f(x) = ");

        for (int i = 0; i < this->orig_coefficients.keys().size(); i++)
        {
            function_line += QString(" + %2 * X%1").arg(this->orig_coefficients.keys().at(i)).arg(this->orig_coefficients[this->orig_coefficients.keys().at(i)].toString());
        }

        function_line += QString(" -> %1").arg(this->orig_mode);

        printf("%s\n", function_line.toStdString().c_str());

        printf("{\n");

        for (int i = 0; i < this->orig_matrix.size(); i++)
        {
            FractionMap limitation = this->orig_matrix.at(i);

            QString line;

            for (int t = 1; t < limitation.keys().size(); t++)
            {
                line += QString(" + %2 * X%1").arg(limitation.keys().at(t)).arg(limitation[limitation.keys().at(t)].toString());
            }

            line += QString(" %1 %2 ").arg(this->orig_signs.at(i)).arg(this->orig_matrix.at(i)[-1].toString());

            printf("%s\n", line.toStdString().c_str());
        }

        printf("}\n");

        QString positive_line;

        for (int i = 0; i < this->orig_positive_variables.size(); i++)
        {
            positive_line += QString(" X%1 >= 0 ").arg(this->orig_positive_variables.at(i));
        }

        printf("%s\n\n", positive_line.toStdString().c_str());
    }

    void debugConvertedData()
    {
        printf("<CALCULATED>\n");

        QString function_line("f(x) = ");

        for (int i = 0; i < this->coefficients.keys().size(); i++)
        {
            function_line += QString(" + %2 * X%1").arg(this->coefficients.keys().at(i)).arg(this->coefficients[this->coefficients.keys().at(i)].toString());
        }

        function_line += QString(" -> %1").arg(this->mode);

        printf("%s\n", function_line.toStdString().c_str());

        printf("{\n");

        for (int i = 0; i < this->matrix.size(); i++)
        {
            FractionMap limitation = this->matrix.at(i);

            QString line;

            for (int t = 1; t < limitation.keys().size(); t++)
            {
                line += QString(" + %2 * X%1").arg(limitation.keys().at(t)).arg(limitation[limitation.keys().at(t)].toString());
            }

            line += QString(" %1 %2 ").arg(this->signs.at(i)).arg(this->matrix.at(i)[-1].toString());

            printf("%s\n", line.toStdString().c_str());
        }

        printf("}\n");

        if (this->positive_variables.size() <= 0)
        {
            printf("<no positive conditions were copied/calculated yet>\n");
        } else
        {
            QString positive_line;

            for (int i = 0; i < this->positive_variables.size(); i++)
            {
                positive_line += QString(" X%1 >= 0 ").arg(this->positive_variables.at(i));
            }

            printf("%s\n", positive_line.toStdString().c_str());
        }

        printf("\n");
    }

    int insertVariable(Fraction equationCoefficient, Fraction matrixCoefficient = Fraction(0), int limitationIndex = -1)
    {
        int index = this->coefficients.keys()[this->coefficients.size() - 1] + 1;

        this->coefficients[index] = equationCoefficient;

        if (limitationIndex > -1)
        {
            this->matrix[limitationIndex][index] = matrixCoefficient;
        }

        this->positive_variables.push_back(index);

        return index;
    }

    void convert()
    {
        // мінімізуємо функцію
        Fraction partialCoefficient(1);

        if (this->orig_mode == "max")
        {
            partialCoefficient = Fraction(-1);
        }

        for (int i = 0; i < this->orig_coefficients.keys().size(); i++)
        {
            int key = this->orig_coefficients.keys().at(i);

            this->coefficients[key] = this->orig_coefficients[key] * partialCoefficient;
        }

        this->mode = "min";

        // робимо усі вільні члени додатніми
        for (int i = 0; i < this->orig_matrix.size(); i++)
        {
            this->matrix.push_back(this->orig_matrix.at(i));
            this->signs.push_back(this->orig_signs.at(i));

            if (this->orig_matrix.at(i)[-1] < Fraction(0))
            {
                for (int t = 0; t < this->orig_matrix.at(i).keys().size(); t++)
                {
                    this->matrix[i][this->orig_matrix.at(i).keys().at(t)] *= Fraction(-1);
                }

                if (this->orig_signs.at(i) == "<=")
                    this->signs[i] = ">="; else
                if (this->orig_signs.at(i) == ">=")
                    this->signs[i] = "<=";
            }
        }

        // заповнюємо коефіцієнти функції цілі, котрих нема нулями
        for (int i = 0; i < this->matrix.size(); i++)
        {
            FractionMap limitation = this->matrix.at(i);

            for (int t = 0; t < limitation.keys().size(); t++)
            {
                int key = limitation.keys().at(t);

                if (key != -1 && !this->coefficients.keys().contains(key))
                {
                    this->coefficients[key] = 0;
                }
            }
        }

        this->debugConvertedData();

        // обробка нерівностей виду "<="
        for (int i = 0; i < this->matrix.size(); i++)
        {
            if (this->signs.at(i) == "<=")
            {
                this->insertVariable(Fraction(0), Fraction(1), i);

                this->signs[i] = "=";
            }
        }

        this->debugConvertedData();

        // шукаємо нерівності виду ">=" в системі та віднімаємо від них
        // додаткові змінні, таким чином перетворюючи нерівності на рівняння
        for (int i = 0; i < this->matrix.size(); i++)
        {
            if (this->signs.at(i) == ">=")
            {
                int index = this->insertVariable(0);

                this->matrix[i][index] = Fraction(-1);
                this->signs[i] = "=";
            }
        }

        this->debugConvertedData();

        // обробка змінних, на які не накладено умову невід'ємності
        for (int i = 0; i < this->orig_positive_variables.size(); i++)
        {
            this->positive_variables.push_back(this->orig_positive_variables.at(i));
        }

        for (int i = 0; i < this->orig_coefficients.keys().size(); i++)
        {
            int index1 = this->orig_coefficients.keys().at(i);

            if (!this->positive_variables.contains(index1))
            {
                int index2 = this->insertVariable(this->orig_coefficients[index1] * Fraction(-1));

                for (int t = 0; t < this->matrix.size(); t++)
                {
                    if (this->matrix[t].keys().contains(index1))
                    {
                        this->matrix[t][index2] = this->matrix[t][index1] * Fraction(-1);
                    }
                }

                this->positive_variables.push_back(index1);
                this->positive_variables.push_back(index2);
            }
        }

        this->debugConvertedData();

        // додаємо базисні змінні там, де їх не вистачає
        for (int i = 0; i < this->matrix.size(); i++)
        {
            FractionMap limitation = this->matrix.at(i);
            bool fl1 = false;

            for (int t = 0; t < limitation.keys().size(); t++)
            {
                int index = limitation.keys().at(t);
                bool fl2 = false;

                for (int j = 0; j < this->matrix.size(); j++)
                {
                    if (j == i)
                    {
                        continue;
                    }

                    if (this->matrix.at(j).keys().contains(index))
                    {
                        fl2 = true;
                        break;
                    }
                }

                if (!fl2 && limitation[index] == Fraction(1))
                {
                    fl1 = true;
                    break;
                }
            }

            if (!fl1)
            {
                this->insertVariable(C_SIMPLEX_M, Fraction(1), i);
            }
        }
    }

    QMap<int, Fraction> getEquationCoefficients()
    {
        return this->coefficients;
    }

    QList<FractionMap> getLimitationCoefficients()
    {
        return this->matrix;
    }
};

#endif
