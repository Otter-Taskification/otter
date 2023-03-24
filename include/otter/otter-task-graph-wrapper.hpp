#pragma once
#include <otter/otter-task-graph.h>
#include <stdio.h>

namespace otter {

    class Task {
    public:

        // default constructor: create a root task (i.e. with no parent)
        Task() {
            printf("*** DEFAULT-CONSTRUCT\n");
            m_task_context = otterTaskBegin(OTTER_SRC_ARGS(), nullptr);
        }

        // explicitly named wrapper for copy constructor
        static Task make_child_of(Task& parent) {
            return Task(parent);
        }

        // move constructor: move the task context handle from other.
        Task(Task&& other) {
            printf("*** MOVE-CONSTRUCT\n");
            m_task_context = other.m_task_context;
            other.m_task_context = nullptr;
        }

        ~Task(void) {
            end_task();
        }

        void end_task() {
            if (m_task_context != nullptr) {
                otterTaskEnd(m_task_context);
                m_task_context = nullptr;
            }
        }

        // it doesn't mean anything to copy-assign a task
        Task& operator=(Task& other) = delete;

        Task& operator=(Task&& other) {
            printf("+++ MOVE-ASSIGN\n");
            m_task_context = other.m_task_context;
            other.m_task_context = nullptr;
            return *this;
        }

    private:
        otter_task_context *m_task_context = nullptr;

        // copy constructor: create a copy as a child of `other`
        // This is private as it doesn't make sense for a user to copy-construct a task
        Task(Task& parent) {
            printf("*** COPY-CONSTRUCT\n");
            m_task_context = otterTaskBegin(OTTER_SRC_ARGS(), parent.m_task_context);
        }
    };

    class Otter {
    public:

        static Otter& get(void) {
            static Otter _handle;
            return _handle;
        }
        
        ~Otter() {
            delete m_root_task;
            otterTraceFinalise();
        };

        Otter(const Otter&) = delete;
        Otter(Otter&&) = delete;
        Otter& operator=(const Otter&) = delete;
        Otter& operator=(Otter&&) = delete;

        Task& get_root_task() {
            return *m_root_task;
        }

    private:
        Otter() { 
            otterTraceInitialise();
            m_root_task = new otter::Task();
        };
        Task* m_root_task;
    };

} // namespace otter
