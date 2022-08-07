#include <gtest/gtest.h>

#include <Common.h>

namespace QtSqlLibTest
{

class DummyQuery : public QtSqlLib::Query::Query
{
public:
  DummyQuery(int num)
    : Query()
    , m_num(num)
  {
  }

  ~DummyQuery() override = default;

  int getNum() const
  {
    return m_num;
  }

  SqlQuery getSqlQuery(const QSqlDatabase& db, QtSqlLib::Schema& schema, QueryResults& previousQueryResults) override
  {
    return { QSqlQuery(db), QueryMode::Single };
  }

private:
  int m_num;
};

class TestVisitor : public IQueryVisitor
{
public:
  TestVisitor() : IQueryVisitor(), m_currentNum(0)
  {}

  ~TestVisitor() override = default;

  void visit(IQuery& query) override
  {
    EXPECT_EQ(m_currentNum, static_cast<DummyQuery&>(query).getNum());
    m_currentNum++;
  }

  void visit(IQuerySequence& query) override
  {
  }

private:
  int m_currentNum;

};

/**
 * @test: Checks the order in which the visitor traverses a nested QuerySequence.
 * @expected: A depth-first traversal.
 */
TEST(VisitorTest, nestedQueriesExecutionOrder)
{
  auto nestedSeq2 = std::make_unique<QuerySequence>();
  nestedSeq2->addQuery(std::make_unique<DummyQuery>(3));
  nestedSeq2->addQuery(std::make_unique<DummyQuery>(4));

  auto nestedSeq1 = std::make_unique<QuerySequence>();
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(1));
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(2));
  nestedSeq1->addQuery(std::move(nestedSeq2));
  nestedSeq1->addQuery(std::make_unique<DummyQuery>(5));

  QuerySequence seq;
  seq.addQuery(std::make_unique<DummyQuery>(0));
  seq.addQuery(std::move(nestedSeq1));

  seq.addQuery(std::make_unique<DummyQuery>(6));

  TestVisitor visitor;
  seq.accept(visitor);
}

}
