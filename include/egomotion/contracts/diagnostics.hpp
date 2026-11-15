#pragma once

#include "egomotion/contracts/result.hpp"

#include <string>
#include <string_view>

namespace egomotion {

// JSONL diagnostics; no printf-style formatting and no non-finite JSON literals.
inline std::string jsonQuote(std::string_view input) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out{"\""};
    for (unsigned char c : input) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                out += "\\u00";
                out += digits[(c >> 4) & 0xf];
                out += digits[c & 0xf];
            } else {
                out.push_back(static_cast<char>(c));
            }
        }
    }
    out.push_back('"');
    return out;
}

inline void appendLocation(std::string& result, const std::string& path,
                           const std::string& variable,
                           const std::optional<std::size_t>& index,
                           const std::optional<RunId>& id) {
    if (!path.empty()) result += ",\"path\":" + jsonQuote(path);
    if (!variable.empty()) result += ",\"variable\":" + jsonQuote(variable);
    if (index) result += ",\"observation_index\":" + std::to_string(*index);
    if (id) result += ",\"run_id\":" + jsonQuote(id->text);
}

template<class Code>
std::string jsonFailure(const Failure<Code>& error, int exit_code) {
    std::string json = "{\"error_domain\":" + jsonQuote(errorDomain<Code>())
        + ",\"error_code\":" + jsonQuote(errorName(error.code))
        + ",\"stage\":" + jsonQuote(stageName(error.detail.stage))
        + ",\"exit_code\":" + std::to_string(exit_code)
        + ",\"message\":" + jsonQuote(error.detail.message);
    appendLocation(json, error.detail.source_path, error.detail.variable,
                   error.detail.observation_index, error.detail.run_id);
    json += ",\"causes\":[";
    bool first = true;
    for (const ErrorFrame& frame : error.detail.causes) {
        if (!first) json += ",";
        first = false;
        json += "{\"domain\":" + jsonQuote(frame.domain)
            + ",\"code\":" + jsonQuote(frame.code)
            + ",\"stage\":" + jsonQuote(stageName(frame.stage))
            + ",\"message\":" + jsonQuote(frame.message);
        appendLocation(json, frame.source_path, frame.variable,
                       frame.observation_index, frame.run_id);
        json += "}";
    }
    json += "]}";
    return json;
}

// Only an R0/R1 bootstrap diagnostic, NOT a newly approved public error enum.
// Once a concrete backend exists, failures use the real contracts enum.
inline std::string jsonBackendUnavailable(std::string_view stage,
                                          std::string_view message) {
    return "{\"error_domain\":\"Bootstrap\","
           "\"error_code\":\"BackendUnavailable\","
           "\"stage\":" + jsonQuote(stage)
           + ",\"exit_code\":5,\"message\":" + jsonQuote(message)
           + ",\"causes\":[]}";
}

}  // namespace egomotion
