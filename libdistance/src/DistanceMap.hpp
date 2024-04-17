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

template <Arithmetic Arithmetic>
struct Point {
    Arithmetic x;
    Arithmetic y;
};

template <Arithmetic Arithmetic>
struct Polygon {
    std::vector<Point<Arithmetic>> points;
};

template <Arithmetic Arithmetic>
struct ExitLine {
    Point<Arithmetic> p1;
    Point<Arithmetic> p2;
};

template <Arithmetic Arithmetic>
struct ExitPolygon {
    Polygon<Point<Arithmetic>> polygon;
};

template <Arithmetic Arithmetic>
struct Line {
    Point<Arithmetic> p1;
    Point<Arithmetic> p2;
};

template <Arithmetic Arithmetic>
struct Arc {
    Point<Arithmetic> center;
    Arithmetic radius;
    Arithmetic startAngle;
    Arithmetic endAngle;
};

template <Arithmetic Arithmetic>
std::tuple<Arithmetic, Arithmetic, Arithmetic, Arithmetic>
ArcBoundingBox(const Arc<Arithmetic>& arc)
{
    // Lambda to convert degrees to radians if needed
    auto toRadians = [](Arithmetic angle) { return angle * (M_PI / 180.0); };

    // Lambda to calculate a point on the arc given an angle
    auto calculatePoint = [&](Arithmetic angle) {
        return Point<Arithmetic>{
            arc.center.x + arc.radius * std::cos(angle),
            arc.center.y + arc.radius * std::sin(angle)};
    };

    // Convert angles to radians if using degrees
    Arithmetic startAngle = toRadians(arc.startAngle);
    Arithmetic endAngle = toRadians(arc.endAngle);

    // Ensure the start angle is less than the end angle for comparison
    if(startAngle > endAngle) {
        std::swap(startAngle, endAngle);
    }

    // Initialize min and max points with start and end points of the arc
    Point<Arithmetic> minPoint = calculatePoint(startAngle);
    Point<Arithmetic> maxPoint = calculatePoint(endAngle);

    // Check for each critical angle within the arc's span
    std::array<Arithmetic, 4> criticalAngles = {0, M_PI_2, M_PI, 3 * M_PI_2};
    for(auto& angle : criticalAngles) {
        // Normalize the angle for comparison
        while(angle < startAngle)
            angle += 2 * M_PI;
        while(angle > endAngle)
            angle -= 2 * M_PI;

        if(angle >= startAngle && angle <= endAngle) {
            Point<Arithmetic> point = calculatePoint(angle);
            minPoint.x = std::min(minPoint.x, point.x);
            minPoint.y = std::min(minPoint.y, point.y);
            maxPoint.x = std::max(maxPoint.x, point.x);
            maxPoint.y = std::max(maxPoint.y, point.y);
        }
    }

    return {minPoint.x, minPoint.y, maxPoint.x, maxPoint.y};
}

template <Arithmetic Arithmetic>
struct Circle {
    Point<Arithmetic> center;
    Arithmetic radius;
};

inline int64_t ToIndex(int64_t x, int64_t y, int64_t xDim)
{
    return x + y * xDim;
}

inline std::pair<int64_t, int64_t> FromIndex(int64_t i, int64_t xDim)
{
    auto x = i % xDim;
    auto y = i / xDim;
    return {x, y};
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<int64_t, int64_t>
ToGrid(SignedIntegral x, SignedIntegral y, Arithmetic xMin, Arithmetic yMin);

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<int64_t, int64_t> ToGrid(Point<Arithmetic> p, Arithmetic xMin, Arithmetic yMin);

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<Arithmetic, Arithmetic> ToWorld(int64_t i, int64_t j, Arithmetic xMin, Arithmetic yMin);

template <SignedIntegral SignedIntegral>
class Map
{
    int64_t width;
    int64_t height;
    std::vector<SignedIntegral> data;

public:
    using ValueType = SignedIntegral; // Define ValueType for access in MapStencilView

    Map(int64_t width_, int64_t height_, SignedIntegral default_value = {})
        : width(width_), height(height_), data(width * height, default_value)
    {
    }

    SignedIntegral& At(int64_t x, int64_t y)
    {
        assert(x < width);
        assert(y < height);

        return data[x + y * width];
    }
    SignedIntegral At(int64_t x, int64_t y) const
    {
        assert(x < width);
        assert(y < height);

        return data[x + y * width];
    }

    int64_t Height() const { return height; }
    int64_t Width() const { return width; }

    std::span<const SignedIntegral> Data() const { return std::span{data}; };

    int64_t Size() const { return data.size(); }
};

template <typename MapType>
class MapStencilView
{
public:
    using MapValue = typename MapType::ValueType;

private:
    using SignedSizeT = int64_t;
    static constexpr MapValue OUT_OF_BOUNDS = std::numeric_limits<MapValue>::max();
    MapType& map;

    int64_t centerX;
    int64_t centerY;
    int64_t blockSize;

public:
    MapStencilView(MapType& map, int64_t centerX, int64_t centerY, int64_t blockSize)
        : map(map), centerX(centerX), centerY(centerY), blockSize(blockSize){};

    MapValue At(SignedSizeT x, SignedSizeT y) const
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

    void Set(SignedSizeT x, SignedSizeT y, MapValue value)
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
                if(val == -2147483647) {
                    output << fmt::format("   -   ");
                } else if(val == -2147483648) {
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
 * @tparam SignedIntegral
 * @tparam Arithmetic
 */
template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
class DistanceMap
{
private:
    using SignedSizeT = int64_t;

    // TODO(TS) data type
    Arithmetic xMin{};
    Arithmetic xMax{};
    Arithmetic yMin{};
    Arithmetic yMax{};

    SignedIntegral xDim{};
    SignedIntegral yDim{};

    Map<SignedIntegral> distance{};

    Map<SignedIntegral> localDistance{};
    MapStencilView<Map<SignedIntegral>> localDistanceStencil{};

    std::vector<std::tuple<SignedSizeT, SignedSizeT>> farToNearIndices{};

public:
    static constexpr Arithmetic CELL_SIZE = 0.2; // in meter
    static constexpr int64_t BLOCK_SIZE = 11; // size of one quadrant
    static constexpr int64_t FULL_BLOCK_SIZE = 2 * BLOCK_SIZE - 1;
    static constexpr SignedIntegral CELL_SIZE_CM =
        DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE * 100;

    static constexpr SignedIntegral FREE_SPACE = std::numeric_limits<SignedIntegral>::lowest() + 1;
    static constexpr SignedIntegral BLOCKED = std::numeric_limits<SignedIntegral>::lowest();

    DistanceMap(
        Arithmetic xMin,
        Arithmetic yMin,
        Arithmetic xMax,
        Arithmetic yMax,
        SignedIntegral xDim,
        SignedIntegral yDim,
        Map<SignedIntegral> distance,
        Map<SignedIntegral> personToIntermediate)
        : xMin(xMin)
        , xMax(xMax)
        , yMin(yMin)
        , yMax(yMax)
        , xDim(xDim)
        , yDim(yDim)
        , distance(std::move(distance))
        , localDistance(createLocalDistanceFull(personToIntermediate))
        , localDistanceStencil(localDistance, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE)
        , farToNearIndices(createFarToNearIndices(localDistance))
    {
    }

    Point<Arithmetic> GetNextTarget(const Point<Arithmetic>& position) const
    {
        auto [targetIndexX, targetIndexY] = findPositionInGrid(position);

        auto surplusDistance = computeSurplusDistance({targetIndexX, targetIndexY});
        MapStencilView<Map<SignedIntegral>> surplusDistanceToExitStencil(
            surplusDistance, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE);

        // find 0 value with the largest distance to center in surplusDistanceToExit
        for(auto const& [idx_x, idx_y] : farToNearIndices) {
            if(surplusDistanceToExitStencil.At(idx_x, idx_y) == 0) {
                targetIndexX += idx_x;
                targetIndexY += idx_y;
                const auto [x, y] =
                    ToWorld<SignedIntegral, Arithmetic>(targetIndexX, targetIndexY, xMin, yMin);
                return {x, y};
            }
        }

        std::string foo = surplusDistanceToExitStencil.Dump();
        throw std::runtime_error("ERROR!!!!");
    }

    const Map<SignedIntegral>& Distance() const { return distance; }

private:
    std::pair<int64_t, int64_t> findPositionInGrid(const Point<Arithmetic>& position) const
    {
        auto [positionX, positionY] = ToGrid<SignedIntegral, Arithmetic>(position, xMin, yMin);

        if(distance.At(positionX, positionY) != BLOCKED &&
           distance.At(positionX, positionY) != FREE_SPACE) {
            return {positionX, positionY};
        }

        const std::pair<int64_t, int64_t> neighbors[4] = {
            {positionX - 1, positionY},
            {positionX, positionY + 1},
            {positionX + 1, positionY},
            {positionX, positionY - 1}};

        auto min = std::numeric_limits<Arithmetic>::max();
        auto minNeighbor = std::make_pair<int64_t, int64_t>(0, 0);

        for(const auto& [neighborX, neighborY] : neighbors) {
            auto neighborValue = distance.At(neighborX, neighborY);
            if(neighborValue != BLOCKED && neighborValue != FREE_SPACE) {
                if(neighborValue < min) {
                    min = neighborValue;
                    minNeighbor = {neighborX, neighborY};
                }
            }
        }

        if(min != std::numeric_limits<Arithmetic>::max()) {
            return minNeighbor;
        }

        throw std::runtime_error("Could not find non blocked/free current position");
    }

    Map<SignedIntegral> createLocalDistanceFull(const Map<SignedIntegral>& localDistanceQuadrant)
    {
        Map<SignedIntegral> localDistanceFull(FULL_BLOCK_SIZE, FULL_BLOCK_SIZE);
        const SignedSizeT offset = BLOCK_SIZE - 1;

        for(int64_t x = 0; x < FULL_BLOCK_SIZE; ++x) {
            for(int64_t y = 0; y < FULL_BLOCK_SIZE; ++y) {
                auto xx = static_cast<SignedSizeT>(x - offset);
                auto yy = static_cast<SignedSizeT>(y - offset);
                localDistanceFull.At(x, y) = localDistanceQuadrant.At(std::abs(xx), std::abs(yy));
            }
        }

        return localDistanceFull;
    }

    std::vector<std::tuple<SignedSizeT, SignedSizeT>>
    createFarToNearIndices(const Map<SignedIntegral>& localDistances) const
    {
        std::vector<std::tuple<SignedSizeT, SignedSizeT>> indices{};
        const SignedSizeT dimension = 2 * BLOCK_SIZE - 1;
        indices.reserve(dimension * dimension);
        const SignedSizeT offset = BLOCK_SIZE - dimension;
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

    Map<SignedIntegral>
    computeSurplusDistance(const std::pair<int64_t, int64_t>& currentIndex) const
    {
        const auto [currentX, currentY] = currentIndex;
        const auto centerValue = distance.At(currentX, currentY);

        Map<SignedIntegral> surplusDistanceToExit(FULL_BLOCK_SIZE, FULL_BLOCK_SIZE);
        MapStencilView<Map<SignedIntegral>> surplusDistanceToExitStencil(
            surplusDistanceToExit, BLOCK_SIZE - 1, BLOCK_SIZE - 1, BLOCK_SIZE);

        // get stencil around position (values from distance map) -> distance
        MapStencilView distanceStencil(distance, currentX, currentY, BLOCK_SIZE);

        const auto strDistanceStencil = distanceStencil.Dump();

        // localDist + distance -> distanceToExit
        for(SignedSizeT y = BLOCK_SIZE - 1; y > static_cast<SignedSizeT>(-BLOCK_SIZE); --y) {
            for(auto x = static_cast<SignedSizeT>(-BLOCK_SIZE);
                x <= static_cast<SignedSizeT>(BLOCK_SIZE);
                ++x) {
                auto distanceToCenter = distanceStencil.At(x, y);
                auto localDistanceToCenter = localDistanceStencil.At(x, y);

                if(distanceToCenter == BLOCKED || distanceToCenter == FREE_SPACE) {
                    surplusDistanceToExitStencil.Set(x, y, BLOCKED);
                } else {
                    surplusDistanceToExitStencil.Set(
                        x, y, distanceToCenter + localDistanceToCenter);
                }
            }
        }
        const auto strLocalDistanceStencil = surplusDistanceToExitStencil.Dump();

        // distanceToExit - (value at position) -> surplusDistanceToExit
        for(SignedSizeT y = BLOCK_SIZE - 1; y > static_cast<SignedSizeT>(-BLOCK_SIZE); --y) {
            for(auto x = static_cast<SignedSizeT>(-BLOCK_SIZE);
                x <= static_cast<SignedSizeT>(BLOCK_SIZE);
                ++x) {

                if(surplusDistanceToExitStencil.At(x, y) == BLOCKED ||
                   surplusDistanceToExitStencil.At(x, y) == FREE_SPACE) {
                    surplusDistanceToExitStencil.Set(x, y, BLOCKED);
                } else {
                    surplusDistanceToExitStencil.Set(
                        x, y, surplusDistanceToExitStencil.At(x, y) - centerValue);
                }
            }
        }
        const auto strSurplusDistanceToExitStencil = surplusDistanceToExitStencil.Dump();

        return surplusDistanceToExit;
    }
};

template <SignedIntegral SignedIntegral>
void DumpDistanceMapMatplotlibCSV(const Map<SignedIntegral>& map, int i)
{
    std::ofstream file(fmt::format("dump_{:02}.csv", i));
    for(int64_t y = 0; y < map.Height(); ++y) {
        for(int64_t x = 0; x < map.Width(); ++x) {
            file << map.At(x, y);
            if(x < map.Width() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::vector<unsigned char> DumpDistanceMap(const Map<SignedIntegral>& map)
{
    const auto max = *std::max_element(
        std::begin(map.Data()), std::end(map.Data()), [](const auto& lhs, const auto& rhs) {
            if(lhs == DistanceMap<SignedIntegral, Arithmetic>::BLOCKED ||
               lhs == DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE) {
                return true;
            }
            return lhs < rhs;
        });

    std::vector<unsigned char> bytes(3 * map.Data().size());
    auto ptrBytes = bytes.data();

    for(const auto value : map.Data()) {
        if(value == DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
            *ptrBytes++ = 255;
            *ptrBytes++ = 0;
            *ptrBytes++ = 0;
        } else if(value == DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE || value == 0) {
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
    return bytes;
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
void PrintDistanceMap(const Map<SignedIntegral>& distance)
{
    using I = typename std::make_signed_t<SignedIntegral>;
    const auto xDim = distance.Width();
    const auto yDim = distance.Height();

    std::string output;
    for(I y = yDim - 1; y >= 0; --y) {
        for(int64_t x = 0; x < xDim; ++x) {
            const auto& val = distance.At(x, y);
            if(val == DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE) {
                output += fmt::format("  -  ");
            } else if(val == DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
                output += fmt::format("  x  ");
            } else {
                output += fmt::format("{:^4d} ", val);
            }
        }
        output += '\n';
    }
    std::cout << output << "\n";
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<int64_t, int64_t> ToGrid(Arithmetic x, Arithmetic y, Arithmetic xMin, Arithmetic yMin)
{
    // TODO(TS) asserts
    auto i = (x - xMin) / DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    auto j = (y - yMin) / DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    return {i, j};
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<int64_t, int64_t> ToGrid(Point<Arithmetic> p, Arithmetic xMin, Arithmetic yMin)
{
    // TODO(TS) asserts
    auto i = (p.x - xMin) / DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    auto j = (p.y - yMin) / DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    return {i, j};
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
std::pair<Arithmetic, Arithmetic> ToWorld(int64_t i, int64_t j, Arithmetic xMin, Arithmetic yMin)
{
    // TODO(TS) asserts
    auto x = xMin + i * DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE +
             0.5 * DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    auto y = yMin + j * DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE +
             0.5 * DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE;
    return {x, y};
}

template <SignedIntegral SignedIntegral, Arithmetic Arithmetic>
class DistanceMapBuilder
{
private:
    using SignedSizeT = int64_t;

    static constexpr SignedIntegral EXIT_VALUE = 0;
    static constexpr SignedIntegral OBSTACLE_VALUE =
        DistanceMap<SignedIntegral, Arithmetic>::BLOCKED;

    std::vector<Point<Arithmetic>> points{};
    std::vector<Line<Arithmetic>> lines{};
    std::vector<Polygon<Arithmetic>> filledPolygons{};
    std::vector<Polygon<Arithmetic>> outlinePolygons{};
    std::vector<Arc<Arithmetic>> arcs{};
    std::vector<Circle<Arithmetic>> circles{};

    std::vector<Point<Arithmetic>> exitPoints{};
    std::vector<Line<Arithmetic>> exitLines{};
    std::vector<Polygon<Arithmetic>> exitPolygons{};

    Arithmetic leftPadding{0};
    Arithmetic rightPadding{0};
    Arithmetic upPadding{0};
    Arithmetic downPadding{0};

    Arithmetic xMin{std::numeric_limits<Arithmetic>::max()};
    Arithmetic xMax{std::numeric_limits<Arithmetic>::lowest()};
    Arithmetic yMin{std::numeric_limits<Arithmetic>::max()};
    Arithmetic yMax{std::numeric_limits<Arithmetic>::lowest()};

    Map<SignedIntegral> distance{0, 0};

    std::tuple<Arithmetic, Arithmetic, Arithmetic, Arithmetic> computeBoundingBox()
    {

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

        // TODO(TS): add computation of BB arc and circle
        for(const auto& circle : circles) {
            xMin = std::min(xMin, circle.center.x - circle.radius);
            xMax = std::max(xMax, circle.center.x + circle.radius);
            yMin = std::min(yMin, circle.center.y - circle.radius);
            yMax = std::max(yMax, circle.center.y + circle.radius);
        }

        for(const auto& arc : arcs) {
            const auto [xMinArc, yMinArc, xMaxArc, yMaxArc] = ArcBoundingBox(arc);
            xMin = std::min(xMin, xMinArc);
            xMax = std::max(xMax, xMaxArc);
            yMin = std::min(yMin, yMinArc);
            yMax = std::max(yMax, yMaxArc);
        }
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

    void markObstacles()
    {
        for(const auto& point : points) {
            markPoint(point, OBSTACLE_VALUE);
        }
        // mark lines
        for(const auto& line : lines) {
            markLine(line, OBSTACLE_VALUE);
        }

        // mark polygons
        for(const auto& filledPolygon : filledPolygons) {
            markFilledPolygon(filledPolygon, OBSTACLE_VALUE);
        }
        for(const auto& polygonOutline : outlinePolygons) {
            markOutlinePolygon(polygonOutline, OBSTACLE_VALUE);
        }

        // mark arcs
        for(const auto& arc : arcs) {
            markArc(arc, OBSTACLE_VALUE);
        }

        // mark circles
        for(const auto& circle : circles) {
            markCircle(circle, OBSTACLE_VALUE);
        }
    }

    void markExits()
    {
        for(const auto& exitPoint : exitPoints) {
            markPoint(exitPoint, EXIT_VALUE);
        }

        // mark exit lines
        for(const auto& exitLine : exitLines) {
            markLine(exitLine, EXIT_VALUE);
        }

        // mark exit polygons
        for(const auto& exitPolygon : exitPolygons) {
            markFilledPolygon(exitPolygon, EXIT_VALUE);
        }
    }

    void markPoint(
        Point<Arithmetic> point,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        const auto [i, j] = ToGrid<SignedIntegral, Arithmetic>(point, xMin, yMin);
        if(distance.At(i, j) != DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
            distance.At(i, j) = fillValue;
        }
    }

    void markLine(
        Line<Arithmetic> line,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        const auto xDim = distance.Width();
        const auto yDim = distance.Height();
        const auto [i1, j1] = ToGrid<SignedIntegral, Arithmetic>(line.p1, xMin, yMin);
        const auto [i2, j2] = ToGrid<SignedIntegral, Arithmetic>(line.p2, xMin, yMin);

        const SignedSizeT si = i1 < i2 ? 1 : -1;
        const SignedSizeT sj = j1 < j2 ? 1 : -1;

        SignedSizeT dx = std::abs(static_cast<SignedSizeT>(i2 - i1));
        SignedSizeT dy = -std::abs(static_cast<SignedSizeT>(j2 - j1));

        SignedSizeT error = dx + dy, e2; // Initial error term

        SignedSizeT i = i1, j = j1;

        while(true) {
            if(i >= 0 && i < static_cast<SignedSizeT>(xDim) && j >= 0 && j < yDim &&
               distance.At(i, j) != DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
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
        Circle<Arithmetic> circle,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        markArc({circle.center, circle.radius, 0, 360}, fillValue);
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

    void markArc(
        Arc<Arithmetic> arc,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        auto startRad = arc.startAngle * (M_PI / 180.);
        auto endRad = arc.endAngle * (M_PI / 180.);

        if(endRad < startRad) {
            std::swap(endRad, startRad);
        }

        Arithmetic thetaStep = (arc.radius > 0.000001) ?
                                   DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE / arc.radius :
                                   1000000;

        for(Arithmetic theta = startRad; theta <= endRad; theta += thetaStep) {
            Arithmetic x = arc.center.x + arc.radius * cos(theta);
            Arithmetic y = arc.center.y + arc.radius * sin(theta);

            const auto [i, j] = ToGrid<SignedIntegral, Arithmetic>(x, y, xMin, yMin);
            if(distance.At(i, j) != DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
                distance.At(i, j) = fillValue;
            }
        }
    }

    void markFilledPolygon(
        Polygon<Arithmetic> polygon,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        std::vector<std::pair<SignedSizeT, SignedSizeT>> vertexIndices;
        vertexIndices.reserve(polygon.points.size());

        std::transform(
            std::begin(polygon.points),
            std::end(polygon.points),
            std::back_inserter(vertexIndices),
            [this](const auto& p) { return ToGrid<SignedIntegral, Arithmetic>(p, xMin, yMin); });

        for(int64_t y = 0; y < distance.Height(); ++y) {
            std::vector<SignedSizeT> xIntersections;

            for(int64_t i = 0; i < static_cast<int64_t>(vertexIndices.size()); ++i) {
                auto [x1, y1] = vertexIndices[i];
                auto [x2, y2] = vertexIndices[(i + 1) % static_cast<int64_t>(vertexIndices.size())];

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

            for(int64_t i = 0; i + 1 < static_cast<int64_t>(xIntersections.size()); i += 2) {
                for(SignedSizeT x = xIntersections[i]; x <= xIntersections[i + 1]; ++x) {
                    if(x >= 0 && x < distance.Width() &&
                       distance.At(x, y) != DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
                        distance.At(x, y) = fillValue;
                    }
                }
            }
        }
    }

    void markOutlinePolygon(
        Polygon<Arithmetic> polygon,
        SignedIntegral fillValue = DistanceMap<SignedIntegral, Arithmetic>::BLOCKED)
    {
        for(int64_t i = 0; i < static_cast<int64_t>(polygon.points.size()); ++i) {
            markLine(
                {polygon.points[i], polygon.points[(i + 1) % polygon.points.size()]}, fillValue);
        }
    }

    void prepareDistanceMap()
    {
        markObstacles();
        markExits();
    }

    void computeDistanceMap(const Map<SignedIntegral>& localDistance)
    {
        const auto xDim = distance.Width();
        const auto yDim = distance.Height();

        std::vector<bool> visited(distance.Size(), false);
        std::queue<std::pair<int64_t, int64_t>> queue;

        // find zero values and put in queue, mark as visited

        for(int64_t i = 0; i < distance.Size(); ++i) {
            const auto [x, y] = FromIndex(i, xDim);
            if(distance.At(x, y) == 0) {
                visited[i] = true;
                queue.emplace(x, y);
            }
        }

        while(!queue.empty()) {

            const auto [idx_x, idx_y] = queue.front();
            queue.pop();
            updateDistances(idx_x, idx_y, localDistance);

            const auto signed_idx_x = static_cast<int64_t>(idx_x);
            const auto signed_idx_y = static_cast<int64_t>(idx_y);

            const std::array<std::pair<SignedSizeT, SignedSizeT>, 4> displacement{
                {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

            for(auto [dx, dy] : displacement) {
                const auto neighbor_idx_x = signed_idx_x + dx;
                const auto neighbor_idx_y = signed_idx_y + dy;

                if(neighbor_idx_x >= 0 && neighbor_idx_x < xDim && neighbor_idx_y >= 0 &&
                   neighbor_idx_y < yDim &&
                   !visited[ToIndex(neighbor_idx_x, neighbor_idx_y, xDim)] &&
                   distance.At(neighbor_idx_x, neighbor_idx_y) !=
                       DistanceMap<SignedIntegral, Arithmetic>::BLOCKED &&
                   distance.At(neighbor_idx_x, neighbor_idx_y) !=
                       DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE) {
                    queue.emplace(neighbor_idx_x, neighbor_idx_y);
                    visited[ToIndex(neighbor_idx_x, neighbor_idx_y, xDim)] = true;
                }
            }
        }
    }

    void updateDistances(int64_t x, int64_t y, const Map<SignedIntegral>& localDistance)
    {
        const auto centerValue = distance.At(x, y);

        // spiral movement in to out
        const int movementDirections[2] = {-1, 1};

        for(const auto yMovementDirection : movementDirections) {
            const auto yLimit =
                static_cast<SignedSizeT>(DistanceMap<SignedIntegral, Arithmetic>::BLOCK_SIZE - 1) *
                yMovementDirection;

            for(const auto xMovementDirection : movementDirections) {
                auto xLimit = static_cast<SignedSizeT>(
                                  DistanceMap<SignedIntegral, Arithmetic>::BLOCK_SIZE - 1) *
                              xMovementDirection;

                auto xObstacleOffset = std::numeric_limits<SignedIntegral>::max();

                for(SignedSizeT yDisplacement = 0; yDisplacement != yLimit;
                    yDisplacement += yMovementDirection) {
                    const auto yCurrent = y + yDisplacement;

                    if(yCurrent < 0 || yCurrent >= distance.Height()) {
                        break;
                    }
                    for(SignedSizeT xDisplacement = 0; xDisplacement != xLimit;
                        xDisplacement += xMovementDirection) {
                        const auto xCurrent = x + xDisplacement;
                        if(xCurrent < 0 || xCurrent >= distance.Width()) {
                            xLimit = xCurrent;
                            break;
                        }

                        const auto valueCurrent = distance.At(xCurrent, yCurrent);

                        if(valueCurrent == DistanceMap<SignedIntegral, Arithmetic>::BLOCKED) {
                            xLimit = xDisplacement;
                            continue;
                        }

                        if(valueCurrent != DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE &&
                           valueCurrent < centerValue) {
                            continue;
                        }

                        const auto candidateValue =
                            centerValue +
                            localDistance.At(std::abs(xDisplacement), std::abs(yDisplacement));

                        if(valueCurrent == DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE ||
                           candidateValue < valueCurrent) {
                            distance.At(xCurrent, yCurrent) = candidateValue;
                        }
                    }
                }
            }
        }
    }

    Map<SignedIntegral> ComputeLocalDistance() const
    {
        Map<SignedIntegral> localDistance(
            DistanceMap<SignedIntegral, Arithmetic>::BLOCK_SIZE,
            DistanceMap<SignedIntegral, Arithmetic>::BLOCK_SIZE);
        for(int64_t x = 0; x < localDistance.Width(); ++x) {
            for(int64_t y = 0; y < localDistance.Height(); ++y) {
                localDistance.At(x, y) = static_cast<SignedIntegral>(
                    0.5 + DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE_CM *
                              std::sqrt(static_cast<float>(x * x + y * y)));
            }
        }
        return localDistance;
    }

public:
    void SetBoundingBox();

    void AddLeftPadding(Arithmetic leftPadding_) { leftPadding = leftPadding_; }
    void AddRightPadding(Arithmetic rightPadding_) { rightPadding = rightPadding_; }
    void AddDownPadding(Arithmetic downPadding_) { downPadding = downPadding_; }
    void AddUpPadding(Arithmetic upPadding_) { upPadding = upPadding_; }

    void AddPoint(const Point<Arithmetic>& point) { points.push_back(point); }
    void AddLine(const Line<Arithmetic>& line) { lines.emplace_back(line); }
    void AddFilledPolygon(const Polygon<Arithmetic>& polygon) { filledPolygons.push_back(polygon); }
    void AddOutlinePolygon(const Polygon<Arithmetic>& polygon)
    {
        outlinePolygons.push_back(polygon);
    }
    void AddArc(const Arc<Arithmetic>& arc) { arcs.emplace_back(arc); }
    void AddCircle(const Circle<Arithmetic>& circle) { circles.emplace_back(circle); }

    void AddExitPoint(const Point<Arithmetic>& exitPoint) { exitPoints.push_back(exitPoint); }
    void AddExitLine(const Line<Arithmetic>& exitLine) { exitLines.emplace_back(exitLine); }
    void AddExitPolygon(const Polygon<Arithmetic> exitPolygon)
    {
        exitPolygons.push_back(exitPolygon);
    };

    std::unique_ptr<const DistanceMap<SignedIntegral, Arithmetic>> Build()
    {
        if(exitPoints.empty() && exitLines.empty() && exitPolygons.empty()) {
            throw std::runtime_error("No exit lines and no exit polygons specified");
        }

        const auto boundingBox = computeBoundingBox();
        xMin = std::get<0>(boundingBox);
        yMin = std::get<1>(boundingBox);
        xMax = std::get<2>(boundingBox);
        yMax = std::get<3>(boundingBox);

        auto xDim = computeXGridSize(xMin, xMax);
        auto yDim = computeYGridSize(yMin, yMax);

        distance =
            Map<SignedIntegral>(xDim, yDim, DistanceMap<SignedIntegral, Arithmetic>::FREE_SPACE);

        auto localDistance = ComputeLocalDistance();

        prepareDistanceMap();

        computeDistanceMap(localDistance);

        DumpDistanceMapMatplotlibCSV(distance, 0);
        return std::make_unique<DistanceMap<SignedIntegral, Arithmetic>>(
            xMin, yMin, xMax, yMax, xDim, yDim, std::move(distance), std::move(localDistance));
    }

    SignedIntegral computeXGridSize(Arithmetic xMin, Arithmetic xMax) const
    {
        return static_cast<SignedIntegral>(
            (std::abs(xMax - xMin) + DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE) /
            DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE);
    }

    SignedIntegral computeYGridSize(Arithmetic yMin, Arithmetic yMax) const
    {
        return static_cast<SignedIntegral>(
            (std::abs(yMax - yMin) + DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE) /
            DistanceMap<SignedIntegral, Arithmetic>::CELL_SIZE);
    }
};
} // namespace distance
