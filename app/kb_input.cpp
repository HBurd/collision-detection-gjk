#include "kb_input.hpp"

KbInputHandler::KbInputHandler(GLFWwindow* window_)
    : window(window_)
{}

// Registers a key action.
// glfw_key_code is the key that triggers the action
// is_down_event is true if the event happens only as the key goes down, false if it always happens when the key is down
// action is the function that is called in these cases
void KbInputHandler::register_action(int glfw_key_code, bool is_down_event, std::function<void(GLFWwindow*)>&& action)
{
    actions.emplace_back(is_down_event, glfw_key_code, std::move(action));
}

// Executes every registered action, if the corresponding key is pressed or held
void KbInputHandler::do_actions()
{
    for (auto& action : actions)
    {
        if (glfwGetKey(window, action.glfw_key_code) == GLFW_PRESS)
        {
            if (!(action.is_down_event && action.is_held))
            {
                action.action(window);
            }

            action.is_held = true;
        }
        else
        {
            action.is_held = false;
        }
    }
}

// Returns a vector corresponding to the W, A, S, D, Q and E keys.
// W/S are -/+z, A/D are -/+x and Q/E are -/+y
demo::math::Vec3 KbInputHandler::get_wasdqe_vector()
{
    demo::math::Vec3 result;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 0.0f, -1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        result += demo::math::Vec3(-1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        result += demo::math::Vec3(1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, -1.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 1.0f, 0.0f);
    }
    return result;
}

// Returns a vector corresponding to the I, J, K, L, U and O keys.
// I/K are -/+z, J/L are -/+x and U/O are -/+y
demo::math::Vec3 KbInputHandler::get_ijkluo_vector()
{
    demo::math::Vec3 result;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 0.0f, -1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        result += demo::math::Vec3(-1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        result += demo::math::Vec3(1.0f, 0.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, -1.0f, 0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        result += demo::math::Vec3(0.0f, 1.0f, 0.0f);
    }
    return result;
}

KbInputHandler::RegisteredKey::RegisteredKey(bool is_down_event_, int key_code, std::function<void(GLFWwindow*)>&& action_)
    : is_down_event(is_down_event_), glfw_key_code(key_code), action(std::move(action_))
{}
