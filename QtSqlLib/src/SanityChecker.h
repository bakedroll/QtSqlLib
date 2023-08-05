#pragma once

#include <QtSqlLib/API/ISanityChecker.h>

#include <map>

namespace QtSqlLib
{

class SanityChecker : public API::ISanityChecker
{
public:
  SanityChecker(
    std::map<API::IID::Type, API::Table>& tables,
    std::map<API::IID::Type, API::Relationship>& relationships);
  ~SanityChecker() override;

  void throwIfTableIdNotExisting(API::IID::Type tableId) const override;
  void throwIfRelationshipIsNotExisting(API::IID::Type relationshipId) const override;
  void throwIfColumnIdNotExisting(const API::Table& table, API::IID::Type colId) const override;

private:
  std::map<API::IID::Type, API::Table>& m_tablesRef;
  std::map<API::IID::Type, API::Relationship>& m_relationshipsRef;

};

}
