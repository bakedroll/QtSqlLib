#include "QtSqlLib/ResultSet.h"

namespace QtSqlLib
{

ResultSet::ResultSet(const QSqlQuery& query) :
  m_sqlQuery(query)
{
}

ResultSet::~ResultSet() = default;

}
