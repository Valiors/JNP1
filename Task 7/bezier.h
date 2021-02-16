#ifndef JNP1_BEZIER_H
#define JNP1_BEZIER_H

#include <cstddef>
#include <iostream>
#include <functional>
#include <cmath>
#include <array>
#include <vector>
#include <algorithm>

namespace bezier {
    namespace types {
        using real_t = double;
        using node_index_t = size_t;

        struct point_2d {
            point_2d(const real_t x, const real_t y) : X{x}, Y{y} {}

            friend types::point_2d operator+(const types::point_2d &a, const types::point_2d &b) {
                return {a.X + b.X, a.Y + b.Y};
            }

            friend types::point_2d operator*(const types::point_2d &p, const types::real_t a) {
                return {p.X * a, p.Y * a};
            }

            friend types::point_2d operator*(const types::real_t a, const types::point_2d &p) {
                return {p.X * a, p.Y * a};
            }

            friend bool operator==(const types::point_2d &a, const types::point_2d &b) {
                return a.X == b.X && a.Y == b.Y;
            }

            friend std::ostream &operator<<(std::ostream &os, const types::point_2d &p) {
                return os << '(' << p.X << ", " << p.Y << ')';
            }

            const real_t X;
            const real_t Y;
        };

        // Cubic curve is represented as a function nodex_index_t -> point_2d.
        using curve = std::function<const point_2d(const node_index_t)>;
    }

    namespace constants {
        constexpr types::node_index_t NUM_OF_CUBIC_BEZIER_NODES = 4;

        const types::real_t ARC = 4 * (sqrt(2) - 1) / 3;

        constexpr size_t DEFAULT_SEGMENT_COUNT = 1;
        constexpr size_t DEFAULT_RESOLUTION = 80;

        constexpr char DEFAULT_CURVE_MARK = '*';
        constexpr char DEFAULT_BACKGROUND_MARK = ' ';
    }

    namespace detail {
        constexpr size_t MINIMUM_POINT_DRAW_FROM_SEGMENT = 20;

        using segment_nodes = std::array<const types::point_2d, constants::NUM_OF_CUBIC_BEZIER_NODES>;

        inline types::curve CurveSegment(const types::point_2d &p0,
                                         const types::point_2d &p1,
                                         const types::point_2d &p2,
                                         const types::point_2d &p3) {
            segment_nodes nodes{p0, p1, p2, p3};

            return [nodes](const types::node_index_t index) {
                return index < constants::NUM_OF_CUBIC_BEZIER_NODES ?
                       nodes[index] :
                       throw std::out_of_range("a curve node index is out of range");
            };
        }

        inline types::point_2d RotatedPoint(const types::point_2d &p, const types::real_t a) {
            const types::real_t rad = (a / 180) * M_PI;
            const types::real_t sine = sin(rad);
            const types::real_t cosine = cos(rad);

            return {p.X * cosine - p.Y * sine, p.X * sine + p.Y * cosine};
        }

        inline types::point_2d ScaledPoint(const types::point_2d &p, const types::real_t x, const types::real_t y) {
            return {p.X * x, p.Y * y};
        }

        inline types::point_2d TranslatedPoint(const types::point_2d &p, const types::real_t x, const types::real_t y) {
            return p + types::point_2d{x, y};
        }

        inline types::point_2d B(const types::real_t t,
                                 const types::point_2d &p0,
                                 const types::point_2d &p1,
                                 const types::point_2d &p2,
                                 const types::point_2d &p3) {
            return p0 * (1 - t) * (1 - t) * (1 - t) +
                   3 * p1 * t * (1 - t) * (1 - t) +
                   3 * p2 * t * t * (1 - t) +
                   p3 * t * t * t;
        }

        inline bool isInPrintSquare(const types::point_2d &p) {
            return -1. <= p.X && p.X <= 1. && -1. <= p.Y && p.Y <= 1.;
        }

        inline detail::segment_nodes SegmentNodes(const types::curve &f, const size_t segment) {
            types::node_index_t firstPoint = segment * constants::NUM_OF_CUBIC_BEZIER_NODES;

            return {f(firstPoint), f(firstPoint + 1), f(firstPoint + 2), f(firstPoint + 3)};
        }
    }

    // Following functions work by creating a lambda performing specified operation.
    inline types::curve Cup() {
        return detail::CurveSegment({-1, 1}, {-1, -1}, {1, -1}, {1, 1});
    }

    inline types::curve Cap() {
        return detail::CurveSegment({-1, -1}, {-1, 1}, {1, 1}, {1, -1});
    }

    inline types::curve ConvexArc() {
        return detail::CurveSegment({0, 1}, {constants::ARC, 1}, {1, constants::ARC}, {1, 0});
    }

    inline types::curve ConcaveArc() {
        return detail::CurveSegment({0, 1}, {0, 1 - constants::ARC}, {1 - constants::ARC, 0}, {1, 0});
    }

    inline types::curve LineSegment(const types::point_2d &p, const types::point_2d &q) {
        return detail::CurveSegment(p, p, q, q);
    }

    inline types::curve MovePoint(const types::curve &f, const types::node_index_t i,
                                  const types::real_t x, const types::real_t y) {
        return [=](const types::node_index_t index) {
            return index == i ? f(index) + types::point_2d{x, y} : f(index);
        };
    }

    inline types::curve Rotate(const types::curve &f, const types::real_t a) {
        return [=](const types::node_index_t index) {
            return detail::RotatedPoint(f(index), a);
        };
    }

    inline types::curve Scale(const types::curve &f, const types::real_t x, const types::real_t y) {
        return [=](const types::node_index_t index) {
            return detail::ScaledPoint(f(index), x, y);
        };
    }

    inline types::curve Translate(const types::curve &f, const types::real_t x, const types::real_t y) {
        return [=](const types::node_index_t index) {
            return detail::TranslatedPoint(f(index), x, y);
        };
    }

    inline types::curve Concatenate(const types::curve &f1, const types::curve &f2) {
        return [=](const types::node_index_t index) {
            return index < constants::NUM_OF_CUBIC_BEZIER_NODES ?
                   f1(index) :
                   f2(index - constants::NUM_OF_CUBIC_BEZIER_NODES);
        };
    }

    template<typename... Args>
    inline types::curve Concatenate(const types::curve &f1, const types::curve &f2, const Args &... args) {
        return Concatenate(f1, Concatenate(f2, args...));
    }

    class P3CurvePlotter {
    public:
        explicit P3CurvePlotter(const types::curve &f,
                                const size_t segmentCount = constants::DEFAULT_SEGMENT_COUNT,
                                const size_t resolution = constants::DEFAULT_RESOLUTION) : resolution_(resolution) {
            const size_t pixelCount = resolution * resolution;

            // According to the message board on Moodle, it is enough to calculate
            // resolution^2 / segmentCount points per segment.
            const size_t theoreticalMinimum = std::ceil(pixelCount / static_cast<types::real_t>(segmentCount));

            // For safety, I calculate at least detail::MINIMUM_POINT_DRAW_FROM_SEGMENT points for each segment.
            const size_t pointsPerSegment = std::max(detail::MINIMUM_POINT_DRAW_FROM_SEGMENT, theoreticalMinimum);

            for (size_t i = 0; i < segmentCount; i++) {
                const auto [p0, p1, p2, p3] = detail::SegmentNodes(f, i);

                // Generate points B(t).
                for (size_t j = 0; j <= pointsPerSegment; j++) {
                    const types::real_t t = j / static_cast<types::real_t>(pointsPerSegment);

                    const types::point_2d point = detail::B(t, p0, p1, p2, p3);

                    if (detail::isInPrintSquare(point)) {
                        curvePoints.push_back(point);
                    }
                }
            }
        }

        void Print(std::ostream &s = std::cout,
                   char fb = constants::DEFAULT_CURVE_MARK,
                   char bg = constants::DEFAULT_BACKGROUND_MARK) const {
            std::vector<std::vector<char>> image(resolution_, std::vector<char>(resolution_, bg));

            // Translating points from cartesian plane to the grid.
            for (const auto &p : curvePoints) {
                const size_t x = (1. + p.X) / 2. * resolution_;
                const size_t y = (1. + p.Y) / 2. * resolution_;

                if (x < resolution_ && y < resolution_) {
                    image[resolution_ - y - 1][x] = fb;
                }
            }

            for (const auto &row : image) {
                for (const auto symbol : row) {
                    s << symbol;
                }

                s << std::endl;
            }
        }

        types::point_2d operator()(const types::curve &f,
                                   const types::real_t t,
                                   const size_t segment) const {
            const auto [p0, p1, p2, p3] = detail::SegmentNodes(f, segment - 1);

            return detail::B(t, p0, p1, p2, p3);
        }

    private:
        // Generated points from a curve are stored on a vector.
        std::vector<types::point_2d> curvePoints;
        const size_t resolution_;
    };
}

#endif // JNP1_BEZIER_H
