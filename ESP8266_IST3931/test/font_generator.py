# font_generator.py
#!/usr/bin/env python3
"""
IST3931 LCD字模生成工具
支持将图像或字体转换为C语言数组格式的字模数据
"""

import argparse
from PIL import Image, ImageFont, ImageDraw
import numpy as np

def image_to_font_data(image_path, width, height, threshold=128):
    """
    将图像转换为字模数据
    """
    # 打开图像并转换为灰度
    img = Image.open(image_path).convert('L')
    # 调整大小
    img = img.resize((width, height))
    # 二值化
    img = img.point(lambda p: 255 if p > threshold else 0)
    
    # 计算每行需要的字节数
    bytes_per_row = (width + 7) // 8
    total_bytes = bytes_per_row * height
    
    # 生成字模数据
    data = []
    pixels = list(img.getdata())
    
    for y in range(height):
        for byte_idx in range(bytes_per_row):
            byte_value = 0
            for bit in range(8):
                x = byte_idx * 8 + bit
                if x < width:
                    pixel_index = y * width + x
                    if pixels[pixel_index] > 0:  # 白色像素
                        byte_value |= (1 << (7 - bit))
            data.append(byte_value)
    
    return data

def text_to_font_data(text, font_path, size, width, height):
    """
    将文本使用指定字体渲染为字模数据
    """
    # 创建空白图像
    img = Image.new('L', (width, height), 0)
    draw = ImageDraw.Draw(img)
    
    # 加载字体
    try:
        font = ImageFont.truetype(font_path, size)
    except IOError:
        print(f"无法加载字体: {font_path}")
        return None
    
    # 绘制文本
    draw.text((0, 0), text, fill=255, font=font)
    
    # 转换为字模数据
    return image_to_font_data(img, width, height)

def generate_c_code(data, var_name, bytes_per_line=12):
    """
    生成C语言数组代码
    """
    output = f"const uint8_t {var_name}[] = {{\n"
    
    for i in range(0, len(data), bytes_per_line):
        line = data[i:i+bytes_per_line]
        hex_line = ", ".join([f"0x{byte:02X}" for byte in line])
        output += f"    {hex_line}"
        if i + bytes_per_line < len(data):
            output += ","
        output += "\n"
    
    output += "};\n"
    return output

def main():
    parser = argparse.ArgumentParser(description='IST3931 LCD字模生成工具')
    parser.add_argument('input', help='输入文件或文本')
    parser.add_argument('-o', '--output', required=True, help='输出文件')
    parser.add_argument('--width', type=int, required=True, help='字符宽度')
    parser.add_argument('--height', type=int, required=True, help='字符高度')
    parser.add_argument('--var-name', required=True, help='C变量名')
    parser.add_argument('--is-text', action='store_true', help输入是文本而不是图像文件')
    parser.add_argument('--font', help='字体文件路径(当使用文本时)')
    parser.add_argument('--font-size', type=int, default=16, help='字体大小(当使用文本时)')
    
    args = parser.parse_args()
    
    if args.is_text:
        if not args.font:
            print("错误: 使用文本输入时需要指定字体文件")
            return
        data = text_to_font_data(args.input, args.font, args.font_size, args.width, args.height)
    else:
        data = image_to_font_data(args.input, args.width, args.height)
    
    if data:
        c_code = generate_c_code(data, args.var_name)
        with open(args.output, 'w') as f:
            f.write(c_code)
        print(f"字模已生成到 {args.output}")

if __name__ == "__main__":
    main()
    