#pragma once

#include <gmock/gmock.h>

#include <QtSqlLib/API/ISanityChecker.h>

using namespace testing;

namespace QtSqlLibTest
{

class MockSanityChecker : public QtSqlLib::API::ISanityChecker
{
public:
  MOCK_METHOD((void), throwIfTableIdNotExisting, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), throwIfRelationshipIsNotExisting, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), throwIfColumnIdNotExisting, (const QtSqlLib::API::Table&, QtSqlLib::API::IID::Type), (const override));

};

}
