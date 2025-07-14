#include <gtest/gtest.h>

#include <QtSqlLib/ColumnHelper.h>
#include <QtSqlLib/ColumnStatistics.h>
#include <QtSqlLib/ID.h>
#include <QtSqlLib/Macros.h>

namespace QtSqlLibTest
{

enum class EColumns
{
    COL1,
    COL2
};

/**
 * @test: Tests the construction of a SelectColumnList.
 * @expected: The list is correctly constructed.
 */
TEST(TestColumnHelper, createSelectColumn)
{
    const auto list = QtSqlLib::ColumnHelper::make<QtSqlLib::ColumnHelper::SelectColumn>(EColumns::COL1, 5, EColumns::COL2, COUNT(EColumns::COL2));

    EXPECT_EQ(list.size(), 4);

    EXPECT_EQ(list.at(0).columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL1));
    EXPECT_EQ(list.at(1).columnId, 5);
    EXPECT_EQ(list.at(2).columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL2));
    EXPECT_EQ(list.at(3).columnId, COUNT(EColumns::COL2));
}

/**
 * @test: Tests the construction of a GroupColumnList.
 * @expected: The list is correctly constructed.
 */
TEST(TestColumnHelper, createGroupColumnList)
{
    const auto list = QtSqlLib::ColumnHelper::make<QtSqlLib::ColumnHelper::GroupColumn>(EColumns::COL1, 5, EColumns::COL2, COUNT(EColumns::COL2));

    EXPECT_EQ(list.size(), 4);

    EXPECT_EQ(list.at(0).data.columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL1));
    EXPECT_EQ(list.at(1).data.columnId, 5);
    EXPECT_EQ(list.at(2).data.columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL2));
    EXPECT_EQ(list.at(3).data.columnId, COUNT(EColumns::COL2));
}

/**
 * @test: Tests the construction of a OrderColumnList.
 * @expected: The list is correctly constructed.
 */
TEST(TestColumnHelper, createOrderColumnList)
{
    const auto list = QtSqlLib::ColumnHelper::make<QtSqlLib::ColumnHelper::OrderColumn>(EColumns::COL1, 5 DESC, EColumns::COL2, COUNT(EColumns::COL2) DESC);

    EXPECT_EQ(list.size(), 4);

    EXPECT_EQ(list.at(0).data.columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL1));
    EXPECT_EQ(list.at(0).order, QtSqlLib::ColumnHelper::EOrder::Ascending);

    EXPECT_EQ(list.at(1).data.columnId, 5);
    EXPECT_EQ(list.at(1).order, QtSqlLib::ColumnHelper::EOrder::Descending);

    EXPECT_EQ(list.at(2).data.columnId, static_cast<QtSqlLib::API::IID::Type>(EColumns::COL2));
    EXPECT_EQ(list.at(2).order, QtSqlLib::ColumnHelper::EOrder::Ascending);

    EXPECT_EQ(list.at(3).data.columnId, COUNT(EColumns::COL2));
    EXPECT_EQ(list.at(3).order, QtSqlLib::ColumnHelper::EOrder::Descending);

}

}
