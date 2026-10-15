
#include "hmq/message_log.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{

    const auto test_path =
        std::filesystem::temp_directory_path() / "hmq_message_log_test.bin";

    void clean_up()
    {
        std::error_code error;
        std::filesystem::remove(test_path, error);
    }

    void test_append_and_read()
    {
        clean_up();

        {
            MessageLog log(test_path);

            log.append("Order A");
            log.append("Order B");
            log.append("Order C");
        }

        // Reopen the log to verify that data remains on disk.
        {
            MessageLog log(test_path);
            const auto messages = log.read_all();

            assert(messages.size() == 3);

            assert(messages[0].offset == 0);
            assert(messages[0].content == "Order A");

            assert(messages[1].offset == 1);
            assert(messages[1].content == "Order B");

            assert(messages[2].offset == 2);
            assert(messages[2].content == "Order C");
        }

        clean_up();
    }

    void test_empty_file()
    {
        clean_up();

        {
            std::ofstream file(test_path, std::ios::binary);
            assert(file.is_open());
        }

        MessageLog log(test_path);
        assert(log.read_all().empty());

        clean_up();
    }

    void test_incomplete_header()
    {
        clean_up();

        {
            std::ofstream file(test_path, std::ios::binary);
            assert(file.is_open());

            // A valid header needs four bytes. Write only two.
            const char incomplete[] = {0x05, 0x00};
            file.write(incomplete, sizeof(incomplete));
        }

        MessageLog log(test_path);

        bool threw = false;

        try
        {
            (void)log.read_all();
        }
        catch (const std::runtime_error &)
        {
            threw = true;
        }

        assert(threw);
        clean_up();
    }

    void test_incomplete_content()
    {
        clean_up();

        {
            std::ofstream file(test_path, std::ios::binary);
            assert(file.is_open());

            // Header says 5 bytes, but only 2 content bytes follow.
            const char record[] = {
                0x05, 0x00, 0x00, 0x00,
                'H', 'i'};

            file.write(record, sizeof(record));
        }

        MessageLog log(test_path);

        bool threw = false;

        try
        {
            (void)log.read_all();
        }
        catch (const std::runtime_error &)
        {
            threw = true;
        }

        assert(threw);
        clean_up();
    }

    void test_empty_message()
    {
        clean_up();

        MessageLog log(test_path);
        log.append("");

        const auto messages = log.read_all();

        assert(messages.size() == 1);
        assert(messages[0].offset == 0);
        assert(messages[0].content.empty());

        clean_up();
    }

} // namespace

int main()
{
    test_append_and_read();
    test_empty_file();
    test_incomplete_header();
    test_incomplete_content();
    test_empty_message();

    clean_up();

    std::cout << "All MessageLog tests passed\n";
    return 0;
}
