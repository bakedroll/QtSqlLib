#pragma once

#include <gmock/gmock.h>

#include <QtSqlLib/API/IQueryElement.h>

using namespace testing;

namespace QtSqlLibTest
{

class MockQueryElement : public QtSqlLib::API::IQueryElement
{
public:
  MOCK_METHOD(void, accept, (QtSqlLib::API::IQueryVisitor&), (override));
};

}
