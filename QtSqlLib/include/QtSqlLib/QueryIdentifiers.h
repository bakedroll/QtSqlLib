#pragma once

#include <QtSqlLib/API/IID.h>

#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ColumnHelper.h>

#include <QString>

#include <optional>
#include <vector>

namespace QtSqlLib
{

class QueryIdentifiers
{
public:
  QueryIdentifiers();
  virtual ~QueryIdentifiers();

  void addTableIdentifier(
    const std::optional<API::IID::Type>& relationshipId,
    API::IID::Type tableId,
    const QString& tableName = "");

  QString resolveColumnIdentifier(API::ISchema& schema, const ColumnHelper::ColumnData& columnData) const;

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
