#pragma once

#include "absl/strings/string_view.h"

namespace Envoy {
namespace Json {
namespace TestUtil {

/**
 * Determines whether the input string can be serialized by protobufs. This is
 * used for testing, to avoid trying to do differentials against Protobuf JSON
 * sanitization, which produces noisy error messages and empty strings when
 * presented with some utf8 sequences that are valid according to spec.
 *
 * @param in the string to validate as utf-8.
 */
bool isProtoSerializableUtf8(absl::string_view in);

bool utf8Equivalent(absl::string_view a, absl::string_view b, std::string& errmsg);
#define EXPECT_UTF8_EQ(a, b, context)                                                              \
  {                                                                                                \
    std::string errmsg;                                                                            \
    EXPECT_TRUE(TestUtil::utf8Equivalent(a, b, errmsg)) << context << "\n" << errmsg;              \
  }

/**
 * Reverses the json escaping algorithm in sanitize(), which is used when the
 * utf-8 serialization fails. Note that `sanitized` may be in any encoding, e.g.
 * ascii, binary, utf-8, gb2132. `sanitized` is valid JSON with `\u` escapes for
 * any characters are not allowed in JSON strings, per
 * https://www.json.org/json-en.html. We want to make sure our json encoding for
 * `original` would be decoded into the same bytes.
 *
 * Writes the decoded version into `decoded`.
 *
 * Note that the `sanitized` argument does not accept arbitrary json string
 * encodings, such as `\r`, as those are not currently generated by the
 * exception handler in Json::sanitize(); only numeric escapes are generated
 * so that's all this test helper accepts.
 *
 * @param sanitized the output of Json::sanitize(), when it is not utf-8 compliant.
 * @param decoded the decoded form, undoing any escapes added by Json::sanitize().
 * @param errmsg details any error encountered during decoding.
 * @param true if the encoding was successful.
 */
bool decodeEscapedJson(absl::string_view sanitized, std::string& decoded, std::string& errmsg);
#define EXPECT_JSON_STREQ(sanitized, original, context)                                            \
  {                                                                                                \
    std::string decoded, errmsg;                                                                   \
    EXPECT_TRUE(TestUtil::decodeEscapedJson(sanitized, decoded, errmsg))                           \
        << context << ": " << errmsg;                                                              \
    EXPECT_EQ(decoded, original) << context;                                                       \
  }

} // namespace TestUtil
} // namespace Json
} // namespace Envoy