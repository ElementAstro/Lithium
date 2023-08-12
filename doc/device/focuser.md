bool is_moving; // 电调是否正在移动

int current_mode; // 电调当前模式
int current_motion;   // 电调当前运动状态
double current_speed; // 电调当前速度

int current_position; // 电调当前位置
int max_position; // 电调可移动的最大位置
int min_position; // 电调可移动的最小位置
int max_step; // 电调可移动的最大步长

bool can_get_temperature;   // 是否支持获取温度功能
double current_temperature; // 当前温度值

bool can_absolute_move; // 是否支持绝对移动功能
bool can_manual_move;   // 是否支持手动移动功能

int delay; // 电调延迟时间

bool has_backlash;