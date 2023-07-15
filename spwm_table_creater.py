import numpy as np
import scipy.integrate as si

timerCLK = 72000000  # 72MHz / 分频
arr = 3600 - 1  # ARR
f = 50  # 频率
amp_scale = 0.75  # 调幅
file_name = 'spwm_table'  # 不带后缀的文件名
file_line_limit = 30  # 行数据量限制
###########################################################
base_rate = timerCLK / (arr + 1)  # 载波频率
t = 1 / f  # 周期
n = round(base_rate / f)  # 一周波分段数量
step_size = 1 / f / n  # 步长
result = []


def spwm_sin(x):
    global f
    return np.sin(2 * np.pi * f * x)


if __name__ == '__main__':
    ccr = range(1, n + 1)
    for i in ccr:
        area = si.quad(spwm_sin, (i - 1) * step_size, i * step_size)[0]  # 求积分
        result += [round((amp_scale * area * arr / step_size + arr) / 2)]

    with open(f'./{file_name}.h', mode='w') as file:
        file.write(f'#ifndef __{file_name}_h\n')
        file.write(f'#define __{file_name}_h\n')

        file.write('\n')
        file.write(f'#define spwm_n {n}\n')
        file.write('\n')
        file.write('const uint16_t spwm_data[spwm_n] = {\n')

        for index, val in enumerate(result):
            if index != 0 and index % file_line_limit == 0:
                file.write('\n')
                continue
            file.write(f'{val},')

        file.write('};\n')
        file.write('\n')

        file.write(f'#endif // __{file_name}_h\n')
