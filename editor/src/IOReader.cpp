#include <fstream>
#include <vector>
#include <string>

namespace robot2D {
    namespace io {
        using byte = uint8_t;

        enum class FileMode {
            text,
            binary
        };

        std::size_t fileSize(const std::string& path);

        /// load file as byte buffer
        bool loadFromFile(const std::string& path, const FileMode& filemode, std::vector<byte>& data);

        /// load file as string
        bool loadFromFile(const std::string& path, std::string& data);


        std::size_t fileSize(const std::string& path) {
            return 0;
        }

        /// load file as byte buffer
        bool loadFromFile(const std::string& path, const FileMode& filemode, std::vector<byte>& data) {
            return true;
        }

        /// load file as string
        bool loadFromFile(const std::string& path, std::string& data) {
            return true;
        }
    }
}

