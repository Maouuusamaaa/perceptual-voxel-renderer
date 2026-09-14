#include "renderer/render_budget.hpp"
#include <algorithm>
namespace pvr {
RenderBudget::RenderBudget(double b,int w):budget_ms_(b),work_units_(w){}
double RenderBudget::frame_budget_ms() const noexcept{return budget_ms_;}
int RenderBudget::target_work_units() const noexcept{return work_units_;}
void RenderBudget::observe(double frame_ms){
    if(frame_ms > budget_ms_*1.05) work_units_=std::max(1, work_units_-10);
    else if(frame_ms < budget_ms_*0.80) work_units_=std::min(1000, work_units_+5);
}
}
