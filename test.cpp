#include "project/json_protocol.hpp"

int main ()
{
    json invalid = "\"{\\\"password\\\":\\\"12\\\",\\\"username\\\":\\\"12\\\"}\"";
    
    std::string username;
    bool valid = AssignIfPresent(invalid, "username", username);
    std::cout << "Valid: " << valid << ", Value: " << username << std::endl;
    // 输出：Valid: 0, Value: 

    // 正确方式：使用解析后的JSON对象
    json valid_json = json::parse("{\"password\":\"12\",\"username\":\"12\"}");
    valid = AssignIfPresent(valid_json, "username", username);
    std::cout << "Valid: " << valid << ", Value: " << username << std::endl;
}