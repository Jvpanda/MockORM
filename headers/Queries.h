/**
 * @file Queries.h
 * @brief Database query utilities for SQLite operations
 *
 * This file provides a namespace of functions for interacting with
 * SQLite databases used by the College Tour application. Supports
 * CRUD operations with type-safe parameter binding.
 */

#ifndef QUERIES_H
#define QUERIES_H
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @namespace QueryData
 * @brief High-level wrapper for SQLite CRUD operations.
 * * Provides a simplified interface for common database interactions,
 * automatically handling statement preparation, parameter binding, and result
 * set mapping.
 * ---------------------------------------------------------------------------------
 * Quick Explanation of general use cases in parameters:
 * ("Use this database path", "with this table", {"Where", "these", "columns"},
 * {"Are", "equal to", "These value"};
 * @warning Please sanitize any and all inputs. This implementation does NOT do
 * that for you
 * ---------------------------------------------------------------------------------
 */
namespace Query
{

/**
 * @typedef SQLiteValue
 * @brief A variant representing data types storable in a SQLite cell.
 */
using SQLiteValue = std::variant<int, double, std::string, std::nullptr_t>;

/**
 * @typedef Row
 * @brief Represents a single database row as a map of Column Names to Values.
 * Metaprogramming is used here for ease of use
 */
class Row
{
                std::unordered_map<
                    std::string,
                    std::variant<int, double, std::string, std::nullptr_t>>
                    data;

        public:
                template <typename V> struct Proxy
                {
                                V &cell;

                                // Implicit Conversions (Read-only)
                                operator int() const
                                {
                                        return std::get<int>(cell);
                                }
                                operator double() const
                                {
                                        return std::get<double>(cell);
                                }
                                operator std::string() const
                                {
                                        return std::get<std::string>(cell);
                                }

                                // Assignment: Only available if V is not const
                                template <typename T> Proxy &operator=(T &&val)
                                {
                                        static_assert(
                                            !std::is_const_v<V>,
                                            "Cannot assign to a const Row!");
                                        cell = std::forward<T>(val);
                                        return *this;
                                }

                                // Support for ostream (std::cout << row["key"])
                                friend std::ostream &
                                operator<<(std::ostream &os, const Proxy &p)
                                {
                                        std::visit(
                                            [&os](auto &&arg)
                                            {
                                                    using T = std::decay_t<
                                                        decltype(arg)>;
                                                    if constexpr (
                                                        std::is_same_v<
                                                            T, std::nullptr_t>)
                                                            os;
                                                    else
                                                            os << arg;
                                            },
                                            p.cell);
                                        return os;
                                }
                };

                // Non-const overload
                Proxy<std::variant<int, double, std::string, std::nullptr_t>>
                operator[](std::string index)
                {
                        return {data[index]};
                }

                // Const overload
                Proxy<const std::variant<int, double, std::string,
                                         std::nullptr_t>>
                operator[](std::string index) const
                {
                        return {data.at(index)};
                }
};

/**
 * @typedef Table
 * @brief A collection of rows returned by a SELECT query.
 */
using Table = std::vector<Row>;

/**
 * @brief Inserts a new row into a database table
 *
 * @param dbName Name/path of the database file
 * @param table Name of the table to insert into
 * @param columns Vector of column names to populate
 * @param values Vector of values corresponding to columns
 * @return true if insertion successful, false otherwise
 *
 * @pre columns.size() == values.size()
 *
 * @code
 * QueryData::insertRow("souvenirs.db", "souvenirs",
 *                      {"college", "item", "price"},
 *                      {"UCLA", "T-Shirt", 25.99});
 * @endcode
 *
 * Time Complexity: O(1) for single row insertion
 */
bool insertRow(const std::string &dbName, const std::string &table,
               const std::vector<std::string> &columns,
               const std::vector<SQLiteValue> &values);

/**
 * @brief Deletes rows from a database table matching criteria
 *
 * @param dbName Name/path of the database file
 * @param table Name of the table to delete from
 * @param whereColumns Columns to match for deletion
 * @param whereValues Values to match in those columns
 * @return true if deletion successful, false otherwise
 *
 * @pre whereColumns.size() == whereValues.size()
 *
 * @code
 * QueryData::deleteRow("souvenirs.db", "souvenirs",
 *                      {"college", "item"},
 *                      {"UCLA", "T-Shirt"});
 * @endcode
 *
 * Time Complexity: O(n) where n is number of rows scanned

 */
bool deleteRow(const std::string &dbName, const std::string &table,
               const std::vector<std::string> &whereColumns,
               const std::vector<SQLiteValue> &whereValues);

/**
 * @brief Updates rows in a database table
 *
 * @param dbName Name/path of the database file
 * @param table Name of the table to update
 * @param whereColumns Columns to match for selection
 * @param whereValues Values to match in those columns
 * @param updateColumns Columns to update
 * @param updateValues New values for those columns
 * @return true if update successful, false otherwise
 *
 * @code
 * // Direct update
 * QueryData::updateRows("souvenirs.db", "souvenirs",
 *                       {"college", "item"}, {"UCLA", "T-Shirt"},
 *                       {"price"}, {29.99});
 * @endcode
 *
 * Time Complexity: O(n) where n is number of rows scanned
 */
bool updateRows(const std::string &dbName, const std::string &table,
                const std::vector<std::string> &whereColumns,
                const std::vector<SQLiteValue> &whereValues,
                const std::vector<std::string> &updateColumns,
                const std::vector<SQLiteValue> &updateValues);

/**
 * @brief Updates rows with math in a table
 *
 * @param dbName Name/path of the database file
 * @param table Name of the table to update
 * @param whereColumns Columns to match for selection
 * @param whereValues Values to match in those columns
 * @param updateColumn Column to update
 * @param mathValue Arithmetic operation (e.g., "+ 5.0")
 * @return true if update successful, false otherwise
 *
 * @code
 * // Math operation (increase price by 5)
 * QueryData::updateRows("souvenirs.db", "souvenirs",
 *                       {"college"}, {"UCLA"},
 *                       "price", "+ 5.0");
 * @endcode
 * Time Complexity: O(n) where n is number of rows scanned
 */
bool doMathUpdate(const std::string &dbName, const std::string &table,
                  const std::vector<std::string> &whereColumns,
                  const std::vector<SQLiteValue> &whereValues,
                  const std::string &updateColumn,
                  const std::string &mathValue);

/**
 * @brief Selects rows from a database table
 *
 * Retrieves data from specified columns, optionally filtered by
 * WHERE conditions. Returns results as vector of row maps.
 *
 * @param dbName Name/path of the database file
 * @param table Name of the table to query
 * @param columns Columns to retrieve (empty for all)
 * @param whereColumns Optional columns for WHERE clause
 * @param whereValues Optional values for WHERE clause
 * @return QueryResult (vector of Row maps)
 *
 * @code
 * // Get all souvenirs for a campus
 * QueryResult results = QueryData::selectRows(
 *     "souvenirs.db", "souvenirs",
 *     {"item", "price"},
 *     {"college"},
 *     {"UCLA"});
 *
 * // Extract values from results
 * for (const auto &row : results) {
 *     std::string item = std::get<std::string>(row.at("item"));
 *     double price = std::get<double>(row.at("price"));
 * }
 * @endcode
 *
 * Time Complexity: O(n) where n is number of rows scanned
 */
Table selectRows(const std::string &dbName, const std::string &table,
                 const std::vector<std::string> &columns,
                 const std::vector<std::string> &whereColumns = {},
                 const std::vector<SQLiteValue> &whereValues  = {});

/**
 * @brief Executes a custom SELECT query
 *
 * For complex queries not supported by selectRows().
 * Use with caution - no parameter binding (SQL injection risk).
 *
 * @param dbName Name/path of the database file
 * @param query Complete SQL SELECT query string
 * @return QueryResult (vector of Row maps)
 *
 * @code
 * QueryResult results = QueryData::selectRowsWithQuery(
 *     "distances.db",
 *     "SELECT DISTINCT starting_college FROM distances ORDER BY
starting_college");
 * @endcode
 *
 * @warning Query string is executed directly - sanitize inputs
 */
Table selectRowsWithQuery(const std::string &dbName, const std::string &query);

/**
 * @brief Executes a custom non-SELECT query
 *
 * For INSERT, UPDATE, DELETE, or DDL statements not supported
 * by the other functions. Use with caution.
 *
 * @param dbName Name/path of the database file
 * @param query Complete SQL query string
 * @return true if execution successful, false otherwise
 *
 * @warning Query string is executed directly - sanitize inputs
 */
bool customExecuteQuery(const std::string &dbName, const std::string &query);

/**
 * @brief Merges data from a table in an external database into a table in the
 * main database.
 *
 * This function performs a column-wise merge between two tables located in
 * separate databases. The specified columns from the source table
 * (`mergeFromTable`) in `otherFile` are inserted into the destination table
 * (`mergeIntoTable`) in `mainDBName`.
 *
 * The column mappings are defined by `mergeIntoColumns` and `mergeFromColumns`,
 * which must be of equal length and correspond positionally.
 *
 * @param mainDBName Name or path of the primary database.
 * @param otherFile Name or path of the external database file to merge from.
 * @param mergeIntoTable Name of the destination table in the primary database.
 * @param mergeFromTable Name of the source table in the external database.
 * @param mergeIntoColumns List of column names in the destination table.
 * @param mergeFromColumns List of corresponding column names in the source
 * table.
 *
 * @return true if the merge operation succeeds; false otherwise.
 *
 * @note The number of columns in `mergeIntoColumns` must match
 * `mergeFromColumns`.
 * @warning Behavior is undefined if column mappings are inconsistent or tables
 * do not exist.
 */
bool mergeDatabases(const std::string &mainDBName, const std::string &otherFile,
                    const std::string &mergeIntoTable,
                    const std::string &mergeFromTable,
                    const std::vector<std::string> &mergeIntoColumns,
                    const std::vector<std::string> &mergeFromColumns);
/**
 * @brief Merges unique records from a table in an external database into a
 * table in the main database.
 *
 * This function inserts records from `mergeFromTable` in `otherFile` into
 * `mergeIntoTable` in `mainDBName`, ensuring that only rows with unique values
 * (based on specified columns) are added.
 *
 * The uniqueness of records is determined by comparing `uniqueIntoColumn` in
 * the destination table with `uniqueFromColumn` in the source table.
 *
 * @param mainDBName Name or path of the primary database.
 * @param otherFile Name or path of the external database file to merge from.
 * @param mergeIntoTable Name of the destination table in the primary database.
 * @param mergeFromTable Name of the source table in the external database.
 * @param uniqueIntoColumn Column in the destination table used to enforce
 * uniqueness.
 * @param uniqueFromColumn Column in the source table used for uniqueness
 * comparison.
 *
 * @return true if the merge operation succeeds; false otherwise.
 *
 * @note Only rows with values not already present in the destination column
 * will be inserted.
 * @warning Behavior is undefined if the specified columns or tables do not
 * exist.
 */
bool mergeUniqueDatabases(const std::string &mainDBName,
                          const std::string &otherFile,
                          const std::string &mergeIntoTable,
                          const std::string &mergeFromTable,
                          const std::string &uniqueIntoColumn,
                          const std::string &uniqueFromColumn);

} // namespace Query

#endif
