#include <gtest/gtest.h>

#include <Common.h>

#include <QtSqlLib/QueryPrepareVisitor.h>

#include <mocks/MockQueryElement.h>
#include <mocks/MockSchema.h>

namespace QtSqlLibTest
{

/**
 * @test: Checks the order in which the visitor traverses a nested QuerySequence.
 * @expected: A depth-first traversal.
 */
TEST(TestVisitor, nestedQueriesExecutionOrder)
{
  NiceMock<MockSchema> schema;
  QtSqlLib::QueryPrepareVisitor visitor(schema);

  auto pMockQueryElement1 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement2 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement3 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement4 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement5 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement6 = std::make_unique<StrictMock<MockQueryElement>>();
  auto pMockQueryElement7 = std::make_unique<StrictMock<MockQueryElement>>();

  InSequence s;
  EXPECT_CALL(*pMockQueryElement6, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement3, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement4, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement1, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement2, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement5, accept(Ref(visitor)));
  EXPECT_CALL(*pMockQueryElement7, accept(Ref(visitor)));

  auto nestedSeq2 = std::make_unique<QuerySequence>();
  nestedSeq2->addQuery(std::move(pMockQueryElement1));
  nestedSeq2->addQuery(std::move(pMockQueryElement2));

  auto nestedSeq1 = std::make_unique<QuerySequence>();
  nestedSeq1->addQuery(std::move(pMockQueryElement3));
  nestedSeq1->addQuery(std::move(pMockQueryElement4));
  nestedSeq1->addQuery(std::move(nestedSeq2));
  nestedSeq1->addQuery(std::move(pMockQueryElement5));

  QuerySequence seq;
  seq.addQuery(std::move(pMockQueryElement6));
  seq.addQuery(std::move(nestedSeq1));
  seq.addQuery(std::move(pMockQueryElement7));

  seq.accept(visitor);
}

}
