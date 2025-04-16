#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/ColumnList.h>
#include <QtSqlLib/PrimaryKey.h>

#include <QPointer>
#include <QSqlResult>
#include <QVariant>

#include <vector>

namespace QtSqlLib
{

class TupleView
{
public:
  explicit TupleView(
    API::IID::Type tableId,
    const std::vector<size_t>& primaryKeyColumnIndices,
    const ColumnList& columns,
    const std::vector<QVariant>& values);

  virtual ~TupleView();

  API::IID::Type tableId() const;
  PrimaryKey primaryKey() const;
  const std::vector<QVariant>& values() const;

  // TODO: use template for IDs
  QVariant columnValue(const API::IID& columnId) const;

private:
  QSqlResult* m_sqlResult;
  const API::QueryMetaInfo& m_queryMetaInfo;
  const std::vector<API::JoinMetaInfo>& m_joinsMetaInfo;
  //API::IID::Type m_tableId;
  //const std::vector<size_t>& m_primaryKeyColumnIndices;
  //const ColumnList& m_columns;
  //const std::vector<QVariant>& m_values;


};

}
