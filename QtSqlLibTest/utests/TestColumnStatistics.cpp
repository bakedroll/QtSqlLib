#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

/**
 * @test: Tests the encoding and decoding of aggregate function min in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionMinEncoding)
{
    const IID::Type columnId = 42;

    const auto statistics = ColumnStatistics::min(columnId);
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Min);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::All);
    EXPECT_EQ(statistics.columnId(), columnId);

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Min);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::All);
    EXPECT_EQ(statisticsReencoded.columnId(), columnId);
}

/**
 * @test: Tests the encoding and decoding of aggregate function max in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionMaxEncoding)
{
    const IID::Type columnId = 42;

    const auto statistics = ColumnStatistics::max(columnId);
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Max);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::All);
    EXPECT_EQ(statistics.columnId(), columnId);

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Max);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::All);
    EXPECT_EQ(statisticsReencoded.columnId(), columnId);
}

/**
 * @test: Tests the encoding and decoding of aggregate function sum in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionSumEncoding)
{
    const IID::Type columnId = 42;

    const auto statistics = ColumnStatistics::sum(columnId, ColumnStatistics::EMethod::Distict);
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Sum);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_EQ(statistics.columnId(), columnId);

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Sum);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_EQ(statisticsReencoded.columnId(), columnId);
}

/**
 * @test: Tests the encoding and decoding of aggregate function count in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionCountEncoding)
{
    const IID::Type columnId = 42;

    const auto statistics = ColumnStatistics::count(columnId, ColumnStatistics::EMethod::Distict);
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Count);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_TRUE(statistics.hasColumn());
    EXPECT_EQ(statistics.columnId(), columnId);

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Count);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_TRUE(statistics.hasColumn());
    EXPECT_EQ(statisticsReencoded.columnId(), columnId);
}

/**
 * @test: Tests the encoding and decoding of aggregate function count in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionCountWithoutColumnEncoding)
{
    const auto statistics = ColumnStatistics::count();
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Count);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::All);
    EXPECT_FALSE(statistics.hasColumn());

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Count);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::All);
    EXPECT_FALSE(statistics.hasColumn());
}

/**
 * @test: Tests the encoding and decoding of aggregate function avg in ColumnStatistics.
 * @expected: Expects same content after reencoding.
 */
TEST(TestColumnStatistics, aggregateFunctionAvgEncoding)
{
    const IID::Type columnId = 42;

    const auto statistics = ColumnStatistics::avg(columnId, ColumnStatistics::EMethod::Distict);
    const auto statisticsId = statistics.id();

    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(statisticsId));
    EXPECT_EQ(statistics.type(), ColumnStatistics::EType::Avg);
    EXPECT_EQ(statistics.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_TRUE(statistics.hasColumn());
    EXPECT_EQ(statistics.columnId(), columnId);

    const auto statisticsReencoded = ColumnStatistics::fromId(statisticsId);

    EXPECT_EQ(statisticsReencoded.type(), ColumnStatistics::EType::Avg);
    EXPECT_EQ(statisticsReencoded.method(), ColumnStatistics::EMethod::Distict);
    EXPECT_TRUE(statistics.hasColumn());
    EXPECT_EQ(statisticsReencoded.columnId(), columnId);
}

/**
 * @test: Tests the distinction between ColumnStatistics ids and conventional column ids
 * @expected: Expects that conventional column ids have a value range of 27 bits.
 */
TEST(TestColumnStatistics, isColumnStatisticsDistinction)
{
    EXPECT_FALSE(ColumnStatistics::isColumnStatistics(42));
    EXPECT_FALSE(ColumnStatistics::isColumnStatistics(0x7FFFFFF));
    EXPECT_TRUE(ColumnStatistics::isColumnStatistics(0xFFFFFFF));
}

}
