import pandas as pd
import re
import numpy as np

def search_star_coordinates(star_code):
    """
    根据星体编号搜索赤经赤纬坐标。

    参数：
    - star_code (str): 星体编号，格式为"前缀数字"，例如"NGC1234"。

    返回值：
    - 如果找到对应的星体编号，则返回包含赤经和赤纬坐标的元组；
    - 如果未找到对应的星体编号，则返回None。

    注意：
    - 需要安装pandas库来读取Excel文件。
    示例：>>> coordinates = search_star_coordinates("NGC1234")>>> print(coordinates)
    (3, 9, 39.0, '-0', 7, 50, 45)
    """

    # 读取 Excel 文件
    df = pd.read_excel('/homen/下载/新NI2023.xls')

    # 使用正则表达式拆分星体编号的前缀和数字部分
    match = re.match(r'([a-zA-Z]+)(\d+)', star_code)
    if not match:
        return None

    prefix = match.group(1)
    number = match.group(2)
    # 判断星体编号的类型（NGC 或 IC）
    if prefix.lower() == 'ngc':
        column_name = '编号'
    elif prefix.lower() == 'ic':
        column_name = '编号'
    elif prefix.lower() == 'm':
        column_name = '编号'
    else:
        return None

    # 搜索星体编号对应的行
    result = df.loc[df[column_name].str.contains(prefix.upper() + number)]

    if not result.empty:
        # 提取赤经和赤纬坐标
        ra_h = result.iloc[0]['RH']
        ra_m = result.iloc[0]['RM']
        ra_s = result.iloc[0]['RS']
        dec_g = result.iloc[0]['V']
        dec_d = result.iloc[0]['DG']
        dec_m = result.iloc[0]['DM']
        dec_s = result.iloc[0]['DS']
        # 处理赤经和赤纬的格式
        if dec_g == '+':
            dec_g = '1'
        else:
            dec_g = '-1'
        # 提取dec的正负号
        RA_deg = (ra_h * 15) + (ra_m * 0.25) + (ra_s * (0.25 / 60))

        DEC_deg = float(dec_g) *(float(dec_d) + float(dec_m) / 60 + float(dec_s) / 3600)
        return ra_h, ra_m, ra_s, dec_g, dec_d, dec_m, dec_s, RA_deg, DEC_deg
    else:
        return None