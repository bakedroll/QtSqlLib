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
    const QString& tableName = "") override;

  QString resolveColumnIdentifier(API::ISchema& schema, const ColumnHelper::ColumnData& columnData) const override;

private:
  struct TableIdentifier
  {
    std::optional<API::IID::Type> relationshipId;
    API::IID::Type tableId;
    QString tableName;
  };

  std::vector<TableIdentifier> m_tableIdentifiers;

};

}
