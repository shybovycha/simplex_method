#ifndef __SIMPLEXCONVERTER_CPP__
#define __SIMPLEXCONVERTER_CPP__

#include <stdio.h>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QString>
#include <QRegExp>

#include "fraction.cpp"

typedef QList<Fraction> FractionList;
typedef QMap<int, Fraction> FractionMap;

class SimplexConverter
{
private:
    FractionMap orig_coefficients;
    QList<FractionMap> orig_matrix;
    QList<QString> orig_signs;
    QList<int> orig_positive_variables;
    QString orig_mode;

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

        QRegExp limitation_re("([^<>=]{1,})\\s*(.?=)\\s*(\\d+)", Qt::CaseInsensitive);

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

        for (int i = 0; i < this->orig_matrix.size(); i++)
        {
            FractionMap limitation = this->orig_matrix.at(i);

            qDebug("{");

            for (int t = 1; t < limitation.keys().size(); t++)
            {
                qDebug() << QString("%2 * X%1").arg(limitation.keys().at(t)).arg(limitation[limitation.keys().at(t)].toString());
            }

            qDebug() << QString("[%1] %2").arg(this->orig_signs.at(i)).arg(this->orig_matrix.at(i)[-1].toString());

            qDebug("}");
        }
    }
};

#endif
