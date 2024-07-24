// mover_2.cpp: определяет точку входа для приложения.
//

#include "mover_2.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <boost/asio.hpp>

namespace fs = std::filesystem;

void move_file(const fs::path& src, const fs::path& dest) {
    try {
        fs::create_directories(dest.parent_path());
        fs::rename(src, dest);
        std::cout << "Moved: " << src << " -> " << dest << std::endl;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error moving file " << src << " to " << dest << ": " << e.what() << std::endl;
    }
}

void worker(boost::asio::io_context& io_context) {
    io_context.run();
}

void check_and_move_files(const fs::path& source_dir, const fs::path& target_dir, boost::asio::io_context& io_context) {
    for (const auto& entry : fs::directory_iterator(source_dir)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path target_path = target_dir / entry.path().filename();
            io_context.post([src = entry.path(), dest = target_path]{
                std::async(std::launch::async, move_file, src, dest);
                });
        }
    }
}

void periodic_check(const fs::path& source_dir, const fs::path& target_dir, boost::asio::io_context& io_context, boost::asio::steady_timer& timer) {
    check_and_move_files(source_dir, target_dir, io_context);
    timer.expires_after(std::chrono::seconds(10));
    timer.async_wait([&](const boost::system::error_code& ec) {
        if (!ec) {
            periodic_check(source_dir, target_dir, io_context, timer);
        }
        });
}

void monitor_directories(const std::vector<std::pair<fs::path, fs::path>>& directory_pairs, boost::asio::io_context& io_context) {
    for (const auto& [source_dir, target_dir] : directory_pairs) {
        auto timer = std::make_shared<boost::asio::steady_timer>(io_context, std::chrono::seconds(10));
        periodic_check(source_dir, target_dir, io_context, *timer);
    }
}

int main() {
    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context.get_executor());

    std::vector<std::thread> threads;
    for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(worker, std::ref(io_context));
    }

    std::vector<std::pair<fs::path, fs::path>> directory_pairs = {
        {"source1", "target1"},
        {"source2", "target2"},
        {"source3", "target3"}
        // Добавьте сюда остальные пары директорий
    };

    monitor_directories(directory_pairs, io_context);

    work_guard.reset();
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "File transfer monitoring stopped!" << std::endl;

    return 0;
}
