class Fraction
{
public:
    int a, b;

public:
    Fraction(int _a = 0, int _b = 1)
    {
        this->a = _a;
        this->b = _b;
    }

    static int gcd(int a, int b)
    {
        return (!b) ? a : gcd(b, a % b);
    }

    Fraction simplify() const
    {
        int k = 2, x = this->a, y = this->b;

        while (k > 1)
        {
            k = gcd(x, y);
            x /= k;
            y /= k;
        }

        return Fraction(x, y);
    }

    Fraction(const Fraction &v)
    {
        this->a = v.a;
        this->b = v.b;
    }

    Fraction& operator=(const Fraction& v)
    {
        this->a = v.a;
        this->b = v.b;

        return *this;
    }

    Fraction operator+(const Fraction& v) const
    {
        return Fraction((this->a * v.b) + (this->b * v.a), this->b + v.b);
    }

    Fraction operator+=(const Fraction& v)
    {
        this->a = (this->a * v.b) + (this->b * v.a);
        this->b = this->b * v.b;

        return *this;
    }

    Fraction operator-=(const Fraction& v)
    {
        this->a = (this->a * v.b) - (this->b * v.a);
        this->b = this->b * v.b;

        return *this;
    }

    Fraction operator-(const Fraction& b) const
    {
        return Fraction((this->a * b.b) - (this->b * b.a), this->b + b.b);
    }

    Fraction operator*(const Fraction& b) const
    {
        return Fraction(this->a * b.a, this->b * b.b);
    }

    Fraction operator/(const Fraction& b) const
    {
        return Fraction(this->a * b.b, this->b * b.a);
    }

    bool operator>(const Fraction& b) const
    {
        return ((*this - b).a > 0);
    }

    bool operator<(const Fraction& b) const
    {
        return ((*this - b).a < 0);
    }

    bool operator==(const Fraction& v) const
    {
        Fraction a = v, b(this->a, this->b);

        a = a.simplify();
        b = b.simplify();

        return (a.a == b.a && a.b == b.b);
    }

    bool operator!=(const Fraction& v) const
    {
        Fraction x = this->simplify(), y = v.simplify();

        return (x.a != y.a && x.b != y.b);
    }

    QString toString()
    {
        return QString("%1/%2").arg(this->a).arg(this->b);
    }
};
