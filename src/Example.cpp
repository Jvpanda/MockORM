#include <Queries.h>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

// Example Program
int main()
{
        cout << "All colleges at start:\n";
        Query::Table AllColleges = Query::selectRows(
            "Databases/distances.db", "distances",
            {"starting_college", "ending_college", "distance"});

        for (auto &row : AllColleges)
        {
                cout << left << setw(45) << row["starting_college"] << ""
                     << setw(45) << row["ending_college"] << ""
                     << row["distance"] << std::endl;
        }

        cout << "\nInserting New Rows:\n";
        Query::insertRow("Databases/distances.db", "distances",
                         {"starting_college", "ending_college", "distance"},
                         {"Test Skool", "Test Escuela", 67.9});

        Query::insertRow("Databases/distances.db", "distances",
                         {"starting_college", "ending_college", "distance"},
                         {"Test Skool", "Test Escuela 2", 69.7});

        Query::Table TestSkool2 = Query::selectRows(
            "Databases/distances.db", "distances",
            {"starting_college", "ending_college", "distance"},
            {"starting_college"}, {"Test Skool"});

        for (auto &row : TestSkool2)
        {
                cout << left << setw(45) << row["starting_college"] << ""
                     << setw(45) << row["ending_college"] << ""
                     << row["distance"] << std::endl;
        }

        cout << "\nDeleting Distance to Test Escuela and regetting rows\n";
        Query::deleteRow("Databases/distances.db", "distances",
                         {"starting_college", "ending_college", "distance"},
                         {"Test Skool", "Test Escuela", 67.9});

        Query::Table afterDelete = Query::selectRows(
            "Databases/distances.db", "distances",
            {"starting_college", "ending_college", "distance"},
            {"starting_college"}, {"Test Skool2"});

        for (auto &row : afterDelete)
        {
                cout << left << setw(45) << row["starting_college"] << ""
                     << setw(45) << row["ending_college"] << ""
                     << row["distance"] << std::endl;
        }

        cout << "\nUpdating and merging into final database state\n";

        Query::updateRows("Databases/distances.db", "distances",
                          {"starting_college"}, {"Arizona State University"},
                          {"distance"}, {0});

        Query::doMathUpdate("Databases/distances.db", "distances",
                            {"starting_college", "ending_college"},
                            {"Arizona State University", "Northwestern"},
                            "distance", "-1000");

        Query::mergeDatabases(
            "Databases/distances.db", "testDatabases/new_campuses.db",
            "distances", "distances",
            {"starting_college", "ending_college", "distance"},
            {"starting_college", "ending_college", "distance"});

        Query::mergeUniqueDatabases("Databases/distances.db",
                                    "testDatabases/new_campuses.db", "colleges",
                                    "distances", "college", "starting_college");

        Query::Table AllCollegesEnd = Query::selectRows(
            "Databases/distances.db", "distances",
            {"starting_college", "ending_college", "distance"});
        for (auto &row : AllCollegesEnd)
        {
                cout << left << setw(45) << row["starting_college"] << ""
                     << setw(45) << row["ending_college"] << ""
                     << row["distance"] << std::endl;
        }

        return 0;
}
