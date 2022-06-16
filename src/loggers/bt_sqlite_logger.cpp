#include "behaviortree_cpp_v3/loggers/bt_sqlite_logger.h"

BT::SqliteLogger::SqliteLogger(const BT::Tree &tree,
                               const char *filename,
                               bool append):
  StatusChangeLogger(tree.rootNode()),
  db_(filename, SQLite::OPEN_READWRITE |SQLite::OPEN_CREATE)
{
  enableTransitionToIdle(true);

  db_.exec("CREATE TABLE IF NOT EXISTS Transitions ("
           "timestamp  INTEGER PRIMARY KEY, "
           "uid        INTEGER, "
           "duration   INTEGER, "
           "prev_state TEXT NOT NULL, "
           "state      TEXT NOT NULL)");

  db_.exec("CREATE TABLE IF NOT EXISTS Nodes ("
           "uid        INTEGER PRIMARY KEY UNIQUE, "
           "node_type  TEXT NOT NULL, "
           "node_name  TEXT NOT NULL, "
           "instance   TEXT)");

  db_.exec("PRAGMA journal_mode=WAL");

  if( !append )
  {
    db_.exec("DELETE from Transitions;");
  }
  db_.exec("DELETE from Nodes;");

  for(const auto& node: tree.nodes)
  {
    SQLite::Statement   query(db_, "INSERT INTO Nodes VALUES (?, ?, ?, ?)");
    query.bind(1, node->UID());
    query.bind(2, toStr(node->type()));
    query.bind(3, node->registrationName());
    query.bind(4, node->name());
    query.exec();
  }

}

BT::SqliteLogger::~SqliteLogger()
{
  flush();
}

void BT::SqliteLogger::callback(BT::Duration timestamp,
                                const BT::TreeNode &node,
                                BT::NodeStatus prev_status,
                                BT::NodeStatus status)
{

  using namespace std::chrono;
  auto tm_usec = duration_cast<microseconds>(timestamp).count();
  monotonic_timestamp_ = std::max( monotonic_timestamp_ + 1, tm_usec);

  long elapsed_time = 0;

  if( prev_status == NodeStatus::IDLE && status == NodeStatus::RUNNING )
  {
    starting_time_[&node] = monotonic_timestamp_;
  }

  if( prev_status == NodeStatus::RUNNING && status != NodeStatus::RUNNING )
  {
    elapsed_time = monotonic_timestamp_;
    auto it = starting_time_.find(&node);
    if( it != starting_time_.end() )
    {
      elapsed_time -= it->second;
    }
  }

  auto ToChar = [](NodeStatus stat) ->const char *
  {
    if( stat == NodeStatus::RUNNING ) return "R";
    if( stat == NodeStatus::SUCCESS ) return "S";
    if( stat == NodeStatus::FAILURE ) return "F";
    return "I";
  };

  SQLite::Statement   query(db_, "INSERT INTO Transitions VALUES (?, ?, ?, ?, ?)");
  query.bind(1, monotonic_timestamp_);
  query.bind(2, node.UID());
  query.bind(3, elapsed_time);
  query.bind(4, ToChar(prev_status));
  query.bind(5, ToChar(status));

  query.exec();

}

void BT::SqliteLogger::flush()
{
  db_.flush();
}
