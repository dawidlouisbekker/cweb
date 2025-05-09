#include "include/orm.h"
#include "include/raw.h"
//#include "include/mysqlwrapper.h"

using std::string;

struct User : public TableBase {
    STRUCT_NAME(Users);

    BEGIN_COLUMNS()
        COLUMN(string, username)
        COLUMN(string, email)
    END_COLUMNS()

    // Implement virtuals correctly
    std::string get_struct_name() const override {
        return static_get_struct_name();
    }

    std::vector<std::string> get_column_names() const override {
        return static_get_column_names();
    }
};

struct Item : public TableBase {
    STRUCT_NAME(Item);

    BEGIN_COLUMNS()
        COLUMN(string, id)
        COLUMN(string, price)
    END_COLUMNS()

    // Implement virtuals correctly
    std::string get_struct_name() const override {
        return static_get_struct_name();
    }

    std::vector<std::string> get_column_names() const override {
        return static_get_column_names();
    }
};

using std::cout;
using std::endl;

int main() {
    int result = connect_mysql();
    return 0;
}



