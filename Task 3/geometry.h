#ifndef JNP1_GEOMETRY_H
#define JNP1_GEOMETRY_H

#include <cstdint>
#include <vector>
#include <initializer_list>

// Abstract class representing a point
// in a system with 2 coordinates.
class AbstractPoint {
public:
    // Alias for type of scalar coordinates.
    using scalar_type = int_fast32_t;

    // Deleted constructor of an abstract point.
    AbstractPoint() = delete;

    // Copy constructor of an abstract point.
    AbstractPoint(const AbstractPoint &) = default;

    // Copy assignment of an abstract point.
    AbstractPoint &operator=(const AbstractPoint &) = default;

    // Pure virtual destructor to make the class abstract.
    virtual ~AbstractPoint() = 0;

    // Constructor of AbstractPoint from x and y.
    AbstractPoint(scalar_type x, scalar_type y) : x_{x}, y_{y} {}

    // Returns x coordinate.
    [[nodiscard]] scalar_type x() const {
        return x_;
    }

    // Returns y coordinate.
    [[nodiscard]] scalar_type y() const {
        return y_;
    }

protected:
    // X coordinate.
    scalar_type x_;

    // Y coordinate.
    scalar_type y_;
};

class Vector;

class Position;

// Class representing vector in 2D plane.
class Vector : public AbstractPoint {
public:
    using AbstractPoint::AbstractPoint;

    // Copy constructor for vector.
    Vector(const Vector &) = default;

    // Copy assignment for vector.
    Vector &operator=(const Vector &) = default;

    // Explicit constructor for creating vector from position.
    explicit Vector(const Position &pos);

    // Returns reflection of vector over y = x.
    [[nodiscard]] Vector reflection() const {
        return {y_, x_};
    }

    // Compares two vectors.
    bool operator==(const Vector &other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

    // Translates vector by a given vector.
    Vector &operator+=(const Vector &other) {
        x_ += other.x_;
        y_ += other.y_;

        return *this;
    }
};

// Class representing position in 2D plane.
class Position : public AbstractPoint {
public:
    using AbstractPoint::AbstractPoint;

    // Copy constructor for position.
    Position(const Position &) = default;

    // Copy assignment for position.
    Position &operator=(const Position &) = default;

    // Explicit constructor for creating position from vector.
    explicit Position(const Vector &vec);

    // Returns reflection of position over y = x.
    [[nodiscard]] Position reflection() const {
        return {y_, x_};
    }

    // Compares two positions.
    bool operator==(const Position &other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

    // Translates position by a given vector.
    Position &operator+=(const Vector &vec) {
        x_ += vec.x();
        y_ += vec.y();

        return *this;
    }

    // Returns const reference to the origin of 2D plane.
    static const Position &origin() {
        static const Position zero{0, 0};

        return zero;
    }
};

// Class representing rectangle in 2D plane.
class Rectangle {
public:
    // Alias for a type of dimensions of rectangle.
    using length_type = int_fast32_t;

    // Alias for a type of an area of rectangle.
    using area_type = int_fast64_t;

    // Deleted default constructor for rectangle.
    Rectangle() = delete;

    // Copy constructor for rectangle.
    Rectangle(const Rectangle &) = default;

    // Copy assignment for rectangle.
    Rectangle &operator=(const Rectangle &) = default;

    // Constructs rectangle with given width, height and position of the lower left corner.
    Rectangle(length_type width, length_type height, const Position &pos = Position::origin());

    // Returns width of rectangle.
    [[nodiscard]] length_type width() const {
        return width_;
    }

    // Returns height of rectangle.
    [[nodiscard]] length_type height() const {
        return height_;
    }

    // Returns position of a lower left corner of rectangle.
    [[nodiscard]] Position pos() const {
        return pos_;
    }

    // Returns reflection of rectangle over y = x.
    [[nodiscard]] Rectangle reflection() const {
        return {height_, width_, pos_.reflection()};
    }

    // Returns area of rectangle.
    [[nodiscard]] area_type area() const {
        return static_cast<area_type>(width_) * height_;
    }

    // Compares two rectangles.
    bool operator==(const Rectangle &other) const {
        return width_ == other.width_ && height_ == other.height_ && pos_ == other.pos_;
    }

    // Translates rectangle by a given vector.
    Rectangle &operator+=(const Vector &vec) {
        pos_ += vec;

        return *this;
    }

private:
    // Width of rectangle.
    length_type width_;

    // Height of rectangle.
    length_type height_;

    // Position of lower left corner of rectangle.
    Position pos_;
};

// Class representing a collection of rectangles.
class Rectangles {
public:
    // Alias for type of rectangle collection size.
    using size_type = std::vector<Rectangle>::size_type;

    // Constructs empty collection.
    Rectangles() = default;

    // Copy constructor for rectangles.
    Rectangles(const Rectangles &) = default;

    // Copy assignment for rectangles.
    Rectangles &operator=(const Rectangles &) = default;

    // Move constructor for rectangles.
    Rectangles(Rectangles &&) noexcept = default;

    // Move assignment for rectangles.
    Rectangles &operator=(Rectangles &&) noexcept = default;

    // Constructor of rectangles from initializer_list.
    Rectangles(std::initializer_list<Rectangle> rectangles) : rectangles{rectangles} {}

    // Overloaded operator[] returning non-const reference to i-th rectangle.
    Rectangle &operator[](size_type i);

    // Overloaded operator[] returning const reference to i-th rectangle.
    const Rectangle &operator[](size_type i) const;

    // Returns size of collection of rectangles.
    [[nodiscard]] size_type size() const {
        return rectangles.size();
    }

    // Compares two collections of rectangles.
    bool operator==(const Rectangles &other) const {
        return rectangles == other.rectangles;
    }

    // Translates rectangles from collection by a given vector.
    Rectangles &operator+=(const Vector &vec) {
        for (auto &rectangle: rectangles) {
            rectangle += vec;
        }

        return *this;
    }

private:
    // Collection of rectangles.
    std::vector<Rectangle> rectangles;
};

// Overloaded operator+ for translating position by a vector.
Position operator+(const Position &pos, const Vector &vec);

// Overloaded operator+ for translating position by a vector.
Position operator+(const Vector &vec, const Position &pos);

// Overloaded operator+ for translating vector by a vector.
Vector operator+(const Vector &vec1, const Vector &vec2);

// Overloaded operator+ for translating rectangle by a vector.
Rectangle operator+(const Rectangle &rec, const Vector &vec);

// Overloaded operator+ for translating rectangle by a vector.
Rectangle operator+(const Vector &vec, const Rectangle &rec);

// Overloaded operator+ for translating collection of rectangles by a vector.
Rectangles operator+(const Rectangles &recs, const Vector &vec);

// Overloaded operator+ for translating collection of rectangles by a vector.
Rectangles operator+(const Vector &vec, const Rectangles &recs);

// Overloaded operator+ for translating collection of rectangles by a vector.
Rectangles operator+(Rectangles &&recs, const Vector &vec);

// Overloaded operator+ for translating collection of rectangles by a vector.
Rectangles operator+(const Vector &vec, Rectangles &&recs);

// Returns result of merging rectangles horizontally.
Rectangle merge_horizontally(const Rectangle &a, const Rectangle &b);

// Returns result of merging rectangles vertically.
Rectangle merge_vertically(const Rectangle &a, const Rectangle &b);

// Returns result of merging all rectangles horizontally or vertically from a given collection.
Rectangle merge_all(const Rectangles &rectangles);

#endif // JNP1_GEOMETRY_H