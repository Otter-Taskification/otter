#include "api/otter-task-graph/otter-task-graph-wrapper.hpp"
#include <stdio.h>

using namespace otter;

Task::Task() : Task(nullptr) {}

Task::Task(int flavour) : Task(nullptr, flavour) {}

Task Task::make_child() { return Task(m_task_context); }

Task Task::make_child(int flavour) { return Task(m_task_context, flavour); }

Task::Task(Task &&other) : m_task_context{other.m_task_context} {
  printf(">>> MOVE CONSTRUCTOR <<<\n");
  other.m_task_context = nullptr;
}

void Task::synchronise_tasks(otter_task_sync_t mode) {
  otterSynchroniseTasks(m_task_context, mode);
}

void Task::end_task() {
  if (m_task_context != nullptr) {
    otterTaskEnd(m_task_context);
    m_task_context = nullptr;
  }
}

Task::~Task() { end_task(); }

Task::Task(otter_task_context *parent, int flavour)
    : m_task_context{
          otterTaskBegin_flavour(OTTER_SRC_ARGS(), parent, flavour)} {}

Otter::Otter(void) : m_finalised{false} {
  otterTraceInitialise();
  m_root_task = new otter::Task();
};

void Otter::close(void) {
  if (!m_finalised) {
    delete m_root_task;
    otterTraceFinalise();
    m_finalised = true;
  }
}

Otter::~Otter(void) { this->close(); }

Otter &Otter::get_otter(void) {
  static Otter _handle;
  return _handle;
}

Task &Otter::get_root_task(void) { return *m_root_task; }
