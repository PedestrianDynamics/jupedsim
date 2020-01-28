/**
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "IO/XmlUtils.h"

#include <catch2/catch.hpp>
#include <cmath>

TEST_CASE("IO/XmlUtils/XmlUtil/int", "[utils][XmlUtil][int]")
{
    SECTION("Valid")
    {
        {
            int input{0};
            std::string inputString{std::to_string(input)};

            int output{XmlUtil::StringToInt(inputString)};
            REQUIRE(input == output);
        }
        {
            int input{std::numeric_limits<int>::min()};
            std::string inputString{std::to_string(input)};

            int output{XmlUtil::StringToInt(inputString)};
            REQUIRE(input == output);
        }
        {
            int input{std::numeric_limits<int>::max()};
            std::string inputString{std::to_string(input)};

            int output{XmlUtil::StringToInt(inputString)};
            REQUIRE(input == output);
        }
    }
    SECTION("Invalid")
    {
        {
            std::string inputString{"invalid"};
            REQUIRE_THROWS(XmlUtil::StringToInt(inputString));
        }
        {
            std::string inputString{"123.123123"};
            REQUIRE_THROWS(XmlUtil::StringToInt(inputString));
        }
    }
}

TEST_CASE("IO/XmlUtils/Validator/int", "[utils][Validator][int]")
{
    SECTION("Valid")
    {
        {
            int input{0};

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(std::to_string(input));
            parent->LinkEndChild(child);

            int output = Validator<int>::GetValue(*child);
            REQUIRE(output == input);
            delete parent;
        }

        {
            int input{std::numeric_limits<int>::min()};

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(std::to_string(input));
            parent->LinkEndChild(child);

            int output = Validator<int>::GetValue(*child);
            REQUIRE(output == input);
            delete parent;
        }
        {
            int input{std::numeric_limits<int>::max()};

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(std::to_string(input));
            parent->LinkEndChild(child);

            int output = Validator<int>::GetValue(*child);
            REQUIRE(output == input);
            delete parent;
        }
    }
    SECTION("Invalid")
    {
        // some invalid input should throw exception with helpful text message
        // Remark: scientific notation not valid at the moment!
        {
            std::string input = "invalid";

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(input);
            parent->LinkEndChild(child);
            int output;
            REQUIRE_THROWS(output = Validator<int>::GetValue(*child));
            delete parent;
        }
        {
            std::string input = "167.1231";

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(input);
            parent->LinkEndChild(child);
            int output;
            REQUIRE_THROWS(output = Validator<int>::GetValue(*child));
            delete parent;
        }
        {
            long max          = std::numeric_limits<long>::max();
            std::string input = std::to_string(max);

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(input);
            parent->LinkEndChild(child);
            int output;
            REQUIRE_THROWS(output = Validator<int>::GetValue(*child));
        }
        {
            long min          = std::numeric_limits<long>::min();
            std::string input = std::to_string(min);

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(input);
            parent->LinkEndChild(child);
            int output;
            REQUIRE_THROWS(output = Validator<int>::GetValue(*child));
            delete parent;
        }
        {
            std::string input{"1e5"};

            auto * parent = new TiXmlElement("parent");
            auto * child  = new TiXmlElement("child");
            child->SetValue(input);
            parent->LinkEndChild(child);

            int output;
            REQUIRE_THROWS(output = Validator<int>::GetValue(*child));
            delete parent;
        }
    }
}