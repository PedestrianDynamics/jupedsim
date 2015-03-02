#ifndef KNOT_H
#define KNOT_H


class knot
{
public:
    knot();
    knot(const double &x, const double &y, const double &value=0);
    ~knot();

    void setValue(const double &value);
    const double &getValue() const;

private:
    double _x;
    double _y;
    double _value;
};

#endif // KNOT_H
