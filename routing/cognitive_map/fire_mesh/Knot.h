#ifndef KNOT_H
#define KNOT_H


class Knot
{
public:
    Knot();
    Knot(const double &x, const double &y, const double &value=0);
    ~Knot();

    void SetValue(const double &value);
    const double &GetValue() const;

private:
    double _x;
    double _y;
    double _value;
};

#endif // KNOT_H
