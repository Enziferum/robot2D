#include <tfd/tinyfiledialogs.h>
#include <editor/FiledialogAdapter.hpp>

namespace editor {

    bool FiledialogAdapter::openFile(std::string& path,
                                     std::string title,
                                     std::string defaultPath2Open,
                                     const std::vector<std::string>& filePatterns,
                                     std::string singleFilterDescription,
                                     bool allowMultiSelection) {

        if(m_libType == LibType::TinyFileDialog) {

            std::vector<const char*> openFilePatterns;
            for(int i = 0; i < filePatterns.size(); ++i)
                openFilePatterns.push_back(filePatterns[i].c_str());

            char* c_path = tinyfd_openFileDialog(title.c_str(),
                                                 defaultPath2Open.c_str(),
                                                 static_cast<int>(openFilePatterns.size()),
                                                 openFilePatterns.data(),
                                                 singleFilterDescription.c_str(),
                                                 static_cast<int>(allowMultiSelection));
            if(!c_path)
                return false;


            path = c_path;
            return true;
        }

        return false;
    }


    bool FiledialogAdapter::saveFile(std::string& path,
                                     const std::string& title,
                                     std::string defaultPath2Open,
                                     const std::vector<std::string>& filePatterns,
                                     std::string singleFilterDescription) {
        if(m_libType == LibType::TinyFileDialog) {
            std::vector<const char*> saveFilePatterns;
            for(int i = 0; i < filePatterns.size(); ++i)
                saveFilePatterns.push_back(filePatterns[i].c_str());

            auto result = tinyfd_saveFileDialog(title.c_str(),
                                                defaultPath2Open.c_str(),
                                                static_cast<int>(saveFilePatterns.size()),
                                                saveFilePatterns.data(),
                                                singleFilterDescription.c_str());
            if(!result)
                return false;

            path = result;
            return true;
        }

        return false;
    }


    bool FiledialogAdapter::selectFolder(std::string& path,
                                         const std::string& title,
                                         std::string defaultPath2Open) {
        if(m_libType == LibType::TinyFileDialog) {
            char* c_path = tinyfd_selectFolderDialog(title.c_str(),
                                                     defaultPath2Open.c_str());
            if(!c_path)
                return false;

            path = c_path;
            return true;
        }

        return false;
    }
}