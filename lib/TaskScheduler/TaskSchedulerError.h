#ifndef TASKSCHEDULER_TASKSCHEDULERERROR_H
#define TASKSCHEDULER_TASKSCHEDULERERROR_H

#include <string>

class TaskSchedulerError : public std::exception {
private:
    std::string m_message;
protected:
    void setMessage(std::string message) {
        m_message = std::move(message);
    }

public:
    explicit TaskSchedulerError(std::string message) : m_message(std::move(message)) {
    }

    [[nodiscard]] const char *what() const noexcept override {
        return m_message.c_str();
    }
};

#endif //TASKSCHEDULER_TASKSCHEDULERERROR_H
