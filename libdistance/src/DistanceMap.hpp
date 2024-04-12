#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <span>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace distance
{
template <typename T>
concept SignedIntegral = std::is_integral_v<T> && std::is_signed_v<T>;

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <Arithmetic T>
struct Point {
    T x;
    T y;
};

template <Arithmetic T>
struct Polygon {
    std::vector<Point<T>> points;
};

template <Arithmetic T>
struct ExitLine {
    Point<T> p1;
    Point<T> p2;
};

template <Arithmetic T>
struct ExitPolygon {
    Polygon<Point<T>> polygon;
};

template <Arithmetic T>
struct Line {
    Point<T> p1;
    Point<T> p2;
};

template <Arithmetic T>
struct Arc {
    Point<T> center;
    T radius;
    T startAngle;
    T endAngle;
};

template <Arithmetic T>
struct Circle {
    Point<T> center;
    T radius;
};

inline size_t ToIndex(size_t x, size_t y, size_t xDim)
{
    return x + y * xDim;
}

inline std::pair<size_t, size_t> FromIndex(size_t i, size_t xDim)
{
    auto x = i % xDim;
    auto y = i / xDim;
    return {x, y};
}

template <SignedIntegral UI, Arithmetic A>
std::pair<size_t, size_t> ToGrid(UI x, UI y, A xMin, A yMin);

template <SignedIntegral UI, Arithmetic A>
std::pair<size_t, size_t> ToGrid(Point<A> p, A xMin, A yMin);

template <SignedIntegral UI, Arithmetic A>
std::pair<A, A> ToWorld(size_t i, size_t j, A xMin, A yMin);

template <SignedIntegral UI>
class Map
{
    size_t width;
    size_t height;
    std::vector<UI> data;

public:
    using ValueType = UI; // Define ValueType for access in MapStencilView

    Map(size_t width_, size_t height_, UI default_value = {})
        : width(width_), height(height_), data(width * height, default_value)
    {
    }

    UI& At(size_t x, size_t y) { return data[x + y * width]; }
    UI At(size_t x, size_t y) const { return data[x + y * width]; }

    size_t Height() const { return height; }
    size_t Width() const { return width; }

    std::span<const UI> Data() const { return std::span{data}; };

    size_t Size() const { return data.size(); }
};

template <typename MapType>
class MapStencilView
{
public:
    using UI = typename MapType::ValueType;

private:
    using SignedSizeT = typename std::make_signed_t<size_t>;
    static constexpr UI OUT_OF_BOUNDS = std::numeric_limits<UI>::max();
    MapType& map;

    size_t centerX;
    size_t centerY;
    size_t blockSize;

public:
    MapStencilView(MapType& map, size_t centerX, size_t centerY, size_t blockSize)
        : map(map), centerX(centerX), centerY(centerY), blockSize(blockSize){};

    UI At(SignedSizeT x, SignedSizeT y) const
    {
        assert(std::abs(x) <= blockSize);
        assert(std::abs(y) <= blockSize);

        SignedSizeT xMapped = centerX + x;
        SignedSizeT yMapped = centerY + y;

        if(xMapped < 0 || xMapped >= map.Width() || yMapped < 0 || yMapped >= map.Height()) {
            return OUT_OF_BOUNDS;
        }

        return map.At(xMapped, yMapped);
    }

    void Set(SignedSizeT x, SignedSizeT y, UI value)
    {
        assert(std::abs(x) <= blockSize);
        assert(std::abs(y) <= blockSize);

        SignedSizeT xMapped = centerX + x;
        SignedSizeT yMapped = centerY + y;

        if(xMapped < 0 || xMapped >= map.Width() || yMapped < 0 || yMapped >= map.Height()) {
            return;
        }

        map.At(xMapped, yMapped) = value;
    }

    std::string Dump() const
    {
        std::stringstream output;

        for(SignedSizeT y = blockSize - 1; y > static_cast<SignedSizeT>(-blockSize); --y) {
            for(SignedSizeT x = -(blockSize - 1); x < static_cast<SignedSizeT>(blockSize); ++x) {
                const auto& val = At(x, y);
                if(val == -2) {
                    output << fmt::format("   -   ");
                } else if(val == -1) {
                    output << fmt::format("   x   ");
                } else {
                    output << fmt::format("{:^6d} ", val);
                }
            }
            output << '\n';
        }
        output << '\n';

        return output.str();
    }
};

/**
 * Distance Map
 *
 * Index -> Grid cell center
 *
 * @tparam UI
 * @tparam A
 */
template <SignedIntegral UI, Arithmetic A>
class DistanceMap
{
private:
    using SignedT = typename std::make_signed_t<UI>;
    using SignedSizeT = typename std::make_signed_t<size_t>;

    // TODO(TS) data type
    A xMin{};
    A xMax{};
    A yMin{};
    A yMax{};

    UI xDim{};
    UI yDim{};

    Map<UI> distance{};

    Map<UI> localDistance{};
    MapStencilView<Map<UI>> localDistanceStencil{};

    std::vector<std::tuple<SignedSizeT, SignedSizeT>> farToNearIndices{};

public:
    static constexpr A CELL_SIZE = 0.2; // in meter
    static constexpr size_t BLOCK_SIZE = 11; // size of one quadrant
    static constexpr size_t FULL_BLOCK_SIZE = 2 * BLOCK_SIZE - 1;
    static constexpr UI CELL_SIZE_CM = DistanceMap<UI, A>::CELL_SIZE * 100;

    static constexpr UI FREE_SPACE = -2;
    static constexpr UI BLOCKED = -1;

    DistanceMap(
        A xMin,
        A yMin,
        A xMax,
        A yMax,
        UI xDim,
        UI yDim,
        Map<UI> distance,
        Map<UI> personToIntermediate)
        : xMin(xMin)
        , yMin(yMin)
        , xMax(xMax)
        , yMax(yMax)
        , xDim(xDim)
        , yDim(yDim)
        , distance(std::move(distance))
        , localDistance(createLocalDistanceFull(personToIntermediate))
        , localDistanceStencil(localDistance, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE)
        , farToNearIndices(createFarToNearIndices(localDistance))
    {
    }

    Point<A> GetNextTarget(const Point<A>& position) const
    {
        auto surplusDistance = computeSurplusDistance(position);
        MapStencilView<Map<UI>> surplusDistanceToExitStencil(
            surplusDistance, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE);

        auto [targetIndexX, targetIndexY] = ToGrid<UI, A>(position, xMin, yMin);

        // find 0 value with the largest distance to center in surplusDistanceToExit
        for(auto const& [idx_x, idx_y] : farToNearIndices) {
            const auto value = surplusDistanceToExitStencil.At(idx_x, idx_y);
            if(surplusDistanceToExitStencil.At(idx_x, idx_y) == 0) {
                targetIndexX += idx_x;
                targetIndexY += idx_y;
                const auto [x, y] = ToWorld<UI, A>(targetIndexX, targetIndexY, xMin, yMin);
                return {x, y};
            }
        }

        throw std::runtime_error("ERROR!!!!");
    }

private:
    Map<UI> createLocalDistanceFull(const Map<UI>& localDistanceQuadrant)
    {
        Map<UI> localDistanceFull(FULL_BLOCK_SIZE, FULL_BLOCK_SIZE);
        const SignedSizeT offset = BLOCK_SIZE - 1;

        for(size_t x = 0; x < FULL_BLOCK_SIZE; ++x) {
            for(size_t y = 0; y < FULL_BLOCK_SIZE; ++y) {
                auto xx = static_cast<SignedT>(x - offset);
                auto yy = static_cast<SignedT>(y - offset);
                localDistanceFull.At(x, y) = localDistanceQuadrant.At(std::abs(xx), std::abs(yy));
            }
        }

        return localDistanceFull;
    }

    std::vector<std::tuple<SignedSizeT, SignedSizeT>>
    createFarToNearIndices(const Map<UI>& localDistances) const
    {
        std::vector<std::tuple<SignedSizeT, SignedSizeT>> indices{};
        const SignedSizeT dimension = 2 * BLOCK_SIZE - 1;
        indices.reserve(dimension * dimension);
        const SignedT offset = BLOCK_SIZE - dimension;
        for(SignedSizeT x = 0; x < dimension; ++x) {
            for(SignedSizeT y = 0; y < dimension; ++y) {
                indices.emplace_back(x + offset, y + offset);
            }
        }
        MapStencilView localDistanceView(
            localDistances, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE);
        std::sort(
            std::begin(indices),
            std::end(indices),
            [&localDistanceView](const auto& tup_lhs, const auto& tup_rhs) {
                const auto& [x_lhs, y_lhs] = tup_lhs;
                const auto& [x_rhs, y_rhs] = tup_rhs;
                return localDistanceView.At(x_lhs, y_lhs) > localDistanceView.At(x_rhs, y_rhs);
            });
        return indices;
    }

    Map<UI> computeSurplusDistance(const Point<A>& position) const
    {
        const auto [currentX, currentY] = ToGrid<UI, A>(position, xMin, yMin);
        const auto centerValue = distance.At(currentX, currentY);

        Map<UI> surplusDistanceToExit(2 * BLOCK_SIZE - 1, 2 * BLOCK_SIZE - 1);
        MapStencilView<Map<UI>> surplusDistanceToExitStencil(
            surplusDistanceToExit, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE);

        // get stencil around position (values from distance map) -> distance
        MapStencilView distanceStencil(distance, currentX, currentY, BLOCK_SIZE);

        // localDist + distance -> distanceToExit
        for(SignedSizeT y = BLOCK_SIZE - 1; y > static_cast<SignedSizeT>(-BLOCK_SIZE); --y) {
            for(auto x = static_cast<SignedSizeT>(-BLOCK_SIZE);
                x <= static_cast<SignedSizeT>(BLOCK_SIZE);
                ++x) {
                auto distanceToCenter = distanceStencil.At(x, y);
                auto localDistanceToCenter = localDistanceStencil.At(x, y);

                if(distanceToCenter == BLOCKED) {
                    surplusDistanceToExitStencil.Set(x, y, BLOCKED);
                } else {
                    surplusDistanceToExitStencil.Set(
                        x, y, distanceToCenter + localDistanceToCenter);
                }
            }
        }

        // distanceToExit - (value at position) -> surplusDistanceToExit
        for(SignedSizeT y = BLOCK_SIZE - 1; y > static_cast<SignedSizeT>(-BLOCK_SIZE); --y) {
            for(auto x = static_cast<SignedSizeT>(-BLOCK_SIZE);
                x <= static_cast<SignedSizeT>(BLOCK_SIZE);
                ++x) {
                surplusDistanceToExitStencil.Set(
                    x, y, surplusDistanceToExitStencil.At(x, y) - centerValue);
            }
        }

        std::cout << "Local distance\n";
        std::cout << localDistanceStencil.Dump();

        std::cout << "Distance stencil\n";
        std::cout << distanceStencil.Dump();

        std::cout << "surplus stencil\n";
        std::cout << surplusDistanceToExitStencil.Dump();

        return surplusDistanceToExit;
    }
};

template <SignedIntegral UI>
void DumpDistanceMapMatplotlibCSV(const Map<UI>& map)
{
    std::ofstream file("dump.csv");
    for(size_t y = 0; y < map.Height(); ++y) {
        for(size_t x = 0; x < map.Width(); ++x) {
            file << map.At(x, y);
            if(x < map.Width() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

template <SignedIntegral UI, Arithmetic A>
std::vector<unsigned char> DumpDistanceMap(const Map<UI>& map)
{
    const auto max = *std::max_element(
        std::begin(map.Data()), std::end(map.Data()), [](const auto& lhs, const auto& rhs) {
            if(lhs == DistanceMap<UI, A>::BLOCKED || lhs == DistanceMap<UI, A>::FREE_SPACE) {
                return true;
            }
            return lhs < rhs;
        });

    std::vector<unsigned char> bytes(3 * map.Data().size());
    auto ptrBytes = bytes.data();

    for(const auto value : map.Data()) {
        if(value == DistanceMap<UI, A>::BLOCKED) {
            *ptrBytes++ = 255;
            *ptrBytes++ = 0;
            *ptrBytes++ = 0;
        } else if(value == DistanceMap<UI, A>::FREE_SPACE || value == 0) {
            *ptrBytes++ = 0;
            *ptrBytes++ = 255;
            *ptrBytes++ = 0;
        } else {
            *ptrBytes++ = 0;
            *ptrBytes++ = 0;
            *ptrBytes++ = static_cast<char>(
                static_cast<double>(value) / static_cast<double>(max) *
                std::numeric_limits<char>::max());
        }
    }
    //    std::transform(
    //        std::begin(map.Data()), std::end(map.Data()), std::begin(bytes), [&max](const UI&
    //        value) {
    //            return static_cast<char>(static_cast<double>(value)/static_cast<double>(max) *
    //            std::numeric_limits<char>::max());
    //        });
    return bytes;
}

template <SignedIntegral UI, Arithmetic A>
void PrintDistanceMap(const Map<UI>& distance)
{
    using I = typename std::make_signed_t<UI>; // Get the corresponding signed type
    const auto xDim = distance.Width();
    const auto yDim = distance.Height();

    std::string output;
    for(I y = yDim - 1; y >= 0; --y) {
        for(size_t x = 0; x < xDim; ++x) {
            const auto& val = distance.At(x, y);
            if(val == DistanceMap<UI, A>::FREE_SPACE) {
                output += fmt::format("  -  ");
            } else if(val == DistanceMap<UI, A>::BLOCKED) {
                output += fmt::format("  x  ");
            } else {
                output += fmt::format("{:^4d} ", val);
            }
        }
        output += '\n';
    }
    std::cout << output << "\n";
}

template <SignedIntegral UI, Arithmetic A>
std::pair<size_t, size_t> ToGrid(A x, A y, A xMin, A yMin)
{
    // TODO(TS) asserts
    auto i = (x - xMin) / DistanceMap<UI, A>::CELL_SIZE;
    auto j = (y - yMin) / DistanceMap<UI, A>::CELL_SIZE;
    return {i, j};
}

template <SignedIntegral UI, Arithmetic A>
std::pair<size_t, size_t> ToGrid(Point<A> p, A xMin, A yMin)
{
    // TODO(TS) asserts
    auto i = (p.x - xMin) / DistanceMap<UI, A>::CELL_SIZE;
    auto j = (p.y - yMin) / DistanceMap<UI, A>::CELL_SIZE;
    return {i, j};
}

template <SignedIntegral UI, Arithmetic A>
std::pair<A, A> ToWorld(size_t i, size_t j, A xMin, A yMin)
{
    // TODO(TS) asserts
    auto x = xMin + i * DistanceMap<UI, A>::CELL_SIZE + 0.5 * DistanceMap<UI, A>::CELL_SIZE;
    auto y = yMin + j * DistanceMap<UI, A>::CELL_SIZE + 0.5 * DistanceMap<UI, A>::CELL_SIZE;
    return {x, y};
}

template <SignedIntegral I, Arithmetic U>
class DistanceMapBuilder
{
private:
    using SignedT = typename std::make_signed_t<I>; // Get the corresponding signed type
    using SignedSizeT = typename std::make_signed_t<size_t>; // Get the corresponding signed type

    static constexpr I EXIT_VALUE = 0;
    static constexpr I OBSTACLE_VALUE = DistanceMap<I, U>::BLOCKED;

    std::vector<Point<U>> points{};
    std::vector<Line<U>> lines{};
    std::vector<Polygon<U>> filledPolygons{};
    std::vector<Polygon<U>> outlinePolygons{};
    std::vector<Arc<U>> arcs{};
    std::vector<Circle<U>> circles{};

    std::vector<Point<U>> exitPoints{};
    std::vector<Line<U>> exitLines{};
    std::vector<Polygon<U>> exitPolygons{};

    U leftPadding{0};
    U rightPadding{0};
    U upPadding{0};
    U downPadding{0};

    std::tuple<U, U, U, U> computeBoundingBox() const
    {
        U xMin{std::numeric_limits<U>::max()};
        U xMax{std::numeric_limits<U>::lowest()};
        U yMin{std::numeric_limits<U>::max()};
        U yMax{std::numeric_limits<U>::lowest()};

        for(const auto& point : points) {
            xMin = std::min(xMin, point.x);
            xMax = std::max(xMax, point.x);
            yMin = std::min(yMin, point.y);
            yMax = std::max(yMax, point.y);
        }

        for(const auto& line : lines) {
            xMin = std::min(xMin, std::min(line.p1.x, line.p2.x));
            xMax = std::max(xMax, std::max(line.p1.x, line.p2.x));
            yMin = std::min(yMin, std::min(line.p1.y, line.p2.y));
            yMax = std::max(yMax, std::max(line.p1.y, line.p2.y));
        }

        for(const auto& polygon : filledPolygons) {
            for(const auto& point : polygon.points) {
                xMin = std::min(xMin, point.x);
                xMax = std::max(xMax, point.x);
                yMin = std::min(yMin, point.y);
                yMax = std::max(yMax, point.y);
            }
        }

        for(const auto& polygon : outlinePolygons) {
            for(const auto& point : polygon.points) {
                xMin = std::min(xMin, point.x);
                xMax = std::max(xMax, point.x);
                yMin = std::min(yMin, point.y);
                yMax = std::max(yMax, point.y);
            }
        }

        //        for(const auto& arc : arcs) {
        //        }

        //        for(const auto& circle : circles) {
        //            xMin =
        //        }

        for(const auto& exitPoint : exitPoints) {
            xMin = std::min(xMin, exitPoint.x);
            xMax = std::max(xMax, exitPoint.x);
            yMin = std::min(yMin, exitPoint.y);
            yMax = std::max(yMax, exitPoint.y);
        }

        for(const auto& exitLine : exitLines) {
            xMin = std::min(xMin, std::min(exitLine.p1.x, exitLine.p2.x));
            xMax = std::max(xMax, std::max(exitLine.p1.x, exitLine.p2.x));
            yMin = std::min(yMin, std::min(exitLine.p1.y, exitLine.p2.y));
            yMax = std::max(yMax, std::max(exitLine.p1.y, exitLine.p2.y));
        }

        for(const auto& exitPolygon : exitPolygons) {
            for(const auto& exitPoint : exitPolygon.points) {
                xMin = std::min(xMin, exitPoint.x);
                xMax = std::max(xMax, exitPoint.x);
                yMin = std::min(yMin, exitPoint.y);
                yMax = std::max(yMax, exitPoint.y);
            }
        }

        xMin -= leftPadding;
        yMin -= downPadding;
        xMax += rightPadding;
        yMax += upPadding;

        return std::make_tuple(xMin, yMin, xMax, yMax);
    }

    void
    markObstacles(Map<I>& distance, U xMin, U yMin, I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        for(const auto& point : points) {
            markPoint(distance, xMin, yMin, point, OBSTACLE_VALUE);
        }
        // mark lines
        for(const auto& line : lines) {
            markLine(distance, xMin, yMin, line, OBSTACLE_VALUE);
        }

        // mark polygons
        for(const auto& filledPolygon : filledPolygons) {
            markFilledPolygon(distance, xMin, yMin, filledPolygon, OBSTACLE_VALUE);
        }
        for(const auto& polygonOutline : outlinePolygons) {
            markOutlinePolygon(distance, xMin, yMin, polygonOutline, OBSTACLE_VALUE);
        }

        // mark arcs
        for(const auto& arc : arcs) {
            markArc(distance, xMin, yMin, arc, OBSTACLE_VALUE);
        }

        // mark circles
        for(const auto& circle : circles) {
            markCircle(distance, xMin, yMin, circle, OBSTACLE_VALUE);
        }
    }

    void markExits(Map<I>& distance, U xMin, U yMin, I fillValue = 0) const
    {
        for(const auto& exitPoint : exitPoints) {
            markPoint(distance, xMin, yMin, exitPoint, EXIT_VALUE);
        }

        // mark exit lines
        for(const auto& exitLine : exitLines) {
            markLine(distance, xMin, yMin, exitLine, EXIT_VALUE);
        }

        for(const auto& exitPolygon : exitPolygons) {
            markFilledPolygon(distance, xMin, yMin, exitPolygon, OBSTACLE_VALUE);
        }

        // mark exit polygons
    }

    void markPoint(
        Map<I>& distance,
        U xMin,
        U yMin,
        Point<U> point,
        I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        const auto [i, j] = ToGrid<I, U>(point, xMin, yMin);
        distance.At(i, j) = fillValue;
    }

    void markLine(
        Map<I>& distance,
        U xMin,
        U yMin,
        Line<U> line,
        I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        const auto xDim = distance.Width();
        const auto yDim = distance.Height();
        const auto [i1, j1] = ToGrid<I, U>(line.p1, xMin, yMin);
        const auto [i2, j2] = ToGrid<I, U>(line.p2, xMin, yMin);

        const SignedT si = i1 < i2 ? 1 : -1;
        const SignedT sj = j1 < j2 ? 1 : -1;

        SignedT dx = std::abs(static_cast<SignedT>(i2 - i1));
        SignedT dy = -std::abs(static_cast<SignedT>(j2 - j1));

        SignedT error = dx + dy, e2; // Initial error term

        SignedT i = i1, j = j1;

        while(true) {
            if(i >= 0 && i < xDim && j >= 0 && j < yDim) {
                distance.At(i, j) = fillValue;
            }

            if(i == i2 && j == j2)
                break; // Check if the end point is reached

            e2 = 2 * error;
            if(e2 >= dy) { // Move to the next pixel in the x direction
                if(i == i2)
                    break; // Check if the end point is reached in x direction
                error += dy;
                i += si;
            }
            if(e2 <= dx) { // Move to the next pixel in the y direction
                if(j == j2)
                    break; // Check if the end point is reached in y direction
                error += dx;
                j += sj;
            }
        }
    }

    void markCircle(
        Map<I>& distance,
        U xMin,
        U yMin,
        Circle<U> circle,
        I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        markArc(distance, xMin, yMin, {circle.center, circle.radius, 0, 360}, fillValue);
    }
    //    void markCircle(int xm, int ym, int r, T value)
    //    {
    //        int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    //        do {
    //            (*this)[xm - x][ym + y] = value;
    //            (*this)[xm - y][ym - x] = value;
    //            (*this)[xm + x][ym - y] = value;
    //            (*this)[xm + y][ym + x] = value;
    //
    //            //          setPixel(xm-x, ym+y); /*   I. Quadrant */
    //            //          setPixel(xm-y, ym-x); /*  II. Quadrant */
    //            //          setPixel(xm+x, ym-y); /* III. Quadrant */
    //            //          setPixel(xm+y, ym+x); /*  IV. Quadrant */
    //            r = err;
    //            if(r <= y)
    //                err += ++y * 2 + 1; /* e_xy+e_y < 0 */
    //            if(r > x || err > y)
    //                err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    //        } while(x < 0);
    //    }

    void
    markArc(Map<I>& distance, U xMin, U yMin, Arc<U> arc, I fillValue = DistanceMap<I, U>::BLOCKED)
        const
    {
        auto startRad = arc.startAngle * (M_PI / 180.);
        auto endRad = arc.endAngle * (M_PI / 180.);

        if(endRad < startRad) {
            std::swap(endRad, startRad);
        }

        U thetaStep = (arc.radius > 0.000001) ? DistanceMap<I, U>::CELL_SIZE / arc.radius : 1000000;

        for(U theta = startRad; theta <= endRad; theta += thetaStep) {
            U x = arc.center.x + arc.radius * cos(theta);
            U y = arc.center.y + arc.radius * sin(theta);

            const auto [i, j] = ToGrid<I, U>(x, y, xMin, yMin);
            distance.At(i, j) = fillValue;
        }
    }

    void markFilledPolygon(
        Map<I>& distance,
        U xMin,
        U yMin,
        Polygon<U> polygon,
        I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        std::vector<std::pair<SignedSizeT, SignedSizeT>> vertexIndices;
        vertexIndices.reserve(polygon.points.size());

        std::transform(
            std::begin(polygon.points),
            std::end(polygon.points),
            std::back_inserter(vertexIndices),
            [xMin, yMin](const auto& p) { return ToGrid<I, U>(p, xMin, yMin); });

        for(size_t y = 0; y < distance.Height(); ++y) {
            std::vector<SignedSizeT> xIntersections;

            for(size_t i = 0; i < vertexIndices.size(); ++i) {
                auto [x1, y1] = vertexIndices[i];
                auto [x2, y2] = vertexIndices[(i + 1) % vertexIndices.size()];

                if(y1 == y2) {
                    continue; // Skip horizontal edges
                }

                // Check for scan line intersection
                if((static_cast<SignedSizeT>(y) >= y1 && static_cast<SignedSizeT>(y) < y2) ||
                   (static_cast<SignedSizeT>(y) < y1 && static_cast<SignedSizeT>(y) >= y2)) {
                    SignedSizeT xIntersect =
                        x1 + (static_cast<SignedSizeT>(y) - y1) * (x2 - x1) / (y2 - y1);
                    xIntersections.push_back(xIntersect);
                }
            }

            std::sort(xIntersections.begin(), xIntersections.end());

            for(size_t i = 0; i + 1 < xIntersections.size(); i += 2) {
                for(SignedSizeT x = xIntersections[i]; x <= xIntersections[i + 1]; ++x) {
                    if(x >= 0 && static_cast<size_t>(x) < distance.Width()) {
                        distance.At(x, y) = fillValue;
                    }
                }
            }
        }
    }

    void markOutlinePolygon(
        Map<I>& distance,
        U xMin,
        U yMin,
        Polygon<U> polygon,
        I fillValue = DistanceMap<I, U>::BLOCKED) const
    {
        for(size_t i = 0; i < polygon.points.size(); ++i) {
            markLine(
                distance,
                xMin,
                yMin,
                {polygon.points[i], polygon.points[(i + 1) % polygon.points.size()]},
                fillValue);
        }
    }

    void prepareDistanceMap(Map<I>& distance, U xMin, U yMin) const
    {
        markObstacles(distance, xMin, yMin);
        markExits(distance, xMin, yMin);
    }

    void computeDistanceMap(Map<I>& distance, const Map<I>& localDistance) const
    {
        const auto xDim = distance.Width();
        const auto yDim = distance.Height();

        std::vector<bool> visited(distance.Size(), false);
        std::queue<std::pair<size_t, size_t>> queue;

        // find zero values and put in queue, mark as visited

        for(size_t i = 0; i < distance.Size(); ++i) {
            const auto [x, y] = FromIndex(i, xDim);
            if(distance.At(x, y) == 0) {
                visited[i] = true;
                queue.emplace(x, y);
            }
        }

        while(!queue.empty()) {

            const auto [idx_x, idx_y] = queue.front();
            queue.pop();
            updateDistances(distance, xDim, yDim, idx_x, idx_y, localDistance);

            const auto signed_idx_x = static_cast<int64_t>(idx_x);
            const auto signed_idx_y = static_cast<int64_t>(idx_y);

            const std::array<std::pair<SignedT, SignedT>, 4> displacement{
                {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

            for(auto [dx, dy] : displacement) {
                const auto neighbor_idx_x = signed_idx_x + dx;
                const auto neighbor_idx_y = signed_idx_y + dy;

                if(neighbor_idx_x >= 0 && neighbor_idx_x < xDim && neighbor_idx_y >= 0 &&
                   neighbor_idx_y < yDim &&
                   !visited[ToIndex(neighbor_idx_x, neighbor_idx_y, xDim)] &&
                   distance.At(neighbor_idx_x, neighbor_idx_y) != DistanceMap<I, U>::BLOCKED &&
                   distance.At(neighbor_idx_x, neighbor_idx_y) != DistanceMap<I, U>::FREE_SPACE) {
                    queue.emplace(neighbor_idx_x, neighbor_idx_y);
                    visited[ToIndex(neighbor_idx_x, neighbor_idx_y, xDim)] = true;
                }
            }
        }
    }

    void updateDistances(
        Map<I>& distance,
        size_t xDim,
        size_t yDim,
        size_t x,
        size_t y,
        const Map<I>& localDistance) const
    {
        const auto centerValue = distance.At(x, y);

        // spiral movement in to out
        const int movementDirections[2] = {-1, 1};

        for(const auto yMovementDirection : movementDirections) {
            const auto yLimit =
                static_cast<SignedT>(DistanceMap<I, U>::BLOCK_SIZE - 1) * yMovementDirection;

            for(const auto xMovementDirection : movementDirections) {
                auto xLimit =
                    static_cast<SignedT>(DistanceMap<I, U>::BLOCK_SIZE - 1) * xMovementDirection;

                auto xObstacle = std::numeric_limits<I>::max();

                for(int yDisplacement = 0; std::abs(yDisplacement) <= std::abs(yLimit);
                    yDisplacement += yMovementDirection) {
                    const auto yCurrent = y + yDisplacement;

                    if(yCurrent >= yDim) {
                        break;
                    }
                    for(int xDisplacement = 0; std::abs(xDisplacement) <= std::abs(xLimit);
                        xDisplacement += xMovementDirection) {
                        const auto xCurrent = x + xDisplacement;
                        if(xCurrent >= xDim || xCurrent >= xObstacle) {
                            break;
                        }

                        const auto valueCurrent = distance.At(xCurrent, yCurrent);

                        if(valueCurrent == DistanceMap<I, U>::BLOCKED) {
                            xObstacle = xCurrent;
                            break;
                        }

                        if(valueCurrent != DistanceMap<I, U>::FREE_SPACE &&
                           valueCurrent < centerValue) {
                            continue;
                        }

                        const auto candidateValue =
                            centerValue +
                            localDistance.At(std::abs(xDisplacement), std::abs(yDisplacement));

                        if(valueCurrent == DistanceMap<I, U>::FREE_SPACE ||
                           candidateValue < valueCurrent) {
                            distance.At(xCurrent, yCurrent) = candidateValue;
                        }
                    }
                }
            }
        }
    }

    Map<I> ComputeLocalDistance() const
    {
        Map<I> localDistance(DistanceMap<I, U>::BLOCK_SIZE, DistanceMap<I, U>::BLOCK_SIZE);
        for(size_t x = 0; x < localDistance.Width(); ++x) {
            for(size_t y = 0; y < localDistance.Height(); ++y) {
                localDistance.At(x, y) = static_cast<I>(
                    0.5 +
                    DistanceMap<I, U>::CELL_SIZE_CM * std::sqrt(static_cast<float>(x * x + y * y)));
            }
        }
        return localDistance;
    }

public:
    void SetBoundingBox();

    void AddLeftPadding(U leftPadding_) { leftPadding = leftPadding_; }
    void AddRightPadding(U rightPadding_) { rightPadding = rightPadding_; }
    void AddDownPadding(U downPadding_) { downPadding = downPadding_; }
    void AddUpPadding(U upPadding_) { upPadding = upPadding_; }

    void AddPoint(const Point<U>& point) { points.push_back(point); }
    void AddLine(const Line<U>& line) { lines.emplace_back(line); }
    void AddFilledPolygon(const Polygon<U>& polygon) { filledPolygons.push_back(polygon); }
    void AddOutlinePolygon(const Polygon<U>& polygon) { outlinePolygons.push_back(polygon); }
    void AddArc(const Arc<U>& arc) { arcs.emplace_back(arc); }
    void AddCircle(const Circle<U>& circle) { circles.emplace_back(circle); }

    void AddExitPoint(const Point<U>& exitPoint) { exitPoints.push_back(exitPoint); }
    void AddExitLine(const Line<U>& exitLine) { exitLines.emplace_back(exitLine); }
    void AddExitPolygon(const Polygon<U> exitPolygon) {};

    std::unique_ptr<const DistanceMap<I, U>> Build() const
    {
        //        if (exitLines.empty() && exitPolygons.empty()){
        //            throw std::runtime_error("No exit lines and no exit polygons specified");
        //        }

        // compute bounding box -> mapping coordinate to grid cell
        const auto [xMin, yMin, xMax, yMax] = computeBoundingBox();
        //        std::cout << fmt::format("x: {} - {}\ty: {} - {}\n", xMin, xMax, yMin, yMax);

        //        auto distanceMap = std::make_unique<DistanceMap<T, U>>(xMin, yMin, xMax,
        //        yMax);
        auto xDim = computeXGridSize(xMin, xMax);
        auto yDim = computeYGridSize(yMin, yMax);
        std::cout << fmt::format("xDim: {} yDim: {}\n", xDim, yDim);

        Map<I> distance(xDim, yDim, DistanceMap<I, U>::FREE_SPACE);

        auto localDistance = ComputeLocalDistance();

        //        std::cout << "empty: \n";
        //        PrintDistanceMap<T, U>(distance, xDim, yDim);

        prepareDistanceMap(distance, xMin, yMin);
        //        std::cout << "Prepared: \n";
        //        PrintDistanceMap<T, U>(distance);

        computeDistanceMap(distance, localDistance);

        std::cout << "Final: \n";
        PrintDistanceMap<I, U>(distance);

        //        auto bytes = DumpDistanceMap<T, U>(distance);
        //        std::fstream out("dump.data", std::ios::trunc | std::ios::binary |
        //        std::ios::out); if(out.good()) {
        //            out.write(bytes.data(), bytes.size());
        //        }
        //        out.close();

        DumpDistanceMapMatplotlibCSV(distance);
        return std::make_unique<DistanceMap<I, U>>(
            xMin, yMin, xMax, yMax, xDim, yDim, std::move(distance), std::move(localDistance));
    }

    I computeXGridSize(U xMin, U xMax) const
    {
        return static_cast<I>(
            (std::abs(xMax - xMin) + DistanceMap<I, U>::CELL_SIZE) / DistanceMap<I, U>::CELL_SIZE);
    }

    I computeYGridSize(U yMin, U yMax) const
    {
        return static_cast<I>(
            (std::abs(yMax - yMin) + DistanceMap<I, U>::CELL_SIZE) / DistanceMap<I, U>::CELL_SIZE);
    }
};
} // namespace distance
