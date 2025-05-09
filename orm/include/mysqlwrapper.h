#include <iostream>
// Includes outside namespace
#if defined(USE_MYSQL)
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#elif defined(USE_POSTGRES)
#include <pqxx/pqxx>
#endif



namespace sqlorm {

    class SessionMaker {
        public:
            SessionMaker(std::string uri, std::string username, std::string password) {
                if (uri.substr(0, 3) != "tcp") {
                    std::cout << "Not a TCP URI!" << std::endl;
                    std::cout << "Please provide a TCP URI" << std::endl;
                    exit(1);
                };
            };

        private:
            
    };

    ///std::string uri
    int connect() {

#if defined(USE_MYSQL)
        try {
            sql::mysql::MySQL_Driver *driver;
            sql::Connection *con;
            sql::Statement *stmt;
            sql::ResultSet *res;

            // Connect to MySQL (host, user, password)
            driver = sql::mysql::get_mysql_driver_instance();
            con = driver->connect("tcp://127.0.0.1:3307", "testuser", "testpass");

            // Connect to the database
            con->setSchema("testdb");

            // Create and execute a query
            stmt = con->createStatement();
            res = stmt->executeQuery("SELECT 'Hello from MySQL!' AS _message");

            while (res->next()) {
                std::cout << "Message: " << res->getString("_message") << std::endl;
            }

            delete res;
            delete stmt;
            delete con;

        } catch (sql::SQLException &e) {
            std::cerr << "ERROR: SQLException: " << e.what()
                    << " (MySQL error code: " << e.getErrorCode()
                    << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
#elif defined(USE_POSTGRES)
        try {
            pqxx::connection conn("host=127.0.0.1 port=5433 dbname=testdb user=testuser password=testpass");
    
            if (conn.is_open()) {
                std::cout << "Connected to " << conn.dbname() << std::endl;
    
                pqxx::work txn(conn);
                pqxx::result res = txn.exec("SELECT 'Hello from PostgreSQL!'");
    
                for (const auto& row : res) {
                    std::cout << row[0].c_str() << std::endl;
                }
    
                txn.commit();
            } else {
                std::cerr << "Connection failed." << std::endl;
            }
    
        } catch (const std::exception &e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
#endif
        return 0;
    }


};
