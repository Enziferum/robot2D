#pragma once
#include <editor/Task.hpp>
#include <editor/ExportOptions.hpp>

namespace editor {
    class ExportTask: public ITask {
    public:
        ExportTask(ITaskFunction::Ptr function, const ExportOptions& exportOptions);
        ~ExportTask() override = default;


        void execute() override;
    private:
        const ExportOptions& m_exportOptions;
    };
}