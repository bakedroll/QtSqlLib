#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/BaseInsert.h>

#include <QVariant>

#include <vector>

#define BATCH_INSERT_INTO(X) QtSqlLib::Query::BatchInsertInto(QtSqlLib::ID(X))
#define VALUES(X, Y) values(QtSqlLib::ID(X), Y)

namespace QtSqlLib::Query
{

class BatchInsertInto : public BaseInsert
{
public:
  BatchInsertInto(const API::IID& tableId);
  ~BatchInsertInto() override;

  BatchInsertInto& values(const API::IID& columnId, const QVariantList& values);

  SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;

protected:
  void bindQueryValues(QSqlQuery& query) const override;

private:
  std::vector<QVariantList> m_values;

};

}