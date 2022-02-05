#ifndef TINIFILE_H
#define TINIFILE_H


//====================================================================
// Filename: inifile.h
// Version:
// Author:        Doctor C
//
// Created at:    12:31 2016/01/02
// Modified at:
// Description:   class to provide ini file behaviour on linux platform
//====================================================================


#include <map>


typedef std::map<std::string, std::string> ini_section;
typedef std::map<std::string, ini_section> ini_file;

class TIniFile {
private:
    // Prevent copy and assigmnemt as well as default constructor
    TIniFile();
    TIniFile(const TIniFile& cp);
    TIniFile& operator=(const TIniFile& cp);

    const std::string m_filename;
    ini_file m_m_file_contents;
    std::string m_str_last_error;
    bool m_valid;
    bool m_dirty_flag;
public:
    TIniFile(const std::string& strfilename);
    ~TIniFile();

    // Access the value of a key from a particular section. If section-key does not exist,
    // the function returns the error string passed to the function as str_error
    std::string getValue(const std::string& section, const std::string& key, const std::string& str_error) const;
    bool sectionExists(const std::string& section_name) const;
    bool keyExists(const std::string& section_name, const std::string& key_name) const;
    ini_section getSectionData(const std::string& section_name) const;

    bool insertNewValue(const std::string& sec, const std::string& ky, const std::string& val);
    bool insertNewSection(const std::string& sec_name, const ini_section& new_section);
    bool updateValue(const std::string& sec, const std::string& ky, const std::string& new_val);
    bool removeValue(const std::string& sec, const std::string& ky);
    bool removeSection(const std::string& sec);


    std::string getLastErrorString() const {return m_str_last_error;};

    std::string getFileAsFormattedString() const;

    std::string getFileName() const {return m_filename;};

    bool saveToFile();
    bool isDirty() const {return m_dirty_flag;};
    bool isValid() const {return m_valid;};

};



#endif // TINIFILE_H
