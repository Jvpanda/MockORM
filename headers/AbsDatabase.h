/**
 * @file AbsDatabase.h
 * @brief Creates and manages databases for SQL Queries automatically
 * Generally fits the RAII Standard
 */

#ifndef ABS_DATABASE_H
#define ABS_DATABASE_H
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <variant>

#ifndef DATABASE_ROOT
#define DATABASE_ROOT ""
#endif

/**
 * @class ABSDatabase
 * @brief A wrapper for the SQLite database connection handle.
 * Handles the opening and closing of the database connection using RAII.
 */
class ABSDatabase
{
        public:
                /** @brief Default constructor. Initializes an empty database
                 * handle. */
                ABSDatabase()
                {
                }

                /** * @brief Overloaded constructor that immediately connects to
                 * a database file.
                 * @param file Path to the SQLite database file.
                 */
                ABSDatabase(const std::string &file)
                {
                        ConnectToDB(DATABASE_ROOT + file);
                }

                /** @brief Destructor. Ensures the database connection is safely
                 * closed.
                 */
                ~ABSDatabase()
                {
                        CloseDB();
                }

                /** @brief Opens a connection to the specified SQLite database
                 * file. */
                void ConnectToDB(const std::string &file);

                /** @brief Closes the current SQLite database connection and
                 * releases resources. */
                void CloseDB();

                /** @return The raw sqlite3 pointer. */
                sqlite3 *get() const;

                /** @param aG Pointer to an existing sqlite3 connection to
                 * manage. */
                void set(sqlite3 *aG)
                {
                        generic_db = aG;
                }

        private:
                sqlite3 *generic_db{
                    nullptr}; ///< The underlying SQLite database handle.
};

/**
 * @class Statement
 * @brief Manages the lifecycle of a prepared SQLite SQL statement.
 * This class handles parameter binding, execution, and column data retrieval.
 */
class Statement
{
        private:
                sqlite3_stmt *stmt{nullptr}; ///< The prepared statement handle.
                sqlite3
                    *absDB; ///< Reference to the parent database connection.

        public:
                /** @return The raw sqlite3_stmt pointer. */
                sqlite3_stmt *getStatement()
                {
                        return stmt;
                }

                /**
                 * @brief Constructs and prepares a new SQL statement.
                 * @param db Pointer to the active sqlite3 database connection.
                 * @param sql The SQL query string to prepare.
                 */
                Statement(sqlite3 *db, const std::string &sql)
                {
                        absDB = db;
                        createStatement(db, sql);
                }

                /**
                 * @brief Compiles the SQL text into a prepared statement.
                 * @throw std::runtime_error If the SQL fails to compile.
                 */
                void createStatement(sqlite3 *db, const std::string &sql)
                {
                        int result = sqlite3_prepare_v2(db, sql.c_str(), -1,
                                                        &stmt, nullptr);
                        if (result != SQLITE_OK)
                        {
                                throw std::runtime_error(sqlite3_errmsg(db));
                        }
                }

                /** @brief Destructor. Finalizes the prepared statement to
                 * prevent memory leaks. */
                ~Statement()
                {
                        sqlite3_finalize(stmt);
                }

                /** @name Parameter Binding
                 * Methods to bind values to '?' or named parameters in the SQL
                 * query.
                 * @{ */

                void bind(int index, int value)
                {
                        sqlite3_bind_int(stmt, index, value);
                }

                void bind(int index, double value)
                {
                        sqlite3_bind_double(stmt, index, value);
                }

                void bind(int index, const std::string &value)
                {
                        sqlite3_bind_text(stmt, index, value.c_str(), -1,
                                          SQLITE_TRANSIENT);
                }

                void bind(int index) ///< Binds a NULL value to the parameter at
                                     ///< index.
                {
                        sqlite3_bind_null(stmt, index);
                }

                /** * @brief Binds a variant to the parameter at
                 * index. Automatically determines the correct SQLite bind
                 * function to call.
                 */
                void bind(int index,
                          std::variant<int, double, std::string, std::nullptr_t>
                              value)
                {
                        std::visit(
                            [this, index](auto &&arg)
                            {
                                    using T = std::decay_t<decltype(arg)>;
                                    if constexpr (std::is_same_v<
                                                      T, std::nullptr_t>)
                                    {
                                            this->bind(
                                                index); // Call the version with
                                                        // no value
                                    }
                                    else
                                    {
                                            this->bind(
                                                index,
                                                arg); // Call the version for
                                                      // int, double, or string
                                    }
                            },
                            value);
                }
                /** @} */

                /**
                 * @brief Executes the statement and checks if a row of data is
                 * returned.
                 * @return true if a row is available (SQLITE_ROW), false
                 * otherwise.
                 */
                bool step()
                {
                        return sqlite3_step(stmt) == SQLITE_ROW;
                }

                /**
                 * @brief Executes a statement that does not return data (e.g.,
                 * INSERT, UPDATE).
                 * @throw std::runtime_error If execution does not result in
                 * SQLITE_DONE.
                 */
                void execute()
                {
                        if (sqlite3_step(stmt) != SQLITE_DONE)
                        {
                                throw std::runtime_error(sqlite3_errmsg(absDB));
                        }
                }

                /** @name Data Retrieval
                 * Methods to extract values from the current row after calling
                 * step().
                 * @{ */
                int getInt(int col)
                {
                        return sqlite3_column_int(stmt, col);
                }

                std::string getText(int col)
                {
                        return reinterpret_cast<const char *>(
                            sqlite3_column_text(stmt, col));
                }
                /** @} */
};
#endif
