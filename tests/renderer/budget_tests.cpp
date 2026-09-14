#include "renderer/render_budget.hpp"
#include <cassert>
#include <iostream>
int main() {
    pvr::RenderBudget budget(16.67);
    assert(budget.frame_budget_ms() == 16.67);
    budget.observe(20.0);
    assert(budget.target_work_units() < 100);
    const auto before = budget.target_work_units();
    budget.observe(10.0);
    assert(budget.target_work_units() >= before);
    std::cout << "budget_tests: PASS\n";
}
