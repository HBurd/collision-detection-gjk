#ifndef KB_INPUT_HANDLER_H
#define KB_INPUT_HANDLER_H

#include <vector>
#include <functional>
#include <GLFW/glfw3.h>
#include "math.hpp"

// This is a wrapper for the GLFW events relevant to the demo application
class InputHandler
{
public:
    InputHandler(GLFWwindow* window_);

    // Registers a key action.
    // glfw_key_code is the key that triggers the action
    // is_down_event is true if the event happens only as the key goes down, false if it always happens when the key is down
    // action is the function that is called in these cases
    void register_action(int glfw_key_code, bool is_down_event, std::function<void()>&& action);

    // Executes every registered action, if the corresponding key is pressed or held
    void do_actions();

    void poll_events();

    // Returns a vector corresponding to the W, A, S, D, Q and E keys.
    // W/S are -/+z, A/D are -/+x and Q/E are -/+y
    demo::math::Vec3 get_wasdqe_vector() const;

    // Returns a vector corresponding to the I, J, K, L, U and O keys.
    // I/K are -/+z, J/L are -/+x and U/O are -/+y
    demo::math::Vec3 get_ijkluo_vector() const;

    // Returns true iff left shift is held
    bool get_shift() const;

    // Returns true iff the window should close
    bool window_should_close() const;

private:
    struct RegisteredKey
    {
        bool is_down_event;
        bool is_held = false;

        int glfw_key_code;
        std::function<void()> action;

        RegisteredKey(bool is_down_event_, int key_code, std::function<void()>&& action_);
    };

    GLFWwindow* window;
    std::vector<RegisteredKey> actions;
};

#endif
