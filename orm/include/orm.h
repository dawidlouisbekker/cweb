#include <iostream>
#include <string>
#include <vector>

#define STRUCT_NAME(name) static std::string static_get_struct_name() { return #name; }

#define BEGIN_COLUMNS() \
    static std::vector<std::string> static_get_column_names() { \
        std::vector<std::string> names;

#define COLUMN(type, name) \
    type name; \
    names.push_back(#name);

#define END_COLUMNS() \
        return names; \
    }

struct TableBase {
    virtual std::string get_struct_name() const = 0;
    virtual std::vector<std::string> get_column_names() const = 0;

    void printall_variables() const {
        std::cout << "Variables of " << get_struct_name() << ":\n";
        for (const auto& name : get_column_names()) {
            std::cout << name << std::endl;
        };
    }
};