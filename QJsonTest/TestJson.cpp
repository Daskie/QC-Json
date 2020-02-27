#include "CppUnitTest.h"

#include "QJson.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace std::string_literals;
using namespace std::string_view_literals;

struct CustomVal { int x, y; };

bool operator==(const CustomVal & cv1, const CustomVal & cv2) {
    return cv1.x == cv2.x && cv1.y == cv2.y;
}

namespace Microsoft {
    namespace VisualStudio {
        namespace CppUnitTestFramework {
            template <> static std::wstring ToString<uint16_t>(const uint16_t & v) { return std::to_wstring(v); }
            template <> static std::wstring ToString<nullptr_t>(const nullptr_t & v) { return L"null"; }
            template <> static std::wstring ToString<qjson::Type>(const qjson::Type & type) { return std::to_wstring(std::underlying_type_t<qjson::Type>(type)); }
            template <> static std::wstring ToString<CustomVal>(const CustomVal & v) { return L"(" + std::to_wstring(v.x) + L", " + std::to_wstring(v.y) + L")"; }
        }
    }
}

template <bool unsafe>
struct qjson_valueTo<CustomVal, unsafe> {
    CustomVal operator()(const qjson::Value & val) const {
        const qjson::Array & arr(val.asArray<unsafe>());
        return {arr.at(0).as<int, unsafe>(), arr.at(1).as<int, unsafe>()};
    }
};

template <>
struct qjson_valueFrom<CustomVal> {
    qjson::Value operator()(const CustomVal & v) const {
        return qjson::Array(v.x, v.y);
    }
};

TEST_CLASS(Json) {

  public:

    template <typename T>
    void thereAndBackAgain(T v) {
        Assert::AreEqual(v, qjson::decode(qjson::encode(v)).as<T>());
    }

    TEST_METHOD(EncodeDecodeString) {
        // Empty
        thereAndBackAgain(""sv);
        // Typical
        thereAndBackAgain("abc"sv);
        // Printable characters
        thereAndBackAgain(R"( !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~)"sv);
        // Escape characters
        thereAndBackAgain("\b\f\n\r\t"sv);
        // Unicode
        thereAndBackAgain("\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007\u000B\u000E\u000F\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F\u007F"sv);
    }

    TEST_METHOD(EncodeDecodeInteger) {
        // Zero
        thereAndBackAgain(0);
        // Typical
        thereAndBackAgain(123);
        // Max 64
        thereAndBackAgain(int64_t(9223372036854775807));
        // Min 64
        thereAndBackAgain(int64_t(9223372036854775808));
        // Max 64 unsigned
        thereAndBackAgain(uint64_t(0xFFFFFFFFFFFFFFFFu));
        // Max 32
        thereAndBackAgain(int32_t(2147483647));
        // Min 32
        thereAndBackAgain(int32_t(2147483648));
        // Max 32 unsigned
        thereAndBackAgain(uint32_t(0xFFFFFFFFu));
        // Max 16
        thereAndBackAgain(int16_t(32767));
        // Min 16
        thereAndBackAgain(int16_t(-32768));
        // Max 16 unsigned
        thereAndBackAgain(uint16_t(0xFFFFu));
        // Max 8
        thereAndBackAgain(int8_t(127));
        // Min 8
        thereAndBackAgain(int8_t(128));
        // Max 8 unsigned
        thereAndBackAgain(uint8_t(0xFFu));
    }

    TEST_METHOD(EncodeDecodeFloater) {
        uint64_t val64;
        uint32_t val32;
        // Zero
        thereAndBackAgain(0.0);
        // Typical
        thereAndBackAgain(123.45);
        // Max integer 64
        thereAndBackAgain(reinterpret_cast<const double &>(val64 = 0b0'10000110011'1111111111111111111111111111111111111111111111111111u));
        // Max integer 32
        thereAndBackAgain(reinterpret_cast<const float &>(val32 = 0b0'10010110'11111111111111111111111u));
        // Max 64
        thereAndBackAgain(reinterpret_cast<const double &>(val64 = 0b0'11111111110'1111111111111111111111111111111111111111111111111111u));
        // Max 32
        thereAndBackAgain(reinterpret_cast<const float &>(val32 = 0b0'11111110'11111111111111111111111u));
        // Min normal 64
        thereAndBackAgain(reinterpret_cast<const double &>(val64 = 0b0'00000000001'0000000000000000000000000000000000000000000000000000u));
        // Min normal 32
        thereAndBackAgain(reinterpret_cast<const float &>(val32 = 0b0'00000001'00000000000000000000000u));
        // Min subnormal 64
        thereAndBackAgain(reinterpret_cast<const double &>(val64 = 0b0'00000000000'0000000000000000000000000000000000000000000000000001u));
        // Min subnormal 32
        thereAndBackAgain(reinterpret_cast<const float &>(val32 = 0b0'00000000'00000000000000000000001u));
        // Positive infinity
        thereAndBackAgain(std::numeric_limits<double>::infinity());
        // Negative infinity
        thereAndBackAgain(-std::numeric_limits<double>::infinity());
        // NaN
        Assert::IsTrue(std::isnan(qjson::decode(qjson::encode(std::numeric_limits<double>::quiet_NaN())).asFloater()));
    }

    TEST_METHOD(EncodeDecodeBoolean) {
        // true
        thereAndBackAgain(true);
        // false
        thereAndBackAgain(false);
    }

    TEST_METHOD(EncodeDecodeNull) {
        thereAndBackAgain(nullptr);
    }

    TEST_METHOD(EncodeDecodeCustom) {
        thereAndBackAgain(CustomVal{1, 2});
    }

    TEST_METHOD(ValueConstruction) {
        // Default
        Assert::AreEqual(qjson::Type::null, qjson::Value().type());
        // Object
        Assert::AreEqual(qjson::Type::object, qjson::Value(qjson::Object()).type());
        // Array
        Assert::AreEqual(qjson::Type::array, qjson::Value(qjson::Array()).type());
        // String
        Assert::AreEqual(qjson::Type::string, qjson::Value("abc"sv).type());
        Assert::AreEqual(qjson::Type::string, qjson::Value("abc"s).type());
        Assert::AreEqual(qjson::Type::string, qjson::Value("abc").type());
        Assert::AreEqual(qjson::Type::string, qjson::Value(const_cast<char *>("abc")).type());
        Assert::AreEqual(qjson::Type::string, qjson::Value('a').type());
        // Integer
        Assert::AreEqual(qjson::Type::integer, qjson::Value(int64_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(uint64_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(int32_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(uint32_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(int16_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(uint16_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(int8_t(0)).type());
        Assert::AreEqual(qjson::Type::integer, qjson::Value(uint8_t(0)).type());
        // Floater
        Assert::AreEqual(qjson::Type::floater, qjson::Value(0.0).type());
        Assert::AreEqual(qjson::Type::floater, qjson::Value(0.0f).type());
        // Boolean
        Assert::AreEqual(qjson::Type::boolean, qjson::Value(false).type());
        // Null
        Assert::AreEqual(qjson::Type::null, qjson::Value(nullptr).type());
    }

    TEST_METHOD(ValueMove) {
        qjson::Value v1("abc"sv);
        Assert::AreEqual(qjson::Type::string, v1.type());
        Assert::AreEqual("abc"sv, v1.asString());

        qjson::Value v2(std::move(v1));
        Assert::AreEqual(qjson::Type::null, v1.type());
        Assert::AreEqual(qjson::Type::string, v2.type());
        Assert::AreEqual("abc"sv, v2.asString());

        v1 = std::move(v2);
        Assert::AreEqual(qjson::Type::string, v1.type());
        Assert::AreEqual("abc"sv, v1.asString());
        Assert::AreEqual(qjson::Type::null, v2.type());
    }

    TEST_METHOD(ValueTyping) {
        { // Object
            qjson::Value v(qjson::Object{});
            Assert::AreEqual(qjson::Type::object, v.type());
            Assert::IsTrue(v.is(qjson::Type::object));
            v.asObject<false>();
            v.asObject<true>();
        }
        { // Array
            qjson::Value v(qjson::Array{});
            Assert::AreEqual(qjson::Type::array, v.type());
            Assert::IsTrue(v.is(qjson::Type::array));
            v.asArray<false>();
            v.asArray<true>();
        }
        { // String
            qjson::Value v("abc"sv);
            Assert::AreEqual(qjson::Type::string, v.type());
            Assert::IsTrue(v.is(qjson::Type::string));
            v.asString<false>();
            v.asString<true>();
            v.as<std::string_view, false>();
            v.as<std::string_view, true>();
        }
        { // Integer
            qjson::Value v(0);
            Assert::AreEqual(qjson::Type::integer, v.type());
            Assert::IsTrue(v.is(qjson::Type::integer));
            v.asInteger<false>();
            v.asInteger<true>();
            v.as<int64_t, false>();
            v.as<int64_t, true>();
            v.as<uint64_t, false>();
            v.as<uint64_t, true>();
            v.as<int32_t, false>();
            v.as<int32_t, true>();
            v.as<uint32_t, false>();
            v.as<uint32_t, true>();
            v.as<int16_t, false>();
            v.as<int16_t, true>();
            v.as<uint16_t, false>();
            v.as<uint16_t, true>();
            v.as<int8_t, false>();
            v.as<int8_t, true>();
            v.as<uint8_t, false>();
            v.as<uint8_t, true>();
        }
        { // Floater
            qjson::Value v(0.0);
            Assert::AreEqual(qjson::Type::floater, v.type());
            Assert::IsTrue(v.is(qjson::Type::floater));
            v.asFloater<false>();
            v.asFloater<true>();
            v.as<double, false>();
            v.as<double, true>();
            v.as<float, false>();
            v.as<float, true>();
        }
        { // Boolean
            qjson::Value v(false);
            Assert::AreEqual(qjson::Type::boolean, v.type());
            Assert::IsTrue(v.is(qjson::Type::boolean));
            v.asBoolean<false>();
            v.asBoolean<true>();
            v.as<bool, false>();
            v.as<bool, true>();
        }
        { // Null
            qjson::Value v;
            Assert::AreEqual(qjson::Type::null, v.type());
            Assert::IsTrue(v.is(qjson::Type::null));
            v.as<nullptr_t, false>();
            v.as<nullptr_t, true>();
        }
    }

    TEST_METHOD(ValueAs) {
        // Safe
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asObject<false>(); } );
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asArray<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asString<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<std::string_view, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asInteger<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<int64_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<uint64_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<int32_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<uint32_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<int16_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<uint16_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<int8_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<uint8_t, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asFloater<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<double, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<float, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().asBoolean<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value().as<bool, false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value(0).asNull<false>(); });
        Assert::ExpectException<qjson::TypeError>([]() { qjson::Value(0).as<nullptr_t, false>(); });

        // Unsafe
        qjson::Value().asObject<true>();
        qjson::Value().asArray<true>();
        qjson::Value().asString<true>();
        qjson::Value().as<std::string_view, true>();
        qjson::Value().asInteger<true>();
        qjson::Value().as<int64_t, true>();
        qjson::Value().as<uint64_t, true>();
        qjson::Value().as<int32_t, true>();
        qjson::Value().as<uint32_t, true>();
        qjson::Value().as<int16_t, true>();
        qjson::Value().as<uint16_t, true>();
        qjson::Value().as<int8_t, true>();
        qjson::Value().as<uint8_t, true>();
        qjson::Value().asFloater<true>();
        qjson::Value().as<double, true>();
        qjson::Value().as<float, true>();
        qjson::Value().asBoolean<true>();
        qjson::Value().as<bool, true>();
        qjson::Value(0).asNull<true>();
        qjson::Value(0).as<nullptr_t, true>();
    }

    TEST_METHOD(Object) {
        qjson::Object obj;
        { // Initial state
            Assert::AreEqual(0u, obj.size());
            Assert::AreEqual(0u, obj.capacity());
            Assert::IsTrue(obj.empty());
            Assert::IsFalse(obj.contains("key"sv));
            Assert::IsTrue(obj.find("key"sv) == obj.end());
            Assert::ExpectException<std::out_of_range>([&obj]() { obj.at("key"sv); });
        }
        { // When adding a single element
            obj.add("k0"s, 0);
            Assert::AreEqual(1u, obj.size());
            Assert::AreEqual(8u, obj.capacity());
            Assert::IsFalse(obj.empty());
            Assert::IsTrue(obj.contains("k0"sv));
            auto it(obj.find("k0"sv));
            Assert::IsTrue(it != obj.end());
            Assert::AreEqual("k0"s, it->first);
            Assert::AreEqual(0, it->second.as<int>());
            Assert::AreEqual(0, obj.at("k0"sv).as<int>());
        }
        { // Filling up its capacity
            obj.add("k1"s, 1);
            obj.add("k2"s, 2);
            obj.add("k3"s, 3);
            obj.add("k4"s, 4);
            obj.add("k5"s, 5);
            obj.add("k6"s, 6);
            obj.add("k7"s, 7);
            Assert::AreEqual(8u, obj.size());
            Assert::AreEqual(8u, obj.capacity());
        }
        { // Exceeding its capacity
            obj.add("k8"s, 8);
            Assert::AreEqual(9u, obj.size());
            Assert::AreEqual(16u, obj.capacity());
        }
        { // Expected contents in order
            auto it(obj.cbegin());
            Assert::AreEqual("k0"s, it->first);
            Assert::AreEqual(0, it->second.as<int>());
            ++it;
            Assert::AreEqual("k1"s, it->first);
            Assert::AreEqual(1, it->second.as<int>());
            ++it;
            Assert::AreEqual("k2"s, it->first);
            Assert::AreEqual(2, it->second.as<int>());
            ++it;
            Assert::AreEqual("k3"s, it->first);
            Assert::AreEqual(3, it->second.as<int>());
            ++it;
            Assert::AreEqual("k4"s, it->first);
            Assert::AreEqual(4, it->second.as<int>());
            ++it;
            Assert::AreEqual("k5"s, it->first);
            Assert::AreEqual(5, it->second.as<int>());
            ++it;
            Assert::AreEqual("k6"s, it->first);
            Assert::AreEqual(6, it->second.as<int>());
            ++it;
            Assert::AreEqual("k7"s, it->first);
            Assert::AreEqual(7, it->second.as<int>());
            ++it;
            Assert::AreEqual("k8"s, it->first);
            Assert::AreEqual(8, it->second.as<int>());
            ++it;
            Assert::IsTrue(it == obj.cend());
        }
        { // Accessing some element
            Assert::IsTrue(obj.contains("k7"sv));
            auto it(obj.find("k7"sv));
            Assert::IsTrue(it != obj.end());
            Assert::AreEqual("k7"s, it->first);
            Assert::AreEqual(7, it->second.as<int>());
            Assert::AreEqual(7, obj.at("k7"sv).as<int>());
        }
        { // Moving
            qjson::Object obj2(std::move(obj));
            Assert::AreEqual(0u, obj.size());
            Assert::AreEqual(0u, obj.capacity());
            Assert::AreEqual(9u, obj2.size());
            Assert::AreEqual(16u, obj2.capacity());
            Assert::IsTrue(obj2.contains("k7"sv));
        }
    }

    TEST_METHOD(Array) {
        qjson::Array arr;
        { // Initial state
            Assert::AreEqual(0u, arr.size());
            Assert::AreEqual(0u, arr.capacity());
            Assert::IsTrue(arr.empty());
            Assert::ExpectException<std::out_of_range>([&arr]() { arr.at(0); });
        }
        { // When adding a single element
            arr.add(0);
            Assert::AreEqual(1u, arr.size());
            Assert::AreEqual(8u, arr.capacity());
            Assert::IsFalse(arr.empty());
            Assert::AreEqual(0, arr.at(0).as<int>());
            Assert::ExpectException<std::out_of_range>([&arr]() { arr.at(1); });
        }
        { // Filling up its capacity
            arr.add(1);
            arr.add(2);
            arr.add(3);
            arr.add(4);
            arr.add(5);
            arr.add(6);
            arr.add(7);
            Assert::AreEqual(8u, arr.size());
            Assert::AreEqual(8u, arr.capacity());
        }
        { // Exceeding its capacity
            arr.add(8);
            Assert::AreEqual(9u, arr.size());
            Assert::AreEqual(16u, arr.capacity());
        }
        { // Expected contents in order
            auto it(arr.cbegin());
            Assert::AreEqual(0, it->as<int>());
            ++it;
            Assert::AreEqual(1, it->as<int>());
            ++it;
            Assert::AreEqual(2, it->as<int>());
            ++it;
            Assert::AreEqual(3, it->as<int>());
            ++it;
            Assert::AreEqual(4, it->as<int>());
            ++it;
            Assert::AreEqual(5, it->as<int>());
            ++it;
            Assert::AreEqual(6, it->as<int>());
            ++it;
            Assert::AreEqual(7, it->as<int>());
            ++it;
            Assert::AreEqual(8, it->as<int>());
            ++it;
            Assert::IsTrue(it == arr.cend());
        }
        { // Accessing some element
            Assert::AreEqual(7, arr.at(7).as<int>());
            Assert::ExpectException<std::out_of_range>([&arr]() { arr.at(9); });
        }
        { // Moving
            qjson::Array arr2(std::move(arr));
            Assert::AreEqual(0u, arr.size());
            Assert::AreEqual(0u, arr.capacity());
            Assert::AreEqual(9u, arr2.size());
            Assert::AreEqual(16u, arr2.capacity());
            Assert::AreEqual(7, arr2.at(7).as<int>());
        }
        { // Explicit instantiation
            arr = qjson::Array(true, 6, "wow");
            Assert::AreEqual(3u, arr.size());
            Assert::AreEqual(0u, arr.capacity());
            Assert::AreEqual(true, arr.at(0).asBoolean());
            Assert::AreEqual(6, arr.at(1).as<int>());
            Assert::AreEqual("wow"sv, arr.at(2).asString());

            arr.add(nullptr);
            Assert::AreEqual(4u, arr.size());
            Assert::AreEqual(8u, arr.capacity());
            Assert::AreEqual(nullptr, arr.at(3).asNull());
        }
    }

    TEST_METHOD(String) {
        { // Standard
            qjson::String str("abc"sv);
            Assert::AreEqual(3u, str.size());
            Assert::AreEqual("abc"sv, str.view());
        }
        { // Inline
            qjson::String str("123456123456"sv);
            Assert::AreEqual(12u, str.size());
            Assert::AreEqual("123456123456"sv, str.view());
            Assert::AreEqual(reinterpret_cast<const char *>(&str) + 4, str.view().data());

            // Moving
            qjson::String str2(std::move(str));
            Assert::AreEqual(""sv, str.view());
            Assert::AreEqual("123456123456"sv, str2.view());
        }
        { // Dynamic
            qjson::String str("1234561234561"sv);
            Assert::AreEqual(13u, str.size());
            Assert::AreEqual("1234561234561"sv, str.view());
            Assert::AreNotEqual(reinterpret_cast<const char *>(&str) + 4, str.view().data());

            // Moving
            qjson::String str2(std::move(str));
            Assert::AreEqual(""sv, str.view());
            Assert::AreEqual("1234561234561"sv, str2.view());
        }
    }

    TEST_METHOD(General) {
        std::string json(R"({
    "Dishes": [
        {
            "Gluten Free": false,
            "Ingredients": [
                "Salt",
                "Barnacles"
            ],
            "Name": "Basket o' Barnacles",
            "Price": 5.45
        },
        {
            "Gluten Free": true,
            "Ingredients": [
                "Tuna"
            ],
            "Name": "Two Tuna",
            "Price": 14.99
        },
        {
            "Gluten Free": false,
            "Ingredients": [
                "Salt",
                "Octopus",
                "Crab"
            ],
            "Name": "18 Leg Bouquet",
            "Price": 18.0
        }
    ],
    "Employees": [
        {
            "Age": 69,
            "Name": "Ol' Joe Fisher",
            "Title": "Fisherman"
        },
        {
            "Age": 41,
            "Name": "Mark Rower",
            "Title": "Cook"
        },
        {
            "Age": 19,
            "Name": "Phineas",
            "Title": "Server Boy"
        }
    ],
    "Founded": 1964,
    "Name": "Salt's Crust",
    "Profit Margin": null
})"s);
        Assert::AreEqual(json, qjson::encode(qjson::decode(json)));
    }

};
