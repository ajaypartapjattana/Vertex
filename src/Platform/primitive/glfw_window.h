#pragma once

#include <utility>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace plf {

    class window {
    private:
        GLFWwindow* m_window = nullptr;

    public:
        window() = default;
        window(uint32_t width, uint32_t height, const char* title = nullptr) {
            int result = create(width, height, title);
            if (result == 0)
                throw std::runtime_error("FAILURE: window_creation!");
        }
        window(const window&) = delete;
        window(window&& other) noexcept 
            :
            m_window(std::exchange(other.m_window, nullptr))
        {

        }
        
        window& operator=(const window&) = delete;
        window& operator=(window&& other) noexcept {
            if (this == &other)
                return *this;

            reset();

            m_window = std::exchange(other.m_window, nullptr);

            return *this;
        }

        ~window() noexcept {
            reset();
        }

        operator GLFWwindow* () const noexcept {
            return m_window;
        }

        int create(uint32_t width, uint32_t height, const char* title = nullptr) noexcept;
        void reset() noexcept;

    };

}
