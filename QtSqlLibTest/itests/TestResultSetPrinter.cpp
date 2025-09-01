#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

#include <limits>

namespace QtSqlLibTest
{

class TestResultSetPrinter : public testing::Test
{
public:
  TestResultSetPrinter()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
    setupTestDatabase(m_db);
  }

  ~TestResultSetPrinter() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

private:
  static void setupTestDatabase(QtSqlLib::API::IDatabase& m_db);

};

void TestResultSetPrinter::setupTestDatabase(QtSqlLib::API::IDatabase& m_db)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Albums, "albums")
    .COLUMN(AlbumsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(AlbumsCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_TABLE(TableIds::Artists, "artists")
    .COLUMN(ArtistsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(ArtistsCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_TABLE(TableIds::Tracks, "tracks")
    .COLUMN(TracksCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(TracksCols::Name, "name", 128).NOT_NULL
    .COLUMN(TracksCols::Length, "length", DataType::Real).NOT_NULL
    .COLUMN(TracksCols::Rating, "rating", DataType::Integer);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumArtists, TableIds::Albums, TableIds::Artists,
    QtSqlLib::API::RelationshipType::ManyToMany);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumTracks, TableIds::Albums, TableIds::Tracks,
    QtSqlLib::API::RelationshipType::OneToMany);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  // Albums
  const auto album1 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 1")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto album2 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 2")
    .RETURN_IDS).nextTuple().primaryKey();

  // Artists
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Artists)
    .VALUE(ArtistsCols::Name, "Artist 1")
    .LINK_TO_MANY_TUPLES(Relationships::AlbumArtists, { album1, album2 })
    .RETURN_IDS).nextTuple().primaryKey();

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Artists)
    .VALUE(ArtistsCols::Name, "Artist 2")
    .LINK_TO_ONE_TUPLE(Relationships::AlbumArtists, album2)
    .RETURN_IDS).nextTuple().primaryKey();

  // TODO: RETURN_IDS should not be necessary

  // Album 1 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 1)
      .VALUE(TracksCols::Rating, 1.0)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2 with much longer name")
      .VALUE(TracksCols::Length, 2)
      .VALUE(TracksCols::Rating, 2.5)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  // Album 2 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 3)
      .VALUE(TracksCols::Rating, 123456.7890123456789)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 4)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 3")
      .VALUE(TracksCols::Length, std::numeric_limits<qlonglong>::max())
      .VALUE(TracksCols::Rating, 3.5)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));
}

/**
 * @test: Test if the ResultSetPrinter formats the columns correctly.
 * @expected: The columns don't exceed the maximum size, but are at minimum as wide as the containing value text.
 */
TEST_F(TestResultSetPrinter, printedColumnsFormat)
{
  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .JOIN(Relationships::AlbumTracks, TracksCols::Id, TracksCols::Name, TracksCols::Length, TracksCols::Rating));

  auto printer = m_db.createResultSetPrinter(results, 24);
  printf("\n################\n\n");
  while (!printer.isEndOfTable())
  {
    printf("%s\n", printer.nextPrinterLine().toStdString().c_str());
  }
}

/**
 * @test: Test output of query with multiple joins.
 * @expected: Multiple joins are correctly displayed.
 */
TEST_F(TestResultSetPrinter, printMultipleJoins)
{
  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .JOIN(Relationships::AlbumArtists, ArtistsCols::Id, ArtistsCols::Name)
    .JOIN(Relationships::AlbumTracks, TracksCols::Name));

  auto printer = m_db.createResultSetPrinter(results, 24);
  printf("\n################\n\n");
  while (!printer.isEndOfTable())
  {
    printf("%s\n", printer.nextPrinterLine().toStdString().c_str());
  }
}

/**
 * @test: Test output of query with aggregated functions.
 * @expected: Aggregated functions are correctly displayed.
 */
TEST_F(TestResultSetPrinter, printAggregatedFunctions)
{
  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name, COUNT(AlbumsCols::Id))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id)
    .GROUP_BY(AlbumsCols::Id));

  auto printer = m_db.createResultSetPrinter(results, 24);
  printf("\n################\n\n");
  while (!printer.isEndOfTable())
  {
    printf("%s\n", printer.nextPrinterLine().toStdString().c_str());
  }
}

}
