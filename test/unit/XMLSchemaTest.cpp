/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include "gtest/gtest.h"
#include <boost/property_tree/ptree.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <pdal/util/FileUtils.hpp>
#include <pdal/Metadata.hpp>
#include <pdal/PointContext.hpp>
#include <pdal/XMLSchema.hpp>

#include "Support.hpp"
#include "TestConfig.hpp"

#include <fstream>

using namespace pdal;

std::string ReadXML(std::string filename)
{
    std::istream* infile = FileUtils::openFile(filename);
    std::ifstream::pos_type size;
    std::vector<char> data;
    if (infile->good())
    {
        infile->seekg(0, std::ios::end);
        size = infile->tellg();
        data.resize(static_cast<std::vector<char>::size_type>(size));
        infile->seekg(0, std::ios::beg);
        infile->read(&data.front(), size);

        delete infile;
        return std::string(&data[0], data.size());
    }
    else
        throw pdal_error("unable to open file!");
}

TEST(XMLSchemaTest, read)
{
    auto getDim = [](XMLDimList& dims, const std::string& name)
    {
        for (auto di = dims.begin(); di != dims.end(); ++di)
        {
            if (di->m_name == name)
                return *di;
        }
        return XMLDim();
    };

    std::string xml =
        ReadXML(TestConfig::g_data_path+"../../schemas/6-dim-schema.xml");
    std::string xsd = ReadXML(TestConfig::g_data_path+"../../schemas/LAS.xsd");

    XMLSchema s(xml, xsd);
    XMLDimList dims = s.xmlDims();

    EXPECT_EQ(dims.size(), 6U);

    XMLDim dim = getDim(dims, "X");
    DimType dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name,  "X");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, .01);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Signed32);

    dim = getDim(dims, "Y");
    dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name, "Y");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, .01);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Signed32);
    
    dim = getDim(dims, "Z");
    dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name, "Z");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, .01);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Signed32);

    dim = getDim(dims, "Intensity");
    dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name, "Intensity");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, 1.0);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Unsigned16);

    dim = getDim(dims, "ReturnNumber");
    dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name, "ReturnNumber");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, 1.0);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Unsigned8);

    dim = getDim(dims, "NumberOfReturns");
    dt = dim.m_dimType;
    EXPECT_EQ(dim.m_name, "NumberOfReturns");
    EXPECT_FLOAT_EQ(dt.m_xform.m_scale, 1.0);
    EXPECT_FLOAT_EQ(dt.m_xform.m_offset, 0.0);
    EXPECT_EQ(dt.m_type, Dimension::Type::Unsigned8);
}


TEST(XMLSchemaTest, copy)
{
    using namespace pdal;

    std::string xml = ReadXML(TestConfig::g_data_path +
        "../../schemas/16-dim-schema.xml");
    std::string xsd = ReadXML(TestConfig::g_data_path+"../../schemas/LAS.xsd");

    XMLSchema s1(xml, xsd);

    PointContext ctx;
    XMLDimList dims = s1.xmlDims();
    for (auto di = dims.begin(); di != dims.end(); ++di)
    {
        Dimension::Id::Enum id =
            ctx.registerOrAssignDim(di->m_name, di->m_dimType.m_type);
        s1.setId(di->m_name, id);
    }

    MetadataNode m;

    MetadataNode m1 = m.add("m1", 1u);
    MetadataNode m2 = m.add("m2", 1);
    MetadataNode m1prime = m.add("m1prime", "Some other metadata");
    m1.add("uuid", boost::uuids::nil_uuid());

    XMLSchema s2(s1.dimTypes(), m);
    std::string xml_output = s2.xml();

    XMLSchema s3(xml_output, xsd);
    XMLDimList dims3 = s3.xmlDims();

    EXPECT_EQ(dims.size(), dims3.size());

    auto di1 = dims.begin();
    auto di3 = dims3.begin();
    while (di1 != dims.end() && di3 != dims3.end())
    {
        XMLDim& dim1 = *di1;
        XMLDim& dim3 = *di3;

        EXPECT_EQ(dim1.m_name, dim3.m_name);
        EXPECT_EQ(dim1.m_dimType.m_type, dim3.m_dimType.m_type);
        di1++;
        di3++;
    }
}

TEST(XMLSchemaTest, utf8)
{
    using namespace pdal;

    std::string inFilename(TestConfig::g_data_path +
        "../../schemas/utf8-schema.xml");
    std::string inXsdFilename(TestConfig::g_data_path +
        "../../schemas/LAS.xsd");
    std::string xml = ReadXML(inFilename);
    std::string xsd = ReadXML(inXsdFilename);

    XMLSchema s1(xml, xsd);

    std::string descripValue("Ég get etið gler án þess að meiða mig.");
    std::string metaName("אני יכול לאכול זכוכית וזה לא מזיק לי.");
    std::string metaValue("أنا قادر على أكل الزجاج و هذا لا يؤلمني");

    XMLDimList dims = s1.xmlDims();
    EXPECT_EQ(dims.size(), 1U);
    XMLDim& dim = *dims.begin();
    EXPECT_EQ(descripValue, dim.m_description);
    MetadataNodeList mlist = s1.getMetadata().children();
    EXPECT_EQ(mlist.size(), 1U);
    MetadataNode& m = *mlist.begin();
    EXPECT_EQ(m.name(), metaName);
    EXPECT_EQ(m.value(), metaValue);
}
