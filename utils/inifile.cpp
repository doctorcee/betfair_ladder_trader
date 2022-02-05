//====================================================================
// Filename: inifile.cpp
// Version:
// Author:        Doctor C
//
// Created at:    12:31 2016/01/02
// Modified at:
// Description:   class to provide ini file behaviour on linux platform
//====================================================================


#include "inifile.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>

//=================================================================
struct removeWhitespace
{
    bool operator()(char c)
    {
        return (c == '\r' || c == '\n' || c == '\t' || c == ' ');
    }

};

struct removeBrackets
{
    bool operator()(char c)
    {
        return (c == '[' || c == ']');
    }

};

//=================================================================
template<typename Out>
void split(const std::string &s, char delim, Out result)
{

    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

//=================================================================
std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

//=================================================================
TIniFile::TIniFile(const std::string& strfilename)
    : m_filename(strfilename),
      m_str_last_error(""),
      m_valid(false),
      m_dirty_flag(false)
{
    m_m_file_contents.clear();
    // open and parse the file
    std::ifstream file_stream;
    file_stream.open(m_filename.c_str(), std::ios::in);
    std::string str_current_section = "";
    if (file_stream.is_open())
    {
        file_stream.seekg(0, std::ios::beg);    // go back to the beginning
        std::string line;		// string to read the next line into
        while(getline(file_stream, line))
        {
            // Strip all whitespace from this line
            line.erase(remove_if(line.begin(),line.end(), removeWhitespace()),line.end());
            // See if the current line is a comment
            if (!line.empty())
            {
                if (line[0] == ';')
                {
                    // comment - ignore
                }
                else if (line[0] == '[')
                {
                    // This is a section header - we should store this
                    str_current_section = line;
                    str_current_section.erase(remove_if(str_current_section.begin(),str_current_section.end(), removeBrackets()),str_current_section.end());
                    if (m_m_file_contents.find(str_current_section) == m_m_file_contents.end())
                    {
                        // new section
                        std::map<std::string, std::string> mTemp;
                        mTemp.clear();
                        m_m_file_contents[str_current_section] = mTemp;
                    }
                }
                else
                {
                    // See if we have decent data
                    std::vector<std::string> tagdata = split(line,'=');
                    if (tagdata.size() == 2)
                    {
                        // This looks to be a valid ini file entry
                        if (!tagdata.at(0).empty() && !tagdata.at(1).empty())
                        {
                            (m_m_file_contents[str_current_section])[tagdata.at(0)] = tagdata.at(1);
                        }
                    }
                    else
                    {
                        std::cout << "WARNING: Found invalid entry in TIniFile()" << std::endl;
                    }
                }
            }
        }
        file_stream.close();
        m_valid = true;
    }
    else
    {
        m_str_last_error = "Unable to open file: " + m_filename + "\n";
    }
}

//=================================================================
TIniFile::~TIniFile()
{

    m_m_file_contents.clear();
}

//=================================================================
std::string TIniFile::getValue(const std::string& section,
                                const std::string& key,
                                const std::string& str_error) const
{

    // Returns the same as str_error when no section-key combination exists
    // mimics TIniFile on Embarcadero

    std::string str_return = str_error;
    std::map<std::string , std::map< std::string, std::string> >::const_iterator itmap = m_m_file_contents.find(section);
    if (itmap != m_m_file_contents.end())
    {
        // section exists
        std::map<std::string,std::string>::const_iterator it = itmap->second.find(key);
        if (it != itmap->second.end())
        {
            // key exists within this section.
            str_return =  it->second;
        }
    }
    return str_return;
}

//=================================================================
bool TIniFile::sectionExists(const std::string& section_name) const
{
    return (m_m_file_contents.find(section_name) != m_m_file_contents.end());
}

//=================================================================
bool TIniFile::keyExists(const std::string& section_name, const std::string& key_name) const
{
    bool b_exists = false;
    ini_file::const_iterator itmap = m_m_file_contents.find(section_name);
    if (itmap != m_m_file_contents.end())
    {
        // section exists
        ini_section::const_iterator it = itmap->second.find(key_name);
        b_exists = (it != itmap->second.end());
    }
    return b_exists;
}

//=================================================================
ini_section TIniFile::getSectionData(const std::string& section_name) const
{
    ini_section ret_val;
    ini_file::const_iterator itmap = m_m_file_contents.find(section_name);
    if (itmap != m_m_file_contents.end())
    {
        // section exists
        ret_val = itmap->second;
    }
    return ret_val;
}

//=================================================================
bool TIniFile::insertNewValue(const std::string& section_name,
                                    const std::string& key_name,
                                    const std::string& val)
{
    // If SECTION does not exist, we do nothing!
    bool b_success = false;
    if ((val.empty() == false) && (key_name.empty() == false))
    {
        ini_file::iterator itmap = m_m_file_contents.find(section_name);
        if (itmap != m_m_file_contents.end())
        {
            // section name exists
            // we just overwrite if key is present, or add new entry if not
            (itmap->second)[key_name] = val;
            b_success = true;
            m_dirty_flag = true;
        }
    }
    return b_success;
}

//=================================================================
bool TIniFile::insertNewSection(const std::string& sec_name,
                                     const ini_section& new_section)
{
    // Note this will NOT replace the section, will simply add
    // if the section name is not already present
    bool b_success = false;
    if (sec_name.empty() == false)
    {
        if (false == sectionExists(sec_name))
        {
            // safe to add as the section name does not exist
            m_m_file_contents[sec_name] = new_section;
            m_dirty_flag = true;
            b_success = true;
        }
    }
    return b_success;
}

//=================================================================
bool TIniFile::updateValue(const std::string& section_name,
                                const std::string& key_name,
                                const std::string& new_val)
{
    // This is JUST update - we do not insert value if key is NOT already
    // present
    bool b_success = false;
    if (new_val.empty() == false)
    {
        ini_file::iterator itmap = m_m_file_contents.find(section_name);
        if (itmap != m_m_file_contents.end())
        {
            // section name exists
            ini_section::iterator it = itmap->second.find(key_name);
            if (it != itmap->second.end())
            {
                // key name exists - now just update the value!
                it->second = new_val;
                b_success = true;
                m_dirty_flag = true;
            }
        }
    }
    return b_success;
}

//=================================================================
bool TIniFile::removeValue(const std::string& sec, const std::string& ky)
{
    bool b_success = false;
    if (ky.empty() == false)
    {
        ini_file::iterator itmap = m_m_file_contents.find(sec);
        if (itmap != m_m_file_contents.end())
        {
            // section name exists
            ini_section::iterator it = itmap->second.find(ky);
            if (it != itmap->second.end())
            {
                // key name exists - now just erase
                (itmap->second).erase(it);
                b_success = true;
                m_dirty_flag = true;
            }
        }
    }
    return b_success;
}

//=================================================================
bool TIniFile::removeSection(const std::string& sec)
{
    bool b_success = false;

    ini_file::iterator itmap = m_m_file_contents.find(sec);
    if (itmap != m_m_file_contents.end())
    {
        // section name exists - just erase
        m_m_file_contents.erase(itmap);
        b_success = true;
        m_dirty_flag = true;
    }
    return b_success;
}

//=================================================================
std::string TIniFile::getFileAsFormattedString() const
{
    // Debugging info only.
    std::ostringstream ost;
    std::map<std::string , std::map<std::string,std::string> >::const_iterator itSec;
    std::map<std::string,std::string>::const_iterator itKeys;
    for (itSec = m_m_file_contents.begin(); itSec != m_m_file_contents.end(); itSec++)
    {
        ost << itSec->first << "\n";
        for (itKeys=itSec->second.begin(); itKeys!=itSec->second.end(); itKeys++)
        {
            ost << "\t" << itKeys->first << " = " << itKeys->second << "\n";
        }
    }
    std::string str_ret = ost.str();
    return str_ret;
}


