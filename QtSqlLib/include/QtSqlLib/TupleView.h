#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/ColumnList.h>
#include <QtSqlLib/PrimaryKey.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class TupleView
{
public:
  struct ForeignKeyRef
  {
    API::IID::Type relationshipId;
    PrimaryKey key;
  };

  explicit TupleView(
    const PrimaryKey& primaryKey,
    const ColumnList& columns,
    const std::vector<QVariant>& values,
    const std::vector<ForeignKeyRef>& foreignKeyRefs);

  virtual ~TupleView();

  PrimaryKey primaryKey() const;
  const std::vector<QVariant>& values() const;
  QVariant columnValue(const API::IID& columnId) const;
  PrimaryKey foreignKey(const API::IID& relationshipId) const;

private:
  const PrimaryKey& m_primaryKey;
  const ColumnList& m_columns;
  const std::vector<QVariant>& m_values;
  const std::vector<ForeignKeyRef>& m_foreignKeyRefs;

};

}
