/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ResultHandling.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <tinyxml.h>

std::unique_ptr<TiXmlDocument>
buildTestDoc(const std::vector<std::string>& path, const std::optional<std::string> text)
{
    std::unique_ptr<TiXmlDocument> doc = std::make_unique<TiXmlDocument>();
    doc->LinkEndChild(new TiXmlDeclaration("1.0", "", ""));

    TiXmlNode* node = doc.get();

    for(const auto& path_element : path) {
        auto* new_node = new TiXmlElement(path_element);
        node->LinkEndChild(new_node);
        node = new_node;
    }
    if(text) {
        node->LinkEndChild(new TiXmlText(*text));
    }
    return doc;
}

void assertTextValueAtPath(
    const TiXmlDocument& doc,
    const std::vector<std::string>& path,
    const std::string& value)
{
    const TiXmlNode* node = &doc;
    for(const auto& p : path) {
        node = node->FirstChild(p);
        ASSERT_NE(node, nullptr);
    }
    const auto* text = node->FirstChild();
    ASSERT_EQ(text->ValueStr(), value);
}

TEST(PatchPath, CanPatchSimplePath)
{
    auto doc = buildTestDoc({"a", "b"}, "/foo/bar/file.txt");
    detail::patchPath(*doc, {"a", "b"});
    assertTextValueAtPath(*doc, {"a", "b"}, "file.txt");
}

TEST(PatchPath, DoesNothingIfPathNotInDoc)
{
    auto doc = buildTestDoc({"a", "b"}, "/foo/bar/file.txt");
    detail::patchPath(*doc, {"a", "x"});
    assertTextValueAtPath(*doc, {"a", "b"}, "/foo/bar/file.txt");
}

TEST(PatchPath, DoesNotChangePathIfAlreadyJustAFileName)
{
    auto doc = buildTestDoc({"a", "b"}, "file.txt");
    detail::patchPath(*doc, {"a", "b"});
    assertTextValueAtPath(*doc, {"a", "b"}, "file.txt");
}

TEST(ExtractPath, CanExtract)
{
    const std::filesystem::path expected_path{"file.txt"};
    const auto doc = buildTestDoc({"a", "b"}, expected_path.string());
    const auto extracted_path = detail::extractPath(*doc, {"a", "b"});
    ASSERT_TRUE(extracted_path.has_value());
    ASSERT_EQ(expected_path, *extracted_path);
}

TEST(ExtractPath, ReturnNulloptOnInvalidPath)
{
    const auto doc = buildTestDoc({"a", "b"}, "file.txt");
    const auto extracted_path = detail::extractPath(
        *doc,
        {
            "x",
        });
    ASSERT_FALSE(extracted_path.has_value());
}

TEST(ExtractPath, ReturnNulloptOnMissingTextNode)
{
    const auto doc = buildTestDoc({"a", "b"}, std::nullopt);
    const auto extracted_path = detail::extractPath(
        *doc,
        {
            "x",
        });
    ASSERT_FALSE(extracted_path.has_value());
}

TEST(ExtractPath, ReturnNulloptIfTextIsEmptyString)
{
    const auto doc = buildTestDoc({"a", "b"}, "");
    const auto extracted_path = detail::extractPath(
        *doc,
        {
            "x",
        });
    ASSERT_FALSE(extracted_path.has_value());
}
