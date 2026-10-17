#include <gtest/gtest.h>
#include <robot2D/Core/FileInputStream.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#endif

namespace {
    class FileInputStreamTest : public ::testing::Test {
    protected:
        void SetUp() override {
            static std::atomic<unsigned> next{0};
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            directory = std::filesystem::temp_directory_path() /
                ("robot2d-stream-" + std::to_string(stamp) + "-" + std::to_string(next++));
            ASSERT_TRUE(std::filesystem::create_directory(directory));
            file = directory / "data.bin";
            std::ofstream output(file, std::ios::binary);
            output.write("a\0bcd", 5);
            ASSERT_TRUE(output.good());
        }

        void TearDown() override {
            std::error_code error;
            std::filesystem::remove_all(directory, error);
            EXPECT_FALSE(error) << error.message();
        }

        std::filesystem::path directory;
        std::filesystem::path file;
    };
}

TEST(FileInputStream, ClosedOperationsReturnError) {
    robot2D::FileInputStream stream;
    char byte = 'x';
    EXPECT_EQ(stream.read(&byte, 1), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.read(nullptr, 0), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.tell(), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.seek(0), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.getSize(), robot2D::FileInputStream::Error);
    EXPECT_EQ(byte, 'x');
}

TEST_F(FileInputStreamTest, BinaryReadsDistinguishShortReadAndEof) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    char bytes[8]{};
    EXPECT_EQ(stream.read(bytes, 2), 2);
    EXPECT_EQ(std::string(bytes, 2), std::string("a\0", 2));
    EXPECT_EQ(stream.tell(), 2);
    EXPECT_EQ(stream.read(bytes, sizeof(bytes)), 3);
    EXPECT_EQ(std::string(bytes, 3), "bcd");
    EXPECT_EQ(stream.tell(), 5);
    EXPECT_EQ(stream.read(bytes, 1), 0);
    EXPECT_EQ(stream.seek(1), 1);
    EXPECT_EQ(stream.read(bytes, 1), 1);
    EXPECT_EQ(bytes[0], '\0');
}

TEST_F(FileInputStreamTest, InvalidBuffersDoNotAdvance) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    EXPECT_EQ(stream.read(nullptr, 1), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.read(nullptr, 0), 0);
    EXPECT_EQ(stream.tell(), 0);
    if constexpr(sizeof(std::size_t) < sizeof(std::uint64_t)) {
        char byte{};
        EXPECT_EQ(stream.read(&byte, std::numeric_limits<std::uint64_t>::max()),
            robot2D::FileInputStream::Error);
        EXPECT_EQ(stream.tell(), 0);
    }
}

TEST_F(FileInputStreamTest, SizePreservesPositionBeforeAndAfterEof) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    EXPECT_EQ(stream.getSize(), 5);
    ASSERT_EQ(stream.seek(2), 2);
    EXPECT_EQ(stream.getSize(), 5);
    EXPECT_EQ(stream.tell(), 2);
    char bytes[8]{};
    ASSERT_EQ(stream.read(bytes, sizeof(bytes)), 3);
    EXPECT_EQ(stream.getSize(), 5);
    EXPECT_EQ(stream.tell(), 5);
    EXPECT_EQ(stream.read(bytes, 1), 0);
}

TEST_F(FileInputStreamTest, EmptyFileHasSizeAndEofZero) {
    const auto empty = directory / "empty.bin";
    std::ofstream(empty, std::ios::binary).close();
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(empty.u8string()));
    EXPECT_EQ(stream.tell(), 0);
    EXPECT_EQ(stream.getSize(), 0);
    char byte = 'x';
    EXPECT_EQ(stream.read(&byte, 1), 0);
    EXPECT_EQ(byte, 'x');
}

TEST_F(FileInputStreamTest, ReopenResetsCursorAndReplacesFile) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    ASSERT_EQ(stream.seek(3), 3);
    const auto replacement = directory / "second.bin";
    std::ofstream(replacement, std::ios::binary) << "new";
    ASSERT_TRUE(stream.openFromFile(replacement.u8string()));
    EXPECT_EQ(stream.tell(), 0);
    EXPECT_EQ(stream.getSize(), 3);
    char bytes[3]{};
    EXPECT_EQ(stream.read(bytes, sizeof(bytes)), 3);
    EXPECT_EQ(std::string(bytes, 3), "new");
}

TEST_F(FileInputStreamTest, FailedReopenClosesPreviousAndAllowsRetry) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    EXPECT_FALSE(stream.openFromFile((directory / "missing").u8string()));
    EXPECT_EQ(stream.getSize(), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.tell(), robot2D::FileInputStream::Error);
    EXPECT_TRUE(stream.openFromFile(file.u8string()));
    EXPECT_EQ(stream.tell(), 0);
}

TEST_F(FileInputStreamTest, RejectsEmptyAndEmbeddedNullPaths) {
    robot2D::FileInputStream stream;
    EXPECT_FALSE(stream.openFromFile(""));
    EXPECT_FALSE(stream.openFromFile(file.u8string() + std::string("\0ignored", 8)));
    EXPECT_EQ(stream.tell(), robot2D::FileInputStream::Error);
}

TEST_F(FileInputStreamTest, OpensUtf8DirectoryAndFileNames) {
    const auto folder = directory / std::filesystem::u8path(u8"\u0440\u0435\u0441\u0443\u0440\u0441\u044b");
    ASSERT_TRUE(std::filesystem::create_directory(folder));
    const auto unicode = folder / std::filesystem::u8path(u8"\u0444\u0430\u0439\u043b-\u4e2d.bin");
    std::ofstream(unicode, std::ios::binary) << "utf8";
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(unicode.u8string()));
    char bytes[4]{};
    EXPECT_EQ(stream.read(bytes, sizeof(bytes)), 4);
    EXPECT_EQ(std::string(bytes, 4), "utf8");
}

TEST_F(FileInputStreamTest, LargeSeekDoesNotExtendFileAndSizeRestoresLargePosition) {
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    constexpr std::uint64_t offset = (std::uint64_t{1} << 32) + 17;
    ASSERT_EQ(stream.seek(offset), offset);
    EXPECT_EQ(stream.tell(), offset);
    EXPECT_EQ(stream.getSize(), 5);
    EXPECT_EQ(stream.tell(), offset);
    char byte{};
    EXPECT_EQ(stream.read(&byte, 1), 0);
    EXPECT_EQ(stream.seek(robot2D::FileInputStream::Error), robot2D::FileInputStream::Error);
    EXPECT_EQ(stream.tell(), offset);
    EXPECT_EQ(stream.seek(0), 0);
    EXPECT_EQ(stream.read(&byte, 1), 1);
    EXPECT_EQ(byte, 'a');
}

TEST_F(FileInputStreamTest, LargeSparseFileSizeAndTailRead) {
    constexpr std::uint64_t offset = (std::uint64_t{1} << 32) + 17;
    const auto large = directory / "sparse.bin";
#ifdef _WIN32
    std::unique_ptr<std::FILE, decltype(& std::fclose)> output(_wfopen(large.c_str(), L"wb"), &std::fclose);
    ASSERT_NE(output, nullptr);
    const auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(output.get())));
    DWORD returned = 0;
    ASSERT_TRUE(DeviceIoControl(handle, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &returned, nullptr));
    ASSERT_EQ(_fseeki64(output.get(), static_cast<__int64>(offset), SEEK_SET), 0);
    ASSERT_NE(std::fputc('z', output.get()), EOF);
    output.reset();
#else
    std::ofstream output(large, std::ios::binary);
    output.seekp(static_cast<std::streamoff>(offset));
    output.put('z');
    output.close();
    ASSERT_FALSE(output.fail());
#endif
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(large.u8string()));
    EXPECT_EQ(stream.getSize(), offset + 1);
    EXPECT_EQ(stream.tell(), 0);
    ASSERT_EQ(stream.seek(offset), offset);
    char byte{};
    ASSERT_EQ(stream.read(&byte, 1), 1);
    EXPECT_EQ(byte, 'z');
    EXPECT_EQ(stream.read(&byte, 1), 0);
}

#ifdef _WIN32
TEST_F(FileInputStreamTest, InvalidUtf8PathIsRejected) {
    robot2D::FileInputStream stream;
    EXPECT_FALSE(stream.openFromFile(directory.u8string() + "/\xff.bin"));
    EXPECT_EQ(stream.tell(), robot2D::FileInputStream::Error);
}

TEST_F(FileInputStreamTest, ReadIoFailureIsNotEofAndSeekAllowsRetry) {
    std::unique_ptr<void, decltype(& CloseHandle)> handle(
        CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr), &CloseHandle);
    ASSERT_NE(handle.get(), INVALID_HANDLE_VALUE);
    OVERLAPPED lock{};
    ASSERT_TRUE(LockFileEx(handle.get(), LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
        0, 1, 0, &lock));
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(file.u8string()));
    char byte{};
    EXPECT_EQ(stream.read(&byte, 1), robot2D::FileInputStream::Error);
    ASSERT_TRUE(UnlockFileEx(handle.get(), 0, 1, 0, &lock));
    ASSERT_EQ(stream.seek(0), 0);
    EXPECT_EQ(stream.read(&byte, 1), 1);
    EXPECT_EQ(byte, 'a');
}
#else
TEST_F(FileInputStreamTest, ReadIoFailureIsNotEof) {
    // POSIX permits fopen on a directory, but fread fails with EISDIR.
    robot2D::FileInputStream stream;
    ASSERT_TRUE(stream.openFromFile(directory.u8string()));
    char byte{};
    EXPECT_EQ(stream.read(&byte, 1), robot2D::FileInputStream::Error);
}
#endif
