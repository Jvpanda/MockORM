#include <AbsDatabase.h>
#include <Queries.h>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

namespace Query
{

bool insertRow(const std::string &dbName, const std::string &table,
               const std::vector<std::string> &columns,
               const std::vector<SQLiteValue> &values)
{
        if (columns.size() != values.size())
                return false;

        // Build SQL
        std::stringstream ss;
        ss << "INSERT INTO " << table << " (";

        for (size_t i = 0; i < columns.size(); ++i)
        {
                ss << columns[i];
                if (i < columns.size() - 1)
                        ss << ", ";
        }

        ss << ") VALUES (";

        for (size_t i = 0; i < values.size(); ++i)
        {
                ss << "?";
                if (i < values.size() - 1)
                        ss << ", ";
        }

        ss << ");";

        std::string sql = ss.str();

        ABSDatabase db(dbName);
        Statement st(db.get(), sql);
        for (int i = 0; i < values.size(); ++i)
        {
                st.bind(i + 1, values[i]);
        }
        st.execute();
        return true;
}

bool deleteRow(const std::string &dbName, const std::string &table,
               const std::vector<std::string> &whereColumns,
               const std::vector<SQLiteValue> &whereValues)
{
        if (whereColumns.size() != whereValues.size())
                return false;

        std::stringstream ss;
        ss << "DELETE FROM " << table;

        if (!whereColumns.empty())
        {
                ss << " WHERE ";
                for (size_t i = 0; i < whereColumns.size(); ++i)
                {
                        ss << whereColumns[i] << " = ?";
                        if (i < whereColumns.size() - 1)
                                ss << " AND ";
                }
        }

        ss << ";";
        std::string sql = ss.str();

        ABSDatabase db(dbName);
        Statement st(db.get(), sql);

        for (int i = 0; i < whereValues.size(); ++i)
        {
                st.bind(i + 1, whereValues[i]);
        }

        st.execute();

        return true;
}

bool updateRows(const std::string &dbName, const std::string &table,
                const std::vector<std::string> &whereColumns,
                const std::vector<SQLiteValue> &whereValues,
                const std::vector<std::string> &updateColumns,
                const std::vector<SQLiteValue> &updateValues)
{
        if (updateColumns.size() != updateValues.size())
                return false;

        if (whereColumns.size() != whereValues.size())
                return false;

        std::stringstream ss;
        ss << "UPDATE " << table << " SET ";

        if (!updateColumns.empty())
        {

                for (size_t i = 0; i < updateColumns.size(); ++i)
                {
                        ss << updateColumns[i] << " = ?";

                        if (i < updateColumns.size() - 1)
                                ss << ", ";
                }
        }

        if (!whereColumns.empty())
        {
                ss << " WHERE ";
                for (size_t i = 0; i < whereColumns.size(); ++i)
                {
                        ss << whereColumns[i] << " = ?";
                        if (i < whereColumns.size() - 1)
                                ss << " AND ";
                }
        }

        ss << ";";
        std::string sql = ss.str();

        ABSDatabase db(dbName);
        Statement st(db.get(), sql);

        int bindIndex = 1;

        // Bind SET values first
        for (int i = 0; i < updateValues.size(); ++i)
        {
                st.bind(bindIndex, updateValues[i]);
                bindIndex++;
        }

        // Bind SET values first
        for (int i = 0; i < whereValues.size(); ++i)
        {
                st.bind(bindIndex, whereValues[i]);
                bindIndex++;
        }

        st.execute();
        return true;
}
bool doMathUpdate(const std::string &dbName, const std::string &table,
                  const std::vector<std::string> &whereColumns,
                  const std::vector<SQLiteValue> &whereValues,
                  const std::string &updateColumn, const std::string &mathValue)
{
        if (updateColumn == "" || mathValue == "")
                return false;

        if (whereColumns.size() != whereValues.size())
                return false;

        std::stringstream ss;
        ss << "UPDATE " << table << " SET ";

        if (mathValue != "")
                ss << updateColumn << " = " << updateColumn << " " << mathValue;

        if (!whereColumns.empty())
        {
                ss << " WHERE ";
                for (size_t i = 0; i < whereColumns.size(); ++i)
                {
                        ss << whereColumns[i] << " = ?";
                        if (i < whereColumns.size() - 1)
                                ss << " AND ";
                }
        }

        ss << ";";
        std::string sql = ss.str();

        ABSDatabase db(dbName);
        Statement st(db.get(), sql);
        // Bind SET values first
        for (int i = 0; i < whereValues.size(); ++i)
        {
                st.bind(i + 1, whereValues[i]);
        }

        st.execute();
        return true;
}

void selectHelper(Table &results, Statement &st)
{
        while (st.step())
        {
                Row row;
                int colCount = sqlite3_column_count(st.getStatement());

                for (int i = 0; i < colCount; ++i)
                {
                        std::string name =
                            sqlite3_column_name(st.getStatement(), i);
                        int type = sqlite3_column_type(st.getStatement(), i);

                        switch (type)
                        {
                                case SQLITE_INTEGER:
                                        row[name] = sqlite3_column_int(
                                            st.getStatement(), i);
                                        break;

                                case SQLITE_FLOAT:
                                        row[name] = sqlite3_column_double(
                                            st.getStatement(), i);
                                        break;

                                case SQLITE_TEXT:
                                        row[name] = std::string(
                                            reinterpret_cast<const char *>(
                                                sqlite3_column_text(
                                                    st.getStatement(), i)));
                                        break;

                                case SQLITE_NULL:
                                        row[name] = nullptr;
                                        break;
                        }
                }

                results.push_back(row);
        }
}

Table selectRows(const std::string &dbName, const std::string &table,
                 const std::vector<std::string> &columns,
                 const std::vector<std::string> &whereColumns,
                 const std::vector<SQLiteValue> &whereValues)
{
        Table results;

        if (whereColumns.size() != whereValues.size())
                return results;

        std::stringstream ss;

        ss << "SELECT ";

        if (columns.empty())
                ss << "*";
        else
        {
                for (size_t i = 0; i < columns.size(); ++i)
                {
                        ss << columns[i];
                        if (i < columns.size() - 1)
                                ss << ", ";
                }
        }

        ss << " FROM " << table;

        if (!whereColumns.empty())
        {
                ss << " WHERE ";
                for (size_t i = 0; i < whereColumns.size(); ++i)
                {
                        ss << whereColumns[i] << " = ?";
                        if (i < whereColumns.size() - 1)
                                ss << " AND ";
                }
        }

        ss << ";";

        std::string sql = ss.str();

        ABSDatabase db(dbName);
        Statement st(db.get(), sql);

        // Bind WHERE values
        for (size_t i = 0; i < whereValues.size(); ++i)
                st.bind(i + 1, whereValues[i]);

        selectHelper(results, st);

        return results;
}

Table selectRowsWithQuery(const std::string &dbName, const std::string &query)
{

        Table results;
        ABSDatabase db(dbName);
        Statement st(db.get(), query);
        selectHelper(results, st);

        return results;
}

bool customExecuteQuery(const std::string &dbName, const std::string &query)
{
        ABSDatabase db(dbName);
        Statement st(db.get(), query);
        st.execute();
        return true;
}

bool mergeUniqueDatabases(const std::string &mainDBName,
                          const std::string &otherFile,
                          const std::string &mergeIntoTable,
                          const std::string &mergeFromTable,
                          const std::string &uniqueIntoColumn,
                          const std::string &uniqueFromColumn)
{
        ABSDatabase mainDB(mainDBName);
        sqlite3 *db = mainDB.get();

        try
        {
                // Begin transaction
                {
                        Statement begin(db, "BEGIN TRANSACTION;");
                        begin.execute();
                }

                // Attach other database
                {
                        Statement attach(db, "ATTACH DATABASE ? AS other;");
                        attach.bind(1, otherFile);
                        attach.execute();
                }

                {
                        std::stringstream ss;

                        ss << "INSERT OR IGNORE INTO main." << mergeIntoTable
                           << " (" << uniqueIntoColumn << ")" << " SELECT "
                           << uniqueFromColumn << " FROM other."
                           << mergeFromTable << ";";

                        std::string sql = ss.str();

                        Statement merge(db, sql);
                        merge.execute();
                }

                // Commit
                {
                        Statement commit(db, "COMMIT;");
                        commit.execute();
                }

                // Detach
                {
                        Statement detach(db, "DETACH DATABASE other;");
                        detach.execute();
                }
        }
        catch (std::runtime_error e)
        {
                Statement rollback(db, "ROLLBACK;");
                rollback.execute();
                std::cerr << e.what() << endl;
                throw std::runtime_error("Could not merge databases");
        }
        return true;
}

bool mergeDatabases(const std::string &mainDBName, const std::string &otherFile,
                    const std::string &mergeIntoTable,
                    const std::string &mergeFromTable,
                    const std::vector<std::string> &mergeIntoColumns,
                    const std::vector<std::string> &mergeFromColumns)
{
        if (mergeFromColumns.size() != mergeIntoColumns.size())
                throw std::runtime_error("Code column sizes do not match");

        ABSDatabase mainDB(mainDBName);
        sqlite3 *db = mainDB.get();

        try
        {
                // Begin transaction
                {
                        Statement begin(db, "BEGIN TRANSACTION;");
                        begin.execute();
                }

                // Attach other database
                {
                        Statement attach(db, "ATTACH DATABASE ? AS other;");
                        attach.bind(1, otherFile);
                        attach.execute();
                }

                {
                        std::stringstream ss;

                        ss << "INSERT INTO main." << mergeIntoTable;

                        if (!mergeIntoColumns.empty())
                        {
                                ss << " (";
                                for (size_t i = 0; i < mergeIntoColumns.size();
                                     ++i)
                                {
                                        ss << mergeIntoColumns[i];

                                        if (i < mergeIntoColumns.size() - 1)
                                                ss << ", ";
                                }
                                ss << ")";
                        }

                        ss << " SELECT ";

                        if (!mergeFromColumns.empty())
                        {
                                for (size_t i = 0; i < mergeFromColumns.size();
                                     ++i)
                                {
                                        ss << mergeFromColumns[i];

                                        if (i < mergeFromColumns.size() - 1)
                                                ss << ", ";
                                }
                        }
                        else
                                ss << "*";

                        ss << " FROM other." << mergeFromTable;

                        ss << ";";

                        std::string sql = ss.str();

                        Statement merge(db, sql);
                        merge.execute();
                }

                // Commit
                {
                        Statement commit(db, "COMMIT;");
                        commit.execute();
                }

                // Detach
                {
                        Statement detach(db, "DETACH DATABASE other;");
                        detach.execute();
                }
        }
        catch (std::runtime_error e)
        {
                Statement rollback(db, "ROLLBACK;");
                rollback.execute();
                std::cerr << e.what() << endl;
                throw std::runtime_error("Could not merge databases");
        }
        return true;
}

} // namespace Query
