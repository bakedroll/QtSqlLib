#pragma once

#include <gmock/gmock.h>

#include <QtSqlLib/API/ISchema.h>

using namespace testing;

namespace QtSqlLibTest
{

class MockSchema : public QtSqlLib::API::ISchema
{
public:
  MOCK_METHOD((std::map<QtSqlLib::API::IID::Type, Table>&), getTables, (), (override));
  MOCK_METHOD((std::map<QtSqlLib::API::IID::Type, Relationship>&), getRelationships, (), (override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), getManyToManyLinkTableId, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), configureRelationships, (), (override));
  MOCK_METHOD((void), throwIfTableIdNotExisting, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), throwIfRelationshipIsNotExisting, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), throwIfColumnIdNotExisting, (const Table&, QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), validatePrimaryKeysAndGetTableId, (const TupleValues&), (const override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), validatePrimaryKeysListAndGetTableId, (const std::vector<TupleValues>&), (const override));
  MOCK_METHOD((std::pair<QtSqlLib::API::IID::Type, QtSqlLib::API::IID::Type>), verifyOneToOneRelationshipPrimaryKeysAndGetTableIds,
    (QtSqlLib::API::IID::Type, const TupleValues&, const TupleValues&), (const override));
  MOCK_METHOD((std::pair<QtSqlLib::API::IID::Type, QtSqlLib::API::IID::Type>), verifyOneToManyRelationshipPrimaryKeysAndGetTableIds,
    (QtSqlLib::API::IID::Type, const TupleValues&, const std::vector<TupleValues>&), (const override));

};

}
