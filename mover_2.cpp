// mover_2.cpp: определяет точку входа для приложения.
//
#define BOOST_ASIO_HAS_CO_AWAIT

#include "mover_2.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/coroutine2/all.hpp>
#include <map>

namespace fs = std::filesystem;
using namespace std::chrono_literals;


boost::asio::awaitable<void> move_file(const fs::path& src, const fs::path& dst,bool only_move,bool is_remove)
{
        char buffer[26];
        std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ctime_s(buffer, sizeof(buffer), &now_time);
        
    try {
        fs::create_directories(dst.parent_path());
        if (only_move)
        {
            fs::rename(src, dst);
            std::cout << "[" << std::this_thread::get_id() << "]" << "[" << now_time << "]" << " Run" << std::endl;
            std::cout << "Moved: " << src << " -> " << dst << std::endl;
        }
        else
        {
            fs::copy_file(src, dst);
            std::cout << "[" << std::this_thread::get_id() << "]" << "[" << now_time << "]" << " Run" << std::endl;
            std::cout << "Coped: " << src << " -> " << dst << std::endl;
            if (is_remove)
            {
                fs::remove(src);
                std::cout << "[" << std::this_thread::get_id() << "]" << "[" << now_time << "]" << " Run" << std::endl;
                std::cout << "Removed: " << src << " -> " << dst << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error& e) {
       // std::cerr << "Error moving file " << src << " to " << dst << ": " << e.what() << std::endl;
    }
    co_return;
}
boost::asio::awaitable<void> file_balancer(boost::asio::io_context& io_context,const fs::path& src, const std::vector<fs::path>& dst) {
    //std::cout << "Starting long task...\n";
    //char buffer[26];
    //std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    //ctime_s(buffer, sizeof(buffer), &now_time);
    //std::cout << "[" << std::this_thread::get_id() << "]" <<"[" << now_time <<"]"<< " Run" << std::endl;
    for (const auto& entry : fs::recursive_directory_iterator(src)) {
        if (fs::is_regular_file(entry.path()))
        {
            for (auto d = dst.begin(); d != dst.end(); ++d)
            {
                fs::path target_path = *d / entry.path().filename();
                if (dst.size() == 1)
                {
                    co_spawn(io_context, move_file(entry.path(), target_path, true, false), boost::asio::detached);
                   // co_await  move_file(entry.path(), target_path, true, false);
                }
                else if (std::next(d) == dst.end()) {
                    co_spawn(io_context, move_file(entry.path(), target_path, true, true), boost::asio::detached);
                 //   co_await move_file(entry.path(), target_path, true, true);
                }
                else
                {
                    co_spawn(io_context, move_file(entry.path(), target_path, false, false), boost::asio::detached);
                    //co_await move_file(entry.path(), target_path, false, false);
                }
            }
        }
    }
   // timer.expires_after(std::chrono::seconds(3));
  //  std::cout << "Long task finished\n";
    co_return;
}
boost::asio::awaitable<void> monitor( boost::asio::io_context &io_context,  const std::map<fs::path, std::vector<fs::path>>& directory_pairs) {

    boost::asio::steady_timer timer(io_context);
    while (true)
    {
        
        for (const auto& dir : directory_pairs)
        {
            
            //co_await timer.async_wait(boost::asio::use_awaitable);
            co_spawn(io_context, file_balancer(io_context, dir.first, dir.second), boost::asio::detached);
           // co_await file_balancer(io_context, dir.first, dir.second);
        }
        timer.expires_after(std::chrono::seconds(3));
        co_await timer.async_wait(boost::asio::use_awaitable);
    }
    co_return;
}
int main() {
    boost::asio::io_context io_context;

   /* boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context.get_executor());
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_upr(io_context_upr.get_executor());*/

    std::vector<std::thread> threads;
    std::map<fs::path, std::vector<fs::path>> directory_pairs = {
        {"D:\\test\\in1", {"D:\\test\\out1","D:\\test\\out8","D:\\test\\out9"}},
        {"D:\\test\\in2", {"D:\\test\\out2","D:\\test\\out3","D:\\test\\out4","D:\\test\\out5","D:\\test\\out6","D:\\test\\out7"}}
        // Добавьте сюда остальные пары директорий
    };

     co_spawn(io_context, monitor( io_context,directory_pairs), boost::asio::detached);
     //io_context.run();

	 for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i) {
		 // if(i%2==0)
		 threads.emplace_back([&io_context] { io_context.run(); });
		 //else
		 //    threads.emplace_back([&io_context_upr] { io_context_upr.run(); });
	 }
	 //work_guard.reset();
	 //work_guard_upr.reset();
	 for (auto& thread : threads) {
		 if (thread.joinable()) {
			 thread.join();
		 }
	 }
     io_context.stop();
 

    return 0;
}
