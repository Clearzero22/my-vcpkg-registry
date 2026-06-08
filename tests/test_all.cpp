#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cassert>

#include <string-utils/string_utils.hpp>
#include <file-utils/file_utils.hpp>
#include <crc-hash/crc32.hpp>
#include <crc-hash/md5.hpp>
#include <binary-serializer/binary_serializer.hpp>
#include <object-pool/object_pool.hpp>
#include <ini-parser/ini_parser.hpp>
#include <simple-logger/logger.hpp>
#include <thread-pool/thread_pool.hpp>
#include <uuid/uuid.hpp>
#include <base64/base64.hpp>
#include <json-config/json_config.hpp>
#include <httplib.h>

static int passed = 0, failed = 0;

#define TEST(name) do { printf("  TEST %s ... ", name); } while(0)
#define PASS()     do { printf("PASS\n"); passed++; } while(0)
#define FAIL(msg)  do { printf("FAIL: %s\n", msg); failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

void test_string_utils() {
    printf("[string-utils]\n");
    TEST("trim"); CHECK(str::trim("  hello  ") == "hello", "trim"); CHECK(str::trim_left("  hello") == "hello", "trim_left"); CHECK(str::trim_right("hello  ") == "hello", "trim_right"); PASS();
    TEST("split/join"); auto parts = str::split("a,b,c", ","); CHECK(parts.size() == 3, "split size"); CHECK(parts[0] == "a", "split[0]"); CHECK(str::join(parts, ", ") == "a, b, c", "join"); PASS();
    TEST("case"); CHECK(str::to_lower("HELLO") == "hello", "lower"); CHECK(str::to_upper("hello") == "HELLO", "upper"); PASS();
    TEST("hex"); auto hex = str::to_hex(std::vector<std::byte>{std::byte{0x48}, std::byte{0x65}, std::byte{0x6c}}); CHECK(hex == "48656c", "to_hex"); auto bin = str::from_hex("48656c"); CHECK(bin.size() == 3, "from_hex"); PASS();
    TEST("replace"); CHECK(str::replace("hello world", "world", "there") == "hello there", "replace"); std::string s = "foo bar"; CHECK(str::replace_in_place(s, "foo", "baz"), "in_place"); CHECK(s == "baz bar", "in_place result"); PASS();
    TEST("check"); CHECK(str::starts_with("hello", "he"), "starts_with"); CHECK(str::ends_with("hello", "lo"), "ends_with"); CHECK(str::contains("hello", "ell"), "contains"); CHECK(!str::contains("hello", "xyz"), "not contains"); PASS();
}

void test_file_utils() {
    printf("[file-utils]\n");
    auto tmp = (std::filesystem::temp_directory_path() / "vcpkg_test_file.txt").string();
    TEST("write/read"); CHECK(file::write_all(tmp, "hello world"), "write"); auto c = file::read_all(tmp); CHECK(c.has_value() && *c == "hello world", "read"); PASS();
    TEST("append"); CHECK(file::append_all(tmp, " more"), "append"); c = file::read_all(tmp); CHECK(c.has_value() && *c == "hello world more", "append read"); PASS();
    TEST("path utils"); CHECK(file::filename("/a/b/c.txt") == "c.txt", "filename"); CHECK(file::stem("/a/b/c.txt") == "c", "stem"); CHECK(file::extension("/a/b/c.txt") == ".txt", "ext"); PASS();
    TEST("exists"); CHECK(file::exists(tmp), "exists"); CHECK(file::is_file(tmp), "is_file"); PASS();
    auto tmp2 = (std::filesystem::temp_directory_path() / "vcpkg_test_file2.txt").string();
    CHECK(file::copy_file(tmp, tmp2, true), "copy"); CHECK(file::exists(tmp2), "exists2"); CHECK(file::remove(tmp), "remove"); CHECK(file::remove(tmp2), "remove2"); CHECK(!file::exists(tmp), "gone"); PASS();
}

void test_crc_hash() {
    printf("[crc-hash]\n");
    TEST("crc32"); CHECK(hash::crc32("hello") == 0x3610a686, "crc32"); PASS();
    TEST("md5"); CHECK(hash::md5("hello").to_string() == "5d41402abc4b2a76b9719d911017c592", "md5"); PASS();
    TEST("diff"); CHECK(hash::md5("hello") != hash::md5("world"), "md5 diff"); CHECK(hash::crc32("hello") != hash::crc32("world"), "crc32 diff"); PASS();
}

void test_binary_serializer() {
    printf("[binary-serializer]\n");
    TEST("integers"); bin::writer w; w.write(42); w.write(-1); w.write(static_cast<std::uint64_t>(123456789));
    bin::reader r(w.data()); auto v1 = r.read<int>(); auto v2 = r.read<int>(); auto v3 = r.read<std::uint64_t>();
    CHECK(v1.has_value() && *v1 == 42, "int"); CHECK(v2.has_value() && *v2 == -1, "neg"); CHECK(v3.has_value() && *v3 == 123456789, "u64"); CHECK(r.eof(), "eof"); PASS();
    TEST("strings"); w.clear(); w.write(std::string("hello")); w.write(std::string(""));
    bin::reader r2(w.data()); auto s1 = r2.read<std::string>(); auto s2 = r2.read<std::string>();
    CHECK(s1.has_value() && *s1 == "hello", "str"); CHECK(s2.has_value() && s2->empty(), "empty"); PASS();
    TEST("vectors"); w.clear(); w.write(std::vector<int>{10, 20, 30});
    bin::reader r3(w.data()); auto vec = r3.read_vector<int>();
    CHECK(vec.has_value() && vec->size() == 3 && (*vec)[0] == 10, "vec"); PASS();
    TEST("map"); w.clear(); std::map<std::string, int> m = {{"a", 1}, {"b", 2}}; w.write(m);
    CHECK(w.bytes().size() > 0, "map bytes"); PASS();
}

void test_object_pool() {
    printf("[object-pool]\n");
    TEST("acquire/release"); pool::object_pool<int> pool(4, false); CHECK(pool.available() == 4, "init"); { auto obj = pool.acquire(); *obj = 42; CHECK(pool.available() == 3, "acquired"); } CHECK(pool.available() == 4, "released"); PASS();
    TEST("multi"); std::vector<std::shared_ptr<int>> objs; for (int i = 0; i < 4; ++i) objs.push_back(pool.acquire()); CHECK(pool.available() == 0, "all acquired"); objs.clear(); CHECK(pool.available() == 4, "all released"); PASS();
}

void test_ini_parser() {
    printf("[ini-parser]\n");
    TEST("parse"); ini::parser p; CHECK(p.load_from_string("[section]\nkey=value\n"), "load"); CHECK(p.has_section("section"), "section"); CHECK(p.has_key("section", "key"), "key"); CHECK(p.get("section", "key").has_value() && *p.get("section", "key") == "value", "get"); PASS();
    TEST("modify"); p.set("section", "key", "new"); CHECK(*p.get("section", "key") == "new", "modified"); PASS();
    TEST("quoted"); ini::parser p2; CHECK(p2.load_from_string("[sec]\nk=\"quoted value\"\n"), "load2"); CHECK(*p2.get("sec", "k") == "quoted value", "quoted"); PASS();
    TEST("missing"); CHECK(!p.get("nonexistent", "key").has_value(), "missing section"); CHECK(!p.has_key("section", "nonexistent"), "missing key"); PASS();
}

void test_simple_logger() {
    printf("[simple-logger]\n");
    TEST("basic"); auto& log = simple::logger::instance(); log.add_console_sink(); log.set_level(simple::log_level::debug);
    LOG_INFO("Test", "This is a test message"); LOG_DEBUG("Test", "Debug"); LOG_WARN("Test", "Warn"); LOG_ERROR("Test", "Error");
    PASS();
}

void test_thread_pool() {
    printf("[thread-pool]\n");
    TEST("basic"); tp::thread_pool pool(4); CHECK(pool.enqueue([]{ return 42; }).get() == 42, "basic"); PASS();
    TEST("multi"); std::vector<std::future<int>> futures; for (int i = 0; i < 10; ++i) futures.push_back(pool.enqueue([i] { return i * i; })); for (int i = 0; i < 10; ++i) CHECK(futures[i].get() == i * i, "multi"); PASS();
    TEST("void"); std::atomic<int> counter{0}; auto f = pool.enqueue([&counter] { counter++; }); f.get(); CHECK(counter == 1, "void"); PASS();
}

void test_uuid() {
    printf("[uuid]\n");
    TEST("generate"); auto u1 = uuid::uuid::v4(); auto u2 = uuid::uuid::v4(); CHECK(u1 != u2, "unique"); CHECK(u1.to_string().size() == 36, "format"); CHECK(u1.to_string()[8] == '-', "dash"); CHECK(u1.to_string_compact().size() == 32, "compact"); PASS();
    TEST("parse"); auto p = uuid::uuid::parse("550e8400-e29b-41d4-a716-446655440000"); CHECK(p.has_value(), "parsed"); CHECK(p->to_string() == "550e8400-e29b-41d4-a716-446655440000", "roundtrip"); PASS();
    TEST("hash"); std::unordered_map<uuid::uuid, int> map; map[u1] = 1; map[u2] = 2; CHECK(map.size() == 2, "hash"); PASS();
}

void test_base64() {
    printf("[base64]\n");
    TEST("encode/decode"); auto enc = base64::encode("hello world"); auto dec = base64::decode_to_string(enc); CHECK(dec.has_value() && *dec == "hello world", "roundtrip"); PASS();
    TEST("empty"); CHECK(base64::encode("") == "", "empty enc"); auto d = base64::decode_to_string(""); CHECK(d.has_value() && d->empty(), "empty dec"); PASS();
    TEST("binary"); std::vector<std::byte> data = { std::byte{0x00}, std::byte{0x01}, std::byte{0xFF} }; auto e = base64::encode(data); auto db = base64::decode(e); CHECK(db.has_value() && db->size() == 3 && std::to_integer<int>((*db)[0]) == 0x00, "binary"); PASS();
    TEST("known"); CHECK(base64::encode("f") == "Zg==", "1 byte"); CHECK(base64::encode("fo") == "Zm8=", "2 byte"); CHECK(base64::encode("foo") == "Zm9v", "3 byte"); CHECK(base64::encode("foob") == "Zm9vYg==", "4 byte"); PASS();
    TEST("invalid"); CHECK(!base64::decode("not-valid!!!").has_value(), "invalid chars"); PASS();
}

void test_json_config() {
    printf("[json-config]\n");
    TEST("parse object"); auto v = json::value::parse(R"({"name":"test","count":42,"flag":true})"); CHECK(!v.is_null(), "not null"); CHECK(v["name"].as_string() == "test", "string"); CHECK(v["count"].as_integer() == 42, "int"); CHECK(v["flag"].as_bool(), "bool"); PASS();
    TEST("array"); auto arr = json::value::parse("[1,2,3]"); CHECK(arr.is_array(), "array"); CHECK(arr[0].as_integer() == 1, "[0]"); CHECK(arr[2].as_integer() == 3, "[2]"); PASS();
    TEST("nested"); auto n = json::value::parse(R"({"a":{"b":{"c":123}}})"); CHECK(n["a"]["b"]["c"].as_integer() == 123, "nested"); PASS();
    TEST("dump/reparse"); auto cfg = json::value::parse(R"({"x":1,"y":"hello"})"); auto reloaded = json::value::parse(cfg.dump()); CHECK(reloaded["x"].as_integer() == 1, "x"); CHECK(reloaded["y"].as_string() == "hello", "y"); PASS();
    TEST("parse file"); auto tmp = std::filesystem::temp_directory_path() / "test_config.json"; { std::ofstream f(tmp); f << R"({"server":{"port":8080}})"; } auto ff = json::value::parse_file(tmp.string()); CHECK(ff["server"]["port"].as_integer() == 8080, "file"); std::filesystem::remove(tmp); PASS();
    TEST("null/bool/number"); CHECK(json::value::parse("null").is_null(), "null"); CHECK(!json::value::parse("false").as_bool(), "false"); CHECK(json::value::parse("3.14").is_real(), "real"); PASS();
}

void test_httplib() {
    printf("[cpp-httplib]\n");
    TEST("client GET"); httplib::Client cli("http://httpbin.org"); cli.set_connection_timeout(5); auto res = cli.Get("/get"); CHECK(res, "response"); CHECK(res->status == 200, "status"); PASS();
}

int main() {
    printf("\n=== vcpkg-registry library tests ===\n\n");
    test_string_utils();
    test_file_utils();
    test_crc_hash();
    test_binary_serializer();
    test_object_pool();
    test_ini_parser();
    test_simple_logger();
    test_thread_pool();
    test_uuid();
    test_base64();
    test_json_config();
    test_httplib();
    printf("\n=== Results: %d passed, %d failed ===\n\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
