#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <QDebug>

#include "fraction.cpp"

#define C_SIMPLEX_M 100

typedef QList<Fraction> FractionVector;
typedef QMap<int, Fraction> FractionMap;

#define E_NOT_ENOUGH_LIMITATIONS 1
#define E_LIMITATION_TOO_SMALL 2

#endif
