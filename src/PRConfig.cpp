/*
 * The MIT License
 * Copyright (c) 2009 Gerry Stellenberg, Adam Preble
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 *  PRConfig.cpp
 *  libpinproc
 *
 */


#include "PRDevice.h"

#include <fstream>
#include "yaml.h"

PRResult PRDevice::LoadDefaultsFromYAML(const char *yamlFilePath)
{
    try
    {
        std::ifstream fin(yamlFilePath);
        if (fin.is_open() == false)
        {
            DEBUG(PRLog("YAML file not found: %s\n", yamlFilePath));
            return kPRFailure;
        }
        YAML::Parser parser(fin);
        
        while(parser) {
            YAML::Node doc;
            parser.GetNextDocument(doc);
            
            try
            {
                const YAML::Node& node = doc["PRGameName"];
                std::string name;  node >> name;
                DEBUG(PRLog("  Game Name: %s\n", name.c_str()));
            } 
            catch (YAML::KeyNotFound& ex) {}
            
            try
            {
                const YAML::Node& dict = doc["PRSwitches"];
                for(YAML::Iterator dictIt = dict.begin(); dictIt != dict.end(); ++dictIt) 
                {
                    std::string key;
                    std::string name;
                    dictIt.first() >> key;
                    dictIt.second() >> name;
                    DEBUG(PRLog("  Switch: %s -> %s\n", key.c_str(), name.c_str()));
                }
            } 
            catch (YAML::KeyNotFound& ex) {}
            
            try
            {
                const YAML::Node& dict = doc["PRDrivers"];
                for(YAML::Iterator dictIt = dict.begin(); dictIt != dict.end(); ++dictIt) 
                {
                    std::string key;
                    std::string name;
                    dictIt.first() >> key;
                    dictIt.second() >> name;
                    DEBUG(PRLog("  Driver: %s -> %s\n", key.c_str(), name.c_str()));
                }
            } 
            catch (YAML::KeyNotFound& ex) {}
            
        }
    }
    catch (YAML::ParserException& ex)
    {
        DEBUG(PRLog("YAML parse error at line=%d col=%d: %s\n", ex.line, ex.column, ex.msg.c_str()));
        return kPRFailure;
    }
    catch (YAML::RepresentationException& ex)
    {
        DEBUG(PRLog("YAML representation error at line=%d col=%d: %s\n", ex.line, ex.column, ex.msg.c_str()));
        return kPRFailure;
    }
    catch (...)
    {
        DEBUG(PRLog("Unexpected exception while parsing YAML config.\n"));
        return kPRFailure;
    }
    return kPRSuccess;
}
