#pragma once

#include <QtSqlLib/API/IQueryIdentifiers.h>

#include <vector>

namespace QtSqlLib
{

class QueryIdentifiers : public API::IQueryIdentifiers
{
public:
  QueryIdentifiers();
  ~QueryIdentifiers() override;

  void addTableIdentifier(
    const std::optional<API::IID::Type>& relationshipId,
    API::IID::Type tableId,
    const std::optional<QString>& tableAlias = std::nullopt) override;

  QString resolveColumnIdentifier(API::ISchema& schema, const ColumnHelper::ColumnData& columnData) const override;
  QString resolveTableIdentifier(API::ISchema& schema, const std::optional<API::IID::Type>& relationshipId = std::nullopt) const override;
  API::IID::Type tableId(const std::optional<API::IID::Type>& relationshipId = std::nullopt) const override;

private:
  struct TableIdentifier
  {
    std::optional<API::IID::Type> relationshipId;
    API::IID::Type tableId;
    std::optional<QString> tableAlias;
  };

  std::vector<TableIdentifier> m_tableIdentifiers;

};

}
