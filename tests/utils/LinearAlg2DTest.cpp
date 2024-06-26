// Copyright (c) 2024 UltiMaker
// CuraEngine is released under the terms of the AGPLv3 or higher.

#include "utils/linearAlg2D.h"

#include <cstdint>
#include <numbers>

#include <gtest/gtest.h>

#include "geometry/Point3Matrix.h"

// NOLINTBEGIN(*-magic-numbers)
namespace cura
{

/*
 * Parameters provided to getDist2FromLineSegment.
 */
struct GetDist2FromLineSegmentParameters
{
    Point2LL line_start;
    Point2LL line_end;
    Point2LL point;
    coord_t actual_distance2;
    int16_t actual_is_beyond;

    GetDist2FromLineSegmentParameters(Point2LL line_start, Point2LL line_end, Point2LL point, coord_t actual_distance2, int16_t actual_is_beyond)
        : line_start(line_start)
        , line_end(line_end)
        , point(point)
        , actual_distance2(actual_distance2)
        , actual_is_beyond(actual_is_beyond)
    {
    }
};

/*
 * Fixture to allow parameterized tests for getDist2FromLineSegment.
 */
class GetDist2FromLineSegmentTest : public testing::TestWithParam<GetDist2FromLineSegmentParameters>
{
public:
    /*
     * Maximum allowed distance error in measurements due to rounding, in
     * microns.
     */
    const coord_t maximum_error = 10;
};

TEST_P(GetDist2FromLineSegmentTest, GetDist2FromLineSegment)
{
    const GetDist2FromLineSegmentParameters parameters = GetParam();
    const Point2LL line_start = parameters.line_start;
    const Point2LL line_end = parameters.line_end;
    const Point2LL point = parameters.point;
    const coord_t actual_distance2 = parameters.actual_distance2;
    const int16_t actual_is_beyond = parameters.actual_is_beyond;

    // FIXME: or at least review this. The optional output parameter supposed_is_beyond in LinearAlg2D::getDist2FromLineSegment()
    // may not always be set. In many cases, the value will not be set so the original value remains. In many test cases
    // supposed_is_beyond is expected to be 0 while it's not true: supposed_is_beyond is expected to remain the same.
    // I initialize supposed_is_beyond to 0 here to fix this for now, but I think this probably should be fixed in
    // LinearAlg2D::getDist2FromLineSegment() so it doesn't have those undefined behaviour.
    int16_t supposed_is_beyond = 0;
    const coord_t supposed_distance = LinearAlg2D::getDist2FromLineSegment(line_start, point, line_end, &supposed_is_beyond);

    // FIXME: Clean-up message with ftm when CURA-8258 is implemented or when we use C++20
    ASSERT_LE(std::fabs(sqrt(double(supposed_distance)) - sqrt(double(actual_distance2))), maximum_error)
        << "Line [" << line_start.X << ", " << line_start.Y << "] -- [" << line_end.X << ", " << line_end.Y << "], point [" << point.X << ", " << point.Y
        << "], squared distance was " << supposed_distance << " rather than " << actual_distance2 << ".";
    ASSERT_EQ(supposed_is_beyond, actual_is_beyond) << "Line [" << line_start.X << ", " << line_start.Y << "] -- [" << line_end.X << ", " << line_end.Y << "], point [" << point.X
                                                    << ", " << point.Y << "], check whether it is beyond was " << static_cast<int>(supposed_is_beyond) << " rather than "
                                                    << static_cast<int>(actual_is_beyond) << ".";
}

INSTANTIATE_TEST_CASE_P(
    GetDist2FromLineSegmentInstantiation,
    GetDist2FromLineSegmentTest,
    testing::Values(
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(25, 3), 9, 0), // Nearby a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(25, 0), 0, 0), // On a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(200, 0), 10000, 1), // Beyond a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(-100, 0), 10000, -1), // Before a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(-1, -1), 2, -1), // In a corner near a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 0), Point2LL(0, 3), 9, 0), // Perpendicular to a horizontal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(5, 25), 25, 0), // Nearby a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(0, 25), 0, 0), // On a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(0, 200), 10000, 1), // Beyond a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(0, -100), 10000, -1), // Before a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(-1, -1), 2, -1), // In a corner near a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 100), Point2LL(3, 0), 9, 0), // Perpendicular to a vertical line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(30, 20), 50, 0), // Nearby a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(25, 25), 0, 0), // On a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(200, 200), 20000, 1), // Beyond a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(-100, -100), 20000, -1), // Before a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(-3, 0), 9, -1), // In a corner near a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 100), Point2LL(3, -3), 9, 0), // Perpendicular to a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(20, 30), 320, 0), // Nearby a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(40, 20), 0, 0), // On a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(0, 0), 0, 0), // On one of the vertices of the diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(200, 100), 12500, 1), // Beyond a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(-100, -50), 12500, -1), // Before a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(-3, 0), 9, -1), // In a corner near a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(100, 50), Point2LL(-2, 4), 20, 0), // Perpendicular to a diagonal line.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(10000, 5000), Point2LL(2000, 3000), 3200000, 0), // Longer distances.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 0), Point2LL(20, 0), 400, 0), // Near a line of length 0.
        GetDist2FromLineSegmentParameters(Point2LL(0, 0), Point2LL(0, 0), Point2LL(0, 0), 0, 0) // On a line of length 0.
        ));

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct GetAngleParameters
{
    Point2LL a;
    Point2LL b;
    Point2LL c;
    double angle; // In degrees.

    GetAngleParameters(Point2LL a, Point2LL b, Point2LL c, double angle)
        : a(a)
        , b(b)
        , c(c)
        , angle(angle)
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

/*
 * Fixture to allow parameterized tests for getAngle.
 */
class GetAngleTest : public testing::TestWithParam<GetAngleParameters>
{
public:
    /*
     * Maximum allowed error in the angle measurement.
     */
    const double maximum_error = 1.0;
};

TEST_P(GetAngleTest, GetAngle)
{
    const GetAngleParameters parameters = GetParam();
    const Point2LL a = parameters.a;
    const Point2LL b = parameters.b;
    const Point2LL c = parameters.c;
    const double angle_degrees = parameters.angle;
    const double angle = angle_degrees * std::numbers::pi / 180.0;

    const double supposed_angle = LinearAlg2D::getAngleLeft(a, b, c);
    ASSERT_LE(std::fabs(angle - supposed_angle), maximum_error)
        << "Corner in " << a << " - " << b << " - " << c << " was computed to have an angle of " << supposed_angle << " instead of " << angle << ".";
}

INSTANTIATE_TEST_CASE_P(
    GetAngleInstantiation,
    GetAngleTest,
    testing::Values(
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(100, 1), 180), // Almost straight line.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(100, 0), 180), // Completely straight line.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(-100, -100), 315), //-45 degrees.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(0, -100), 270), //-90 degrees.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(0, 100), 90), // Straight angle.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(-100, 1), 0), // Almost straight back.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(-100, -1), 360), // Almost straight back but the other way around.
        GetAngleParameters(Point2LL(-100, 0), Point2LL(0, 0), Point2LL(-100, 0), 0) // Completely straight back.
        ));

TEST(GetAngleTest, GetAngleLeftAABTest)
{
    LinearAlg2D::getAngleLeft(Point2LL(0, 0), Point2LL(0, 0), Point2LL(100, 0)); // Any output is allowed. Just don't crash!
}

TEST(GetAngleTest, GetAngleLeftABBTest)
{
    LinearAlg2D::getAngleLeft(Point2LL(0, 0), Point2LL(100, 0), Point2LL(100, 100)); // Any output is allowed. Just don't crash!
}

TEST(GetAngleTest, GetAngleLeftAAATest)
{
    LinearAlg2D::getAngleLeft(Point2LL(0, 0), Point2LL(0, 0), Point2LL(0, 0)); // Any output is allowed. Just don't crash!
}

TEST(PointIsLeftOfLineTest, LeftOfLine)
{
    constexpr short actual = 1;
    const Point2LL p(0, 10); //    ^
    const Point2LL a(10, 0); //  . |
    const Point2LL b(10, 20); //   |

    const coord_t supposed = LinearAlg2D::pointIsLeftOfLine(p, a, b);

    // FIXME: Clean-up message with ftm when CURA-8258 is implemented or when we use C++20
    ASSERT_TRUE(actual * supposed > 0 || (actual == 0 && supposed == 0))
        << "Point " << p << " was computed as lying " << ((supposed == 0) ? "on" : ((supposed < 0) ? "left" : "right")) << " the line from " << a << " to " << b << ", instead of "
        << ((actual == 0) ? "on" : ((actual < 0) ? "left" : "right"));
}

TEST(PointIsLeftOfLineTest, Sharp)
{
    constexpr short actual = -1;
    const Point2LL p(3896, 3975); // ^
    const Point2LL a(1599, 3975); //  \    .
    const Point2LL b(200, 3996); //    \                                      .

    const coord_t supposed = LinearAlg2D::pointIsLeftOfLine(p, a, b);
    // FIXME: Clean-up message with ftm when CURA-8258 is implemented or when we use C++20
    ASSERT_TRUE(actual * supposed > 0 || (actual == 0 && supposed == 0))
        << "Point " << p << " was computed as lying " << ((supposed == 0) ? "on" : ((supposed < 0) ? "left" : "right")) << " the line from " << a << " to " << b << ", instead of "
        << ((actual == 0) ? "on" : ((actual < 0) ? "left" : "right"));
}

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct GetPointOnLineWithDistParameters
{
    Point2LL p;
    Point2LL a;
    Point2LL b;
    coord_t dist;
    Point2LL actual_result;
    bool actual_returned;

    GetPointOnLineWithDistParameters(Point2LL p, Point2LL a, Point2LL b, coord_t dist, Point2LL actual_result, bool actual_returned)
        : p(p)
        , a(a)
        , b(b)
        , dist(dist)
        , actual_result(actual_result)
        , actual_returned(actual_returned)
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

/*
 * Fixture to allow parameterized tests for getPointOnLineWithDist.
 */
class GetPointOnLineWithDistTest : public testing::TestWithParam<GetPointOnLineWithDistParameters>
{
    // Just here to allow parameterized tests. No actual fixtures necessary.
};

TEST_P(GetPointOnLineWithDistTest, GetPointOnLineWithDist)
{
    const GetPointOnLineWithDistParameters parameters = GetParam();
    const Point2LL p = parameters.p;
    const Point2LL a = parameters.a;
    const Point2LL b = parameters.b;
    const coord_t dist = parameters.dist;
    const Point2LL actual_result = parameters.actual_result;
    const bool actual_returned = parameters.actual_returned;

    Point2LL supposed_result;
    const bool supposed_returned = LinearAlg2D::getPointOnLineWithDist(p, a, b, dist, supposed_result);
    const coord_t supposed_dist = vSize(supposed_result - p);

    if (actual_returned)
    {
        EXPECT_TRUE(supposed_returned) << "Point " << p << " wasn't projected on (" << a << " - " << b << ") instead of projecting to " << actual_result << ".";
        EXPECT_LT(vSize2(actual_result - supposed_result), 10 * 10)
            << "Point " << p << " was projected on (" << a << " - " << b << ") to " << supposed_result << " instead of " << actual_result << ".";
        EXPECT_LT(std::abs(supposed_dist - dist), 10) << "Projection distance of " << p << " onto (" << a << " - " << b << ") was " << supposed_dist << " instead of " << dist
                                                      << "."; // FIXME: Clean-up message with ftm when CURA-8258 is implemented or when we use C++20
    }
    else
    {
        ASSERT_FALSE(supposed_returned) << "Point " << p << " should not be projected on (" << a << " - " << b << ").";
    }
}

INSTANTIATE_TEST_CASE_P(
    GetPointOnLineWithDistInstantiation,
    GetPointOnLineWithDistTest,
    testing::Values(
        GetPointOnLineWithDistParameters(Point2LL(110, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(70, 0), true),
        GetPointOnLineWithDistParameters(Point2LL(90, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(50, 0), true),
        GetPointOnLineWithDistParameters(Point2LL(10, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(50, 0), true),
        GetPointOnLineWithDistParameters(Point2LL(-10, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(30, 0), true),
        GetPointOnLineWithDistParameters(Point2LL(50, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(10, 0), true),
        GetPointOnLineWithDistParameters(Point2LL(210, 30), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(70, 0), false),
        GetPointOnLineWithDistParameters(Point2LL(110, 130), Point2LL(0, 0), Point2LL(100, 0), 50, Point2LL(70, 0), false)));

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct RotateAroundParameters
{
    Point2LL point;
    Point2LL origin;
    double angle;
    Point2LL actual_result;

    RotateAroundParameters(Point2LL point, Point2LL origin, double angle, Point2LL actual_result)
        : point(point)
        , origin(origin)
        , angle(angle)
        , actual_result(actual_result)
    {
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

/*
 * Fixture to allow parameterized tests for rotateAround.
 */
class RotateAroundTest : public testing::TestWithParam<RotateAroundParameters>
{
    // Just here to allow parameterized tests. No actual fixtures necessary.
};

TEST_P(RotateAroundTest, RotateAround)
{
    const RotateAroundParameters parameters = GetParam();
    const Point2LL point = parameters.point;
    const Point2LL origin = parameters.origin;
    const double angle = parameters.angle;
    const Point2LL actual_result = parameters.actual_result;

    const Point3Matrix mat = LinearAlg2D::rotateAround(origin, angle);
    const Point2LL supposed_result = mat.apply(point);
    ASSERT_LT(vSize(supposed_result - actual_result), 2) << "LinearAlg2D::rotateAround failed: Rotating " << point << " around " << origin << " for " << angle
                                                         << " degrees resulted in " << supposed_result << " instead of expected " << actual_result << ".";
}

INSTANTIATE_TEST_SUITE_P(
    RotateAroundInstantiation,
    RotateAroundTest,
    testing::Values(
        RotateAroundParameters(Point2LL(25, 30), Point2LL(10, 17), 90, Point2LL(-3, 32)), // 90 degrees rotation.
        RotateAroundParameters(Point2LL(25, 30), Point2LL(10, 17), -90, Point2LL(23, 2)), //-90 degrees rotation.
        RotateAroundParameters(Point2LL(-67, 14), Point2LL(50, 50), 0, Point2LL(-67, 14)), // No rotation at all.
        RotateAroundParameters(Point2LL(-67, 14), Point2LL(50, 50), 12, Point2LL(-57, -9)) // 12 degrees rotation. Actually ends up at [-57, -9.5]!
        ));

class Temp
{
};

TEST(Temp, LineDistTests)
{
    std::srand(987);
    for (int z = 0; z < 100; ++z)
    {
        const Point2LL p{ 500000 + (std::rand() % 4000) - 2000, 500000 + (std::rand() % 4000) - 2000 };

        const coord_t d = (std::rand() % 2000) - 1000 / 2;
        const double rang = std::rand() / (static_cast<double>(RAND_MAX) / 6.29);
        const Point2LL x{ p.X + static_cast<coord_t>(d * std::cos(rang)), p.Y - static_cast<coord_t>(d * std::sin(rang)) };

        // Use positive lengths here, so line and line-segment should give the same answers.
        coord_t len = std::rand() % 1000;
        const Point2LL a{ x.X + static_cast<coord_t>(len * std::sin(rang)), x.Y + static_cast<coord_t>(len * std::cos(rang)) };
        len = std::rand() % 1000;
        const Point2LL b{ x.X - static_cast<coord_t>(len * std::sin(rang)), x.Y - static_cast<coord_t>(len * std::cos(rang)) };

        const coord_t abs_d = std::abs(d);
        ASSERT_NEAR(LinearAlg2D::getDistFromLine(p, a, b), abs_d, 5);
        ASSERT_NEAR(vSize(LinearAlg2D::getClosestOnLine(p, a, b) - x), 0, 5);
        ASSERT_NEAR(vSize(LinearAlg2D::getClosestOnLineSegment(p, a, b) - x), 0, 5);
        ASSERT_NEAR(std::sqrt(LinearAlg2D::getDist2FromLine(p, a, b)), abs_d, 5);
        ASSERT_NEAR(std::sqrt(LinearAlg2D::getDist2FromLineSegment(a, p, b)), abs_d, 5);

        ASSERT_NEAR(std::round(std::sqrt(LinearAlg2D::getDist2FromLine(p, a, b))), LinearAlg2D::getDistFromLine(p, a, b), 5);
    }
}

} // namespace cura
// NOLINTEND(*-magic-numbers)
