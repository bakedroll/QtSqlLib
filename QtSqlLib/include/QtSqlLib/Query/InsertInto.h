#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

#define INSERT_INTO(X) QtSqlLib::Query::InsertInto(QtSqlLib::ID(X))

#define VALUE(X, Y) value(QtSqlLib::ID(X), Y)

namespace QtSqlLib::Query
{

class InsertInto : public BaseInsert
{
public:
  InsertInto(const API::IID& tableId);
  ~InsertInto() override;

  InsertInto& value(const API::IID& columnId, const QVariant& value);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariant> m_values;

};

}
