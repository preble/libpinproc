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
            
            for(YAML::Iterator it=doc.begin();it!=doc.end();++it) {
                std::string key;
                it.first() >> key;
                DEBUG(PRLog("Parsing key %s...\n", key.c_str()));
                if (key.compare("PRGameName") == 0)
                {
                    std::string name;
                    it.second() >> name;
                    DEBUG(PRLog("  Machine name: %s\n", name.c_str()));
                }
                else if (key.compare("PRDriverGlobalConfig") == 0)
                {
                    //const YAML::Node& dict = it.second();
                }
                else if (key.compare("PRDriverGroupConfigs") == 0)
                {
                    const YAML::Node& groups = it.second();
                    for(YAML::Iterator it=groups.begin();it!=groups.end();++it) 
                    {
                        std::string key;
                        it.first() >> key;
                    }
                }
                else if (key.compare("PRDrivers") == 0)
                {
                    const YAML::Node& dict = it.second();
                    for(YAML::Iterator dictIt=dict.begin(); dictIt != dict.end(); ++dictIt) 
                    {
                        std::string driverKey;
                        std::string driverName;
                        dictIt.first() >> driverKey;
                        dictIt.second() >> driverName;
                        DEBUG(PRLog("  Driver: %s -> %s\n", driverKey.c_str(), driverName.c_str()));
                    }
                }
                else if (key.compare("PRSwitches") == 0)
                {
                    const YAML::Node& dict = it.second();
                    for(YAML::Iterator dictIt=dict.begin(); dictIt != dict.end(); ++dictIt) 
                    {
                        std::string driverKey;
                        std::string driverName;
                        dictIt.first() >> driverKey;
                        dictIt.second() >> driverName;
                        DEBUG(PRLog("  Switch: %s -> %s\n", driverKey.c_str(), driverName.c_str()));
                    }
                }
            }
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
