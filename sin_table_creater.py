import math

# 配置信息
step_size = 1  # 步长
line_limit = 30  # 行数据量限制
file_name = 'sin_table2'  # header文件名

if __name__ == '__main__':
    dat = []
    for i in range(0, int(360 / step_size / line_limit)):
        for j in range(0, line_limit):
            dat += [math.sin((line_limit * step_size * i + step_size * j) / 180 * math.pi)]
    with open(f'./{file_name}.h', mode='w') as file:
        file.write(f'#ifndef __{file_name}_h\n')
        file.write(f'#define __{file_name}_h\n')
        file.write('\n')
        file.write('// 正弦表\n')
        file.write(f'double {file_name}[] = ' + '{\n')
        file.write('\t')
        for index, val in enumerate(dat):
            if index != 0 and index % line_limit == 0:
                file.write('\n')
                file.write('\t')
            file.write(f'{val:.3f}, ')
        file.write('\n')
        file.write('};\n')
        file.write('\n')
        file.write(f'#endif // __{file_name}_h\n')
