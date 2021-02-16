#include "geometry.h"
#include <cassert>

AbstractPoint::~AbstractPoint() = default;

Vector::Vector(const Position &pos) : AbstractPoint{pos} {
}

Position::Position(const Vector &vec) : AbstractPoint{vec} {
}

Rectangle::Rectangle(length_type width, length_type height, const Position &pos) :
        width_{width}, height_{height}, pos_{pos} {
    assert(((void) "Passed rectangle dimensions are nonpositive!", width_ > 0 && height_ > 0));
}

Rectangle &Rectangles::operator[](Rectangles::size_type i) {
    assert(i < size());
    return rectangles[i];
}

const Rectangle &Rectangles::operator[](Rectangles::size_type i) const {
    assert(i < size());
    return rectangles[i];
}

Position operator+(const Position &pos, const Vector &vec) {
    Position ans{pos};
    ans += vec;
    return ans;
}

Position operator+(const Vector &vec, const Position &pos) {
    return pos + vec;
}

Vector operator+(const Vector &vec1, const Vector &vec2) {
    Vector ans{vec1};
    ans += vec2;
    return ans;
}

Rectangle operator+(const Rectangle &rec, const Vector &vec) {
    Rectangle ans{rec};
    ans += vec;
    return ans;
}

Rectangle operator+(const Vector &vec, const Rectangle &rec) {
    return rec + vec;
}

Rectangles operator+(const Rectangles &recs, const Vector &vec) {
    Rectangles ans{recs};
    ans += vec;
    return ans;
}

Rectangles operator+(const Vector &vec, const Rectangles &recs) {
    return recs + vec;
}

Rectangles operator+(Rectangles &&recs, const Vector &vec) {
    Rectangles ans{std::move(recs)};
    ans += vec;
    return ans;
}

Rectangles operator+(const Vector &vec, Rectangles &&recs) {
    return std::move(recs) + vec;
}

namespace {
    // Checks whether rectangles a and b can be merged horizontally.
    bool can_merge_horizontally(const Rectangle &a, const Rectangle &b) {
        return a.width() == b.width() && a.pos() + Vector{0, a.height()} == b.pos();
    }

    // Checks whether rectangles a and b can be merged vertically.
    bool can_merge_vertically(const Rectangle &a, const Rectangle &b) {
        return a.height() == b.height() && a.pos() + Vector{a.width(), 0} == b.pos();
    }
}

Rectangle merge_horizontally(const Rectangle &a, const Rectangle &b) {
    assert(((void) "Can't merge rectangles horizontally!", can_merge_horizontally(a, b)));

    return {a.width(), a.height() + b.height(), a.pos()};
}

Rectangle merge_vertically(const Rectangle &a, const Rectangle &b) {
    assert(((void) "Can't merge rectangles vertically!", can_merge_vertically(a, b)));

    return {a.width() + b.width(), a.height(), a.pos()};
}

Rectangle merge_all(const Rectangles &rectangles) {
    assert(((void) "Trying to merge empty collection!", rectangles.size() > 0));

    Rectangle mergedPrefix = rectangles[0];

    for (Rectangles::size_type i = 1; i < rectangles.size(); i++) {
        if (can_merge_horizontally(mergedPrefix, rectangles[i])) {
            mergedPrefix = merge_horizontally(mergedPrefix, rectangles[i]);
        } else if (can_merge_vertically(mergedPrefix, rectangles[i])) {
            mergedPrefix = merge_vertically(mergedPrefix, rectangles[i]);
        } else {
            assert(((void) "Can't merge passed rectangles collection!", false));
        }
    }

    return mergedPrefix;
}
