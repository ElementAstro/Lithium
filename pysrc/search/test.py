from star_search_model import search_star_coordinates

# 用户输入星体编号
star_code = input("请输入星体编号: ")

# 搜索并返回赤经赤纬坐标
coordinates = search_star_coordinates(star_code)

if coordinates:
    print(f"星体编号 {star_code} 的赤经赤纬坐标为：")
    print(f"赤经：{coordinates[0]}h {coordinates[1]}m {coordinates[2]}s")
    print(f"赤纬：{coordinates[4]}° {coordinates[5]}' {coordinates[6]}''")
    print(f"赤经：{coordinates[7]}°", end=" ")
    print(f"赤纬：{coordinates[8]}°")

else:
    print(f"未找到星体编号为 {star_code} 的坐标信息。")