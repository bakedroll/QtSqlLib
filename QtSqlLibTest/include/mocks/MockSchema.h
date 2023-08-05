#pragma once

#include <gmock/gmock.h>

#include <QtSqlLib/API/ISchema.h>

using namespace testing;

namespace QtSqlLibTest
{

class MockSchema : public QtSqlLib::API::ISchema
{
public:
  MOCK_METHOD((std::map<QtSqlLib::API::IID::Type, QtSqlLib::API::Table>&), getTables, (), (override));
  MOCK_METHOD((std::map<QtSqlLib::API::IID::Type, QtSqlLib::API::Relationship>&), getRelationships, (), (override));
  MOCK_METHOD((std::vector<QtSqlLib::API::Index>&), getIndices, (), (override));
  MOCK_METHOD((const QtSqlLib::API::ISanityChecker&), getSanityChecker, (), (const override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), getManyToManyLinkTableId, (QtSqlLib::API::IID::Type), (const override));
  MOCK_METHOD((void), configureRelationships, (), (override));
  MOCK_METHOD((void), validateAndPrepareIndices, (), (override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), validatePrimaryKeysAndGetTableId, (const QtSqlLib::API::TupleValues&), (const override));
  MOCK_METHOD((QtSqlLib::API::IID::Type), validatePrimaryKeysListAndGetTableId, (const std::vector<QtSqlLib::API::TupleValues>&), (const override));
  MOCK_METHOD((std::pair<QtSqlLib::API::IID::Type, QtSqlLib::API::IID::Type>), verifyOneToOneRelationshipPrimaryKeysAndGetTableIds,
    (QtSqlLib::API::IID::Type, const QtSqlLib::API::TupleValues&, const QtSqlLib::API::TupleValues&), (const override));
  MOCK_METHOD((std::pair<QtSqlLib::API::IID::Type, QtSqlLib::API::IID::Type>), verifyOneToManyRelationshipPrimaryKeysAndGetTableIds,
    (QtSqlLib::API::IID::Type, const QtSqlLib::API::TupleValues&, const std::vector<QtSqlLib::API::TupleValues>&), (const override));

};

}
