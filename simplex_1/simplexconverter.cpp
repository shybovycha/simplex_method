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
    SimplexConverter(QString equation, QList<QString> limitations = QList<QString>(), QList<QString> positive_variables = QList<QString>())
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

        for (int i = 0; i < this->orig_coefficients.keys().size(); i++)
        {
            qDebug() << QString("c[%1] => %2").arg(this->orig_coefficients.keys().at(i)).arg(this->orig_coefficients[this->orig_coefficients.keys().at(i)].toString());
        }

        if (this->orig_mode == "min")
            qDebug("function is minimizing"); else
        if (this->orig_mode == "max")
            qDebug("function is maximizing"); else
                qDebug() << QString("function has unknown state '%1'").arg(this->orig_mode);

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
    }

    void debugOriginalData()
    {
        QString function_line("f(x) = ");

        for (int i = 0; i < this->orig_coefficients.keys().size(); i++)
        {
            function_line += QString(" + %2 * X%1").arg(this->orig_coefficients.keys().at(i)).arg(this->orig_coefficients[this->orig_coefficients.keys().at(i)].toString());
        }

        function_line += QString(" -> %1").arg(this->orig_mode);

        qDebug(function_line.toStdString().c_str());

        qDebug("{");

        for (int i = 0; i < this->orig_matrix.size(); i++)
        {
            FractionMap limitation = this->orig_matrix.at(i);

            QString line;

            for (int t = 1; t < limitation.keys().size(); t++)
            {
                line += QString(" + %2 * X%1").arg(limitation.keys().at(t)).arg(limitation[limitation.keys().at(t)].toString());
            }

            line += QString(" %1 %2 ").arg(this->orig_signs.at(i)).arg(this->orig_matrix.at(i)[-1].toString());

            qDebug(line.toStdString().c_str());
        }

        qDebug("}");
    }

    void debugConvertedData()
    {
        QString function_line("f(x) = ");

        for (int i = 0; i < this->coefficients.keys().size(); i++)
        {
            function_line += QString(" + %2 * X%1").arg(this->coefficients.keys().at(i)).arg(this->coefficients[this->coefficients.keys().at(i)].toString());
        }

        function_line += QString(" -> %1").arg(this->mode);

        qDebug(function_line.toStdString().c_str());

        qDebug("{");

        for (int i = 0; i < this->matrix.size(); i++)
        {
            FractionMap limitation = this->matrix.at(i);

            QString line;

            for (int t = 1; t < limitation.keys().size(); t++)
            {
                line += QString(" + %2 * X%1").arg(limitation.keys().at(t)).arg(limitation[limitation.keys().at(t)].toString());
            }

            line += QString(" %1 %2 ").arg(this->signs.at(i)).arg(this->matrix.at(i)[-1].toString());

            qDebug(line.toStdString().c_str());
        }

        qDebug("}");
    }

    int insertVariable(Fraction equationCoefficient, Fraction matrixCoefficient = Fraction(0), int limitationIndex = -1)
    {
        int index = this->coefficients[this->coefficients.size() - 1] + 1;

        this->coefficients[index] = equationCoefficient;

        if (limitationIndex > -1)
        {
            this->matrix[limitationIndex][index] = matrixCoefficient;
        }

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

        // обробка нерівностей виду "<="
        for (int i = 0; i < this->matrix.size(); i++)
        {
            if (this->signs.at(i) == "<=")
            {
                int index = this->coefficients.size() + 1;

                this->coefficients[index] = Fraction(0);
                this->matrix[i][index] = Fraction(1);
                this->signs[i] = "=";
            }
        }

        // шукаємо нерівності виду ">=" в системі та рівняння у вихідній системі
        bool geEquationsPresent = false;
        bool eqEquationsPresent = false;

        for (int i = 0; i < this->matrix.size(); i++)
        {
            if (this->signs.at(i) == ">=")
            {
                for (int t = 0; t < this->orig_matrix.size(); t++)
                {
                    if (i == t)
                    {
                        continue;
                    }

                    if (this->signs.at(t) == "=")
                    {
                        eqEquationsPresent = true;
                        break;
                    }
                }

                geEquationsPresent = true;
                break;
            }
        }

        // випадок, коли в системі присутні нерівності виду ">="
        // і в оригінальній системі відсутні рівняння
        if (geEquationsPresent && !eqEquationsPresent)
        {
        } else
        {

        }
    }
};

#endif
