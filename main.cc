#include "reflect.hpp"
#include <string>

struct Params {
    std::string hello;
};

bool operator==(const Params& l, const Params& r)
{
    return l.hello == r.hello;
}

REFLECT_STRUCT(Params, (hello));

struct Entity {
    uint64_t id;
    std::string name;
    Params params;
};

REFLECT_STRUCT(Entity, (id)(name)(params));

struct Items {
    std::string name;
    std::vector<Params> items;
};

REFLECT_STRUCT(Items, (name)(items));

void test_items()
{
    const char* fmt = R"({"name":"ooknn","items":[{"hello":"one"},{"hello":"two"}]})";
    Items one;
    one.name = "ooknn";
    one.items.push_back({ "one" });
    one.items.push_back({ "two" });
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        ooknn::JsonWriter json_writer(&writer);
        reflect(json_writer, one);
        std::string input = sb.GetString();
        assert(input == fmt);
    }
    {

        rapidjson::Document reader;
        rapidjson::ParseResult ok = reader.Parse(fmt);
        assert(ok);
        ooknn::JsonReader json_reader { &reader };

        Items two;
        reflect(json_reader, two);
        assert(two.name == one.name);
        assert(two.items == one.items);
    }
}

void test_reflect()
{
    const char* fmt = R"({"id":10,"name":"hello","params":{"hello":"world"}})";
    Entity one { 10, "hello", { "world" } };
    {
        rapidjson::Document reader;
        rapidjson::ParseResult ok = reader.Parse(fmt);
        assert(ok);
        ooknn::JsonReader json_reader { &reader };

        Entity two;
        reflect(json_reader, two);

        assert(one.id == two.id);
        assert(one.name == two.name);
        assert(one.params.hello == two.params.hello);
    }
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
        ooknn::JsonWriter json_writer(&writer);
        reflect(json_writer, one);
        std::string input = sb.GetString();
        assert(input == fmt);
    }
}

int main(void)
{
    test_reflect();
    test_items();
    return 0;
}
