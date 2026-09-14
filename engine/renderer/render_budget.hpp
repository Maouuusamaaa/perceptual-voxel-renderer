#pragma once
namespace pvr {
class RenderBudget {
public:
    explicit RenderBudget(double frame_budget_ms=16.67, int initial_work_units=100);
    double frame_budget_ms() const noexcept;
    int target_work_units() const noexcept;
    void observe(double frame_ms);
private:
    double budget_ms_;
    int work_units_;
};
}
