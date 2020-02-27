#pragma once

//==============================================================================
// QJson 1.1.0
// Austin Quick
// July 2019 - February 2020
//------------------------------------------------------------------------------
// Basic, lightweight JSON encoder.
//
// Encodes a json string.
//
// Basic Usage:
//
//      qjson::Encoder encoder;
//      encoder.object();
//      encoder.key("Name").val("Roslin");
//      encoder.key("Favorite Books").array(true).val("Dark Day").end();
//      ...
//      encoder.end();
//
//      std::string jsonString(encoder.finish());
//
// To allow custom types to be passed to `Encoder::val`, specialize the
// `qjson_encode` function.
//
// Example:
//
//      // Specialized for std::pair<int, int>
//      void qjson_encode(qjson::Encoder & encoder, const std::pair<int, int> & v) {
//          encoder.array(true).val(v.first).val(v.second).end();
//      }
//
//      ...
//
//      // You are then able to pass std::pair<int, int> to `val`
//      encoder.val(std::pair<int, int>{69, 420}); // -> encodes to "[ 69, 420 ]"
//
//------------------------------------------------------------------------------

#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace qjson {

#ifndef QJSON_COMMON
#define QJSON_COMMON

    struct Error : public std::runtime_error {

        Error() : std::runtime_error(nullptr) {}
        Error(const std::string & msg) : std::runtime_error(msg) {}

        ~Error() override = default;

    };

#endif

    constexpr bool defaultCompact{false};

    // This will be thrown if anything goes wrong during the encoding process.
    struct EncodeError : public Error {

        EncodeError(const std::string & msg) : Error(msg) {}

    };

    class Encoder {

      public:

        Encoder() = default;

        Encoder(const Encoder & other) = delete;
        Encoder(Encoder && other);

        Encoder & operator=(const Encoder & other) = delete;
        Encoder & operator=(Encoder && other) = delete;

        ~Encoder() = default;

        Encoder & object(bool compact = defaultCompact);

        Encoder & array(bool compact = defaultCompact);

        Encoder & key(std::string_view k);

        Encoder & val(std::string_view v);
        Encoder & val(const std::string & v);
        Encoder & val(const char * v);
        Encoder & val(char * v);
        Encoder & val(char v);
        Encoder & val(int64_t v);
        Encoder & val(uint64_t v);
        Encoder & val(int32_t v);
        Encoder & val(uint32_t v);
        Encoder & val(int16_t v);
        Encoder & val(uint16_t v);
        Encoder & val(int8_t v);
        Encoder & val(uint8_t v);
        Encoder & val(double v);
        Encoder & val(float v);
        Encoder & val(bool v);
        Encoder & val(nullptr_t);
        template <typename T> Encoder & val(const T & v);

        Encoder & end();

        std::string finish();

      private:

        struct m_State { bool array, compact, content; };

        std::ostringstream m_oss;
        std::vector<m_State> m_state;
        int m_indentation{0};
        bool m_isKey{false};
        bool m_isComplete{false};

        template <typename T> void m_val(T v);

        void m_prefix();

        void m_indent();

        void m_checkPre() const;

        void m_encode(std::string_view val);
        void m_encode(int64_t val);
        void m_encode(double val);
        void m_encode(bool val);
        void m_encode(nullptr_t);

    };

}

// IMPLEMENTATION //////////////////////////////////////////////////////////////////////////////////////////////////////

namespace qjson {

    using std::string;
    using std::string_view;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    inline Encoder::Encoder(Encoder && other) :
        m_oss(std::move(other.m_oss)),
        m_state(std::move(other.m_state)),
        m_indentation(other.m_indentation),
        m_isKey(other.m_isKey),
        m_isComplete(other.m_isComplete)
    {
        other.m_indentation = 0;
        other.m_isKey = false;
        other.m_isComplete = false;
    }

    inline Encoder & Encoder::object(bool compact) {
        m_checkPre();
        m_prefix();
        m_oss << '{';
        if (!m_state.empty()) {
            m_state.back().content = true;
            m_isKey = false;
            compact = compact || m_state.back().compact;
        }
        m_state.push_back(m_State{false, compact, false});

        return *this;
    }

    inline Encoder & Encoder::array(bool compact) {
        m_checkPre();
        m_prefix();
        m_oss << '[';
        if (!m_state.empty()) {
            m_state.back().content = true;
            m_isKey = false;
            compact = compact || m_state.back().compact;
        }
        m_state.push_back(m_State{true, compact, false});

        return *this;
    }

    inline Encoder & Encoder::key(string_view key) {
        if (m_isKey) {
            throw EncodeError("A key has already been given");
        }
        if (m_state.empty() || m_state.back().array) {
            throw EncodeError("A key may only be givin within an object");
        }
        if (key.empty()) {
            throw EncodeError("Key must not be empty");
        }

        m_prefix();
        m_encode(key);
        m_oss << ": "sv;
        m_isKey = true;

        return *this;
    }

    inline Encoder & Encoder::val(string_view v) {
        m_val(v);

        return *this;
    }

    inline Encoder & Encoder::val(const string & v) {
        return val(string_view(v));
    }

    inline Encoder & Encoder::val(const char * v) {
        return val(string_view(v));
    }

    inline Encoder & Encoder::val(char * v) {
        return val(string_view(v));
    }

    inline Encoder & Encoder::val(char v) {
        return val(string_view(&v, 1));
    }

    inline Encoder & Encoder::val(int64_t v) {
        m_val(v);

        return *this;
    }

    inline Encoder & Encoder::val(uint64_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(int32_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(uint32_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(int16_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(uint16_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(int8_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(uint8_t v) {
        return val(int64_t(v));
    }

    inline Encoder & Encoder::val(double v) {
        m_val(v);

        return *this;
    }

    inline Encoder & Encoder::val(float v) {
        return val(double(v));
    }

    inline Encoder & Encoder::val(bool v) {
        m_val(v);

        return *this;
    }

    inline Encoder & Encoder::val(nullptr_t) {
        m_val(nullptr);
        return *this;
    }

    template <typename T>
    inline Encoder & Encoder::val(const T & v) {
        ::qjson_encode(*this, v);
        return *this;
    }

    inline Encoder & Encoder::end() {
        if (m_state.empty()) {
            throw EncodeError("No object or array to end");
        }
        if (m_isKey) {
            throw EncodeError("Cannot end object with a dangling key");
        }

        const m_State & state(m_state.back());
        if (state.content) {
            if (state.compact) {
                m_oss << ' ';
            }
            else {
                m_oss << '\n';
                --m_indentation;
                m_indent();
            }
        }
        m_oss << (state.array ? ']' : '}');
        m_state.pop_back();

        if (m_state.empty()) {
            m_isComplete = true;
        }

        return *this;
    }

    inline string Encoder::finish() {
        if (!m_isComplete) {
            throw EncodeError("Cannot finish, JSON is not yet complete");
        }

        string str(m_oss.str());

        // Reset state
        m_oss.str(""s);
        m_oss.clear();
        m_isComplete = false;

        return str;
    }

    template <typename T>
    inline void Encoder::m_val(T t) {
        m_checkPre();
        m_prefix();
        m_encode(t);

        if (m_state.empty()) {
            m_isComplete = true;
        }
        else {
            m_state.back().content = true;
            m_isKey = false;
        }
    }

    inline void Encoder::m_prefix() {
        if (!m_isKey && !m_state.empty()) {
            const m_State & state(m_state.back());
            if (state.content) {
                m_oss << ',';
            }
            if (state.compact) {
                m_oss << ' ';
            }
            else {
                m_oss << '\n';
                m_indentation += !state.content;
                m_indent();
            }
        }
    }

    inline void Encoder::m_indent() {
        for (int i(0); i < m_indentation; ++i) {
            m_oss << "    "sv;
        }
    }

    inline void Encoder::m_checkPre() const {
        if (m_isComplete) {
            throw EncodeError("Cannot add value to complete JSON");
        }
        if (!m_isKey && !(m_state.empty() || m_state.back().array)) {
            throw EncodeError("Cannot add value to object without first providing a key");
        }
    }

    inline void Encoder::m_encode(string_view v) {
        static constexpr char s_hexChars[16]{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        m_oss << '"';

        for (unsigned char c : v) {
            if (std::isprint(c)) {
                if (c == '"' || c == '\\') m_oss << '\\';
                m_oss << c;
            }
            else {
                switch (c) {
                    case '\b': m_oss << R"(\b)"; break;
                    case '\f': m_oss << R"(\f)"; break;
                    case '\n': m_oss << R"(\n)"; break;
                    case '\r': m_oss << R"(\r)"; break;
                    case '\t': m_oss << R"(\t)"; break;
                    default:
                        if (c < 128) {
                            m_oss << R"(\u00)" << s_hexChars[(c >> 4) & 0xF] << s_hexChars[c & 0xF];
                        }
                        else {
                            throw EncodeError("Non-ASCII unicode is not supported");
                        }
                }
            }
        }

        m_oss << '"';
    }

    inline void Encoder::m_encode(int64_t v) {
        char buffer[24];

        std::to_chars_result res(std::to_chars(buffer, buffer + sizeof(buffer), v));

        m_oss << string_view(buffer, res.ptr - buffer);
    }

    inline void Encoder::m_encode(double v) {
        char buffer[32];

        std::to_chars_result res(std::to_chars(buffer, buffer + sizeof(buffer), v));
        int length(int(res.ptr - buffer));

        // std::to_chars doesn't suffix whole number results with a ".0", so we have to do that ourselves
        // Only if the result isn't "inf", "-inf", or "nan"
        if (std::isdigit(buffer[length - 1])) {
            bool needsDecimal(true);
            for (int i(1); i < length - 1; ++i) {
                if (buffer[i] == '.' || buffer[i] == 'e') {
                    needsDecimal = false;
                    break;
                }
            }
            if (needsDecimal) {
                buffer[length] = '.';
                buffer[length + 1] = '0';
                length += 2;
            }
        }

        m_oss << string_view(buffer, length);
    }

    inline void Encoder::m_encode(bool v) {
        m_oss << (v ? "true"sv : "false"sv);
    }

    inline void Encoder::m_encode(nullptr_t) {
        m_oss << "null"sv;
    }

}
